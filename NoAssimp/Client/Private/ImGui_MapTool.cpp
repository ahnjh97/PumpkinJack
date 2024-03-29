#include "stdafx.h"
#include "ImGui_MapTool.h"
#include "GameInstance.h"
#include "Monster.h"
#include "GameObject.h"
#include <fstream>
#include "ImGuizmo.h"
#include <locale>
#include <codecvt>
#include "Grid.h"
#include "Cell.h"
#include <chrono>
#include <ctime>

CImGui_MapTool::CImGui_MapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMyImGui{ pDevice, pContext }
{

}

static _bool Is_IMGUI_Switch = true;
static _bool IsMouseTrigger = false;

static _int iSettingTransformSelct = { 0 };	/* 위치, 회전, 크기 인덱스 :: Transform용 */
static _int iSelectTileSetting = { 0 };		/* 라디오 버튼 인덱스 :: 타일용 */

static _int AnimModelIdx = 0;
static _int NonAnimModelIdx = -1;
static _int iLevelIndex = 3;
static _int AnimationIdx = -1;
static _int iNavigationIndex = 0;

HRESULT CImGui_MapTool::Initialize()
{
	__super::Initialize(g_hWnd);

	m_pShader = CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_Cell.hlsl"), VTXPOS::Elements, VTXPOS::iNumElements);
	if (nullptr == m_pShader)
		return E_FAIL;

	return S_OK;
}


void CImGui_MapTool::Tick(_float fTimeDelta)
{
	if (strCurModel == "")
		strCurModel = vecAnimModels.front();
		
	bCallNewFrame = false;
	__super::Tick(fTimeDelta);
	bCallNewFrame = true;


	if (true == ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
		true == ImGui::IsAnyItemHovered() ||
		true == ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		IsMouseTrigger = false;
	else
		IsMouseTrigger = true;


	strCurLevel = vecLevelName[iLevelIndex + (_int)(LEVEL_GAMEPLAY)];

	if (m_pGameInstance->KeyDown(DIK_SPACE))
		bNavigationMode = !bNavigationMode;

	if (m_pGameInstance->MouseDown(DIMKS_RBUTTON))
	{
		if (bNavigationMode == false)
			OnRightClick_ObjectMode(); // 피킹으로 객체생성
	}
	
	if ((m_pGameInstance->Get_DIMouseState(DIMKS_RBUTTON) & 0x80) && bNavigationMode)
	{
		if(m_iOption == 1)
			Add_Cells();
		else if(m_iOption == 2)
		{
			_int iCellIndex{};
			CCell* pCell = Pick_Cell(iCellIndex);
			if (pCell != nullptr)
			{
				Safe_Release(pCell);
				vecCells[iCellIndex] = nullptr;
			}
		}
	}

	if (!ImGui::IsAnyItemHovered() && m_pGameInstance->MouseDown(DIMKS_LBUTTON) && !(m_pGameInstance->Get_DIKeyState(DIK_LSHIFT) & 0x80))
	{
		if (bNavigationMode == false)
			OnLeftClick_ObjectMode(); // 피킹으로 객체 고르기
		else
			OnLeftClick_NavigationMode();
	}
		

	if (m_pGameInstance->KeyDown(DIK_ESCAPE)) 
		On_DIK_Escape(); // esc누르면 선택 취소
	
	if ((m_pGameInstance->Get_DIKeyState(DIK_LSHIFT) & 0x80 )&& (m_pGameInstance->KeyDown(DIK_TAB)))
		Is_IMGUI_Switch = !Is_IMGUI_Switch; // ImGui 보이기 / 숨기기 토글

	if (true == Is_IMGUI_Switch)
	{
		ImGui::Begin(u8"Models");
		
		if (Menu_Level())
		{
			ImGui::End();
			return;
		}
		Menu_Anim();
		Menu_NonAnim();
		Menu_Animation();
		ImGui::End();

		ImGui::Begin(u8"Mode");
		Menu_Navigation();
		ImGui::End();

		Edit_Object(); // 선택한 객체 월드행렬 수정
	}

	
	if (m_pGameInstance->KeyDown(DIK_DELETE))
		On_DIK_Delete();

	if (bIsLevelOpen == false)
	{
		Load_Level();
		bIsLevelOpen = true;
	}

	if ((m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) & 0x80) && m_pGameInstance->KeyDown(DIK_S))
		Save_Level();

	if (bNavigationMode == true)
		OptionChange();
}

HRESULT CImGui_MapTool::Render()
{
	if (bCallNewFrame)
		__super::Render();

	if (!Is_IMGUI_Switch || !bNavigationMode)
		return S_OK;

	CGrid* pGrid = dynamic_cast<CGrid*>(m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Prototype_GameObject_Grid")));
	if (pGrid != nullptr) 
	{
		if (m_pShader == nullptr)
			return S_OK;
		CTransform* pGridTransform = dynamic_cast<CTransform*>(pGrid->Get_Component(g_strTransformTag));
		
		if (FAILED(m_pShader->Bind_Matrix("g_WorldMatrix", pGridTransform->Get_WorldFloat4x4_Ptr())))
			return E_FAIL;

		if (FAILED(m_pShader->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;

		if (FAILED(m_pShader->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;

		m_pShader->Begin(0);

		for (auto& cell : vecCells)
		{
			if (cell != nullptr)
				cell->Render();
		}
		
	}

	return S_OK;
}

void CImGui_MapTool::OnRightClick_ObjectMode()
{
	CGameObject* pGrid = m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Prototype_GameObject_Grid"));
	CVIBuffer_Terrain* pTerrainBuf = dynamic_cast<CVIBuffer_Terrain*>(pGrid->Get_Component(TEXT("Com_VIBuffer")));
	const CTransform* pTransform = dynamic_cast<const CTransform*>(pGrid->Get_Component(g_strTransformTag));

	m_vPickPos = pTerrainBuf->Get_PickPos(pTransform);

	if (!::XMVector3Equal(::XMLoadFloat3(&m_vPickPos), ::XMVectorSet(0.f, 0.f, 0.f, 0.f)))
	{
		_float4 vTemp(m_vPickPos.x, m_vPickPos.y, m_vPickPos.z, 1.0f); // 위치
		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		memcpy(&tempDesc.matWorld.m[CTransform::STATE_POSITION], &vTemp, sizeof(_float4));

		m_bIsPicking = true;
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		wstring wstrModelTag = L"Prototype_Component_Model_" + converter.from_bytes(strCurModel);
		_bool IsAnim = { false };
		for (auto& AnimModelIter : vecAnimModels)
		{
			if (AnimModelIter == strCurModel)
			{
				wstrModelTag = wstrModelTag + L"_NonAnim";
				break;
			}
		}
		tempDesc.wstrModelTag = wstrModelTag;
		for (_uint i = 0; i < vecLevelName.size(); i++)
		{
			if(strCurLevel == vecLevelName[i])
				tempDesc.iLevel = i;
		}
				
			
		wstring wstrPrototypeTag = TEXT("Prototype_GameObject_Tool_NonAnim");
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_MAPTOOL, TEXT("Layer_Parse"), wstrPrototypeTag, &tempDesc)))
			return;
	
		m_bIsPicking = false;
	}
}

void CImGui_MapTool::OnLeftClick_ObjectMode()
{
	CGameObject* pObj = m_pGameInstance->Select_ModelByPicking(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Parse"));
	if (pObj == nullptr)
		return;

	if (pObj == m_pPickedObject)
		return;

	if (pObj != m_pPickedObject)
	{
		if (bSeeAnim)
		{
			pNonAnimModel->Set_Hide(false);
			pNonAnimModel = nullptr;
			if (pAnimObject != nullptr)
			{
				pAnimObject->Set_Dead();
				pAnimObject = nullptr;
			}
			bSeeAnim = false;
			AnimationIdx = -1;
		}
		vecAnimNames.clear();
		bAnimWindow = false;

		CModel* pModel = dynamic_cast<CModel*>(pObj->Get_Component(TEXT("Com_Model")));
		string strModelName;
		if (pModel != nullptr)
			strModelName = pModel->Get_ModelName();

		for (auto& AnimModelsIter : vecAnimModels)
		{
			if (AnimModelsIter == strModelName)
			{
				bAnimWindow = true;
				break;
			}
		}

		if (bAnimWindow == true) // AnimModel이면
		{
			// 애님 버전의 
			wstring_convert<codecvt_utf8<wchar_t>> converter;
			wstring wstrModelName = converter.from_bytes(strModelName);
			wstring wstrPrototypeTag = L"Prototype_Component_Model_" + wstrModelName;
			CModel* pAnimModel = dynamic_cast<CModel*>(m_pGameInstance->Find_Prototype(LEVEL_MAPTOOL, wstrPrototypeTag));

			pAnimModel->Copy_Animations(vecAnimations);
			for (auto& AnimationIter : vecAnimations)
			{
				string strAnimName = AnimationIter->Get_Name();
				vecAnimNames.push_back(strAnimName);
			}
		}
	}

	m_pPickedObject = pObj;
}

void CImGui_MapTool::OnLeftClick_NavigationMode()
{
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_MAPTOOL, TEXT("Layer_Parse"));

	if (objList.empty())
		return;

	vector<_float4> vecPickPos;
	for (auto& obj : objList)
	{
		if (obj == nullptr)
			continue;

		CModel* pModel = dynamic_cast<CModel*>(obj->Get_Component(TEXT("Com_Model")));
		if (pModel == nullptr)
			continue;

		CTransform* pTransform = dynamic_cast<CTransform*>(obj->Get_Component(g_strTransformTag));
		if (pTransform == nullptr)
			continue;

		_float4 vTemp = pModel->Check_Meshes(pTransform);
		if(vTemp.w != 0)
			vecPickPos.push_back(vTemp);
	}

	if (vecPickPos.empty())
		return;

	_float fShortest = { FLT_MAX };
	_float4 fResult = {};
	for (auto& iter : vecPickPos)
	{
		if (iter.w <= fShortest)
			fResult = iter;
	}

	m_vLeftClickPos = fResult;
}

void CImGui_MapTool::On_DIK_Escape()
{
	bNavigationMode = false;

	if (bSeeAnim)
	{
		pNonAnimModel->Set_Hide(false);
		pNonAnimModel = nullptr;
		if (pAnimObject != nullptr)
		{
			pAnimObject->Set_Dead();
			pAnimObject = nullptr;
		}
		bSeeAnim = false;
		AnimationIdx = -1;
	}

	m_pPickedObject = nullptr;
}

void CImGui_MapTool::On_DIK_Delete()
{
	if (m_pPickedObject != nullptr)
		m_pPickedObject->Set_Dead();
	pNonAnimModel = nullptr;
	m_pPickedObject = nullptr;
	if (pAnimObject != nullptr)
		pAnimObject->Set_Dead();
	pAnimModel = nullptr;
	pAnimObject = nullptr;
}

void CImGui_MapTool::OptionChange()
{
	 if (m_pGameInstance->KeyDown(DIK_1))
		m_iOption = 1;
	else if (m_pGameInstance->KeyDown(DIK_2))
		m_iOption = 2;

	if (!(m_pGameInstance->Get_DIKeyState(DIK_LSHIFT) & 0x80) && m_pGameInstance->KeyDown(DIK_TAB))
	{
		m_iMode += 1;
		if (m_iMode >= 2)
			m_iMode = 0;
	}

	CGrid* pGrid = dynamic_cast<CGrid*>(m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Prototype_GameObject_Grid")));
	if (m_iMode == 0) // SOLID
	{
		pGrid->Set_PassIndex(0);
		pGrid->Set_Hide(false);
	}
	else if (m_iMode == 1) // WIREFRAME
	{
		pGrid->Set_PassIndex(1);
		pGrid->Set_Hide(false);
	}
	else if (m_iMode == 2) // CELL
		pGrid->Set_Hide(true);
}

_bool CImGui_MapTool::Menu_Level()
{
	ImGui::SeparatorText("Level");

	for (_int i = LEVEL_GAMEPLAY; i < LEVEL_END; i++)
	{
		if (ImGui::RadioButton(vecLevelName[i].c_str(), iLevelIndex == i - LEVEL_GAMEPLAY))
		{
			iLevelIndex = i - LEVEL_GAMEPLAY; // 선택 시 실행할 로직 추가
		}
		if (i % 2 == 0)
			ImGui::SameLine();
	}
	if (ImGui::Button("Save", ImVec2(125, 40)))
		Save_Level();
	ImGui::SameLine();
	if (ImGui::Button("Load", ImVec2(125, 40)))
	{
		Load_Level();
		return true;
	}
	return false;
}

void CImGui_MapTool::Menu_Anim()
{
	ImGui::SeparatorText("Anim");
	ImGui::SetNextItemWidth(250.0f);
	const _char** items = new const char* [vecAnimModels.size()];
	for (size_t i = 0; i < vecAnimModels.size(); ++i)
		items[i] = vecAnimModels[i].c_str();

	if (ImGui::ListBox("##Anim", &AnimModelIdx, items, static_cast<int>(vecAnimModels.size()), 5))
	{
		strCurModel = vecAnimModels[AnimModelIdx];
		NonAnimModelIdx = -1;
	}
	Safe_Delete_Array(items);
	ImGui::NewLine();
}

void CImGui_MapTool::Menu_NonAnim()
{
	ImGui::SeparatorText("NonAnim");
	ImGui::SetNextItemWidth(250.0f);
	const _char** items2 = new const char* [vecNonAnimModels.size()];
	for (size_t i = 0; i < vecNonAnimModels.size(); ++i)
		items2[i] = vecNonAnimModels[i].c_str();

	if (ImGui::ListBox("##NonAnim", &NonAnimModelIdx, items2, static_cast<int>(vecNonAnimModels.size()), 9))
	{
		strCurModel = vecNonAnimModels[NonAnimModelIdx];
		AnimModelIdx = -1;
	}

	if (m_pPickedObject == nullptr)
		pAnimModel = nullptr;
	Safe_Delete_Array(items2);
}

void CImGui_MapTool::Menu_Animation()
{
	if (bAnimWindow && m_pPickedObject)
	{
		ImGui::Begin(u8"Animation");
		ImGui::SetNextItemWidth(250.0f);

		const _char** items3 = new const char* [vecAnimNames.size()];
		for (size_t i = 0; i < vecAnimNames.size(); ++i)
			items3[i] = vecAnimNames[i].c_str();

		if (ImGui::ListBox("##Animations", &AnimationIdx, items3, static_cast<int>(vecAnimNames.size()), 9))
		{
			if (bSeeAnim == false)
			{
				bSeeAnim = true;

				PARSE_DATA tempParseData = {};
				tempParseData = m_pPickedObject->Get_ParseData();
				CGameObject::GAMEOBJECT_DESC tempDesc = {};
				tempDesc.matWorld = tempParseData.matWorld;
				tempDesc.wstrModelTag = tempParseData.wstrModelTag;

				wstring wstrPrototypeTag = TEXT("Prototype_GameObject_Tool_Anim");
				if (FAILED(m_pGameInstance->Add_Clone(LEVEL_MAPTOOL, TEXT("Layer_Temp"), wstrPrototypeTag, &tempDesc)))
					return;

				
				pAnimObject = dynamic_cast<CTool_Anim*>(m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Temp"), wstrPrototypeTag));
				if (pAnimObject != nullptr)
					pAnimModel = dynamic_cast<CModel*>(pAnimObject->Get_Component(TEXT("Com_Model")));

				if (m_pPickedObject != nullptr)
				{
					pNonAnimModel = dynamic_cast<CModel*>(m_pPickedObject->Get_Component(TEXT("Com_Model")));
					pNonAnimModel->Set_Hide(true);
				}
			}

			if (m_pPickedObject != nullptr)
				pAnimModel->Set_Animation(AnimationIdx, true);
		}
		Safe_Delete_Array(items3);
		ImGui::End();
	}
}

void CImGui_MapTool::Edit_Object()
{
	if (m_pPickedObject != nullptr)
	{
		if (bSeeAnim == true)
		{
			CTransform* pAnimTransform = dynamic_cast<CTransform*>(pAnimObject->Get_Component(g_strTransformTag));
			if (pAnimTransform != nullptr)
			{
				_float4x4 tempMatrix = pAnimTransform->Get_WorldFloat4x4();
				EditGuizmo(tempMatrix); // 선택한 Anim모델의 월드행렬 위치만 보기
				//pAnimTransform->Set_WorldMatrix(tempMatrix);
			}
		}
		else
		{
			CTransform* pTransform = dynamic_cast<CTransform*>(m_pPickedObject->Get_Component(g_strTransformTag));
			if (pTransform != nullptr)
			{
				_float4x4 tempMatrix = pTransform->Get_WorldFloat4x4();
				EditGuizmo(tempMatrix); // 선택한 모델의 월드행렬을 수정 
				pTransform->Set_WorldMatrix(tempMatrix);
			}
		}
	}
	else
		EditGuizmo(_float4x4());
}

void CImGui_MapTool::Menu_Navigation()
{
	ImGui::Checkbox(vecMode[1].c_str(), &bNavigationMode);

	if (!bNavigationMode)
		return;
	
	if (m_iOption == 1)
		ImGui::Text("	Option : ADD");
	else if(m_iOption == 2)
		ImGui::Text("	Option : DELETE");

	switch (m_iMode)
	{
	case 0:
		ImGui::Text("	RenderState : SOLID");
		break;
	case 1:
		ImGui::Text("	RenderState : WIREFRAME");
		break;
	case 2:
		ImGui::Text("	RenderState : CELLS");
		break;
	}

	if (ImGui::Button("Save", ImVec2(100, 40)))
		Save_Cells();
	ImGui::SameLine();
	if (ImGui::Button("Load", ImVec2(100, 40)))
		Load_Cells();

	ImGui::SeparatorText("LeftClickPos");
	ImGui::Text("	%.2f, %.2f, %.2f ", m_vLeftClickPos.x, m_vLeftClickPos.y, m_vLeftClickPos.z);
}

void CImGui_MapTool::Save_Level()
{
	_uint iCurLevel = { LEVEL_GAMEPLAY };
	for (_uint i = LEVEL_GAMEPLAY; i < LEVEL_END; i++)
	{
		if (strCurLevel == vecLevelName[i])
		{
			iCurLevel = i;
			break;
		}
	}
	m_pGameInstance->Binarize_Objects(iCurLevel, vecLevelName[iCurLevel]);
	MB(string(vecLevelName[iCurLevel] + " Saved."));
}

void CImGui_MapTool::Load_Level()
{
	On_DIK_Escape();
	On_DIK_Delete();
	m_pGameInstance->Clear_Layer(LEVEL_MAPTOOL, TEXT("Layer_Parse"));
	m_pGameInstance->Clear_Layer(LEVEL_MAPTOOL, TEXT("Layer_Temp"));
	string strFileName = "../../../objects_txt/" + strCurLevel + ".txt";

	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		MB("Failed to open CurLevelTxt");
		return;
	}
	
	wchar_t wcharPrototypeTag[MAX_PATH];
	wchar_t wcharModelTag[MAX_PATH];
	_float4x4 matWorld{};

	while (!fileStream.eof())
	{
		fileStream.read(reinterpret_cast<char*>(&wcharPrototypeTag), sizeof(wcharPrototypeTag));
		fileStream.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		fileStream.read(reinterpret_cast<char*>(&wcharModelTag), sizeof(wcharModelTag));

		if (fileStream.eof())
			break;
			
		wstring wstrPrototypeTag = wcharPrototypeTag;
		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		tempDesc.matWorld = matWorld;
		tempDesc.wstrModelTag = wcharModelTag;
		for (_uint i = 0; i < vecLevelName.size(); i++)
		{
			if (strCurLevel == vecLevelName[i])
				tempDesc.iLevel = i;
		}
		string strModelTag = m_pGameInstance->WstrToStr(tempDesc.wstrModelTag);
		size_t lastUnderscorePos = strModelTag.find_last_of('_');
		strModelTag = strModelTag.substr(lastUnderscorePos + 1);
		for (auto& AnimModelIter : vecAnimModels)
		{
			if (AnimModelIter == strModelTag)
			{
				tempDesc.wstrModelTag = tempDesc.wstrModelTag + L"_NonAnim";
				break;
			}
		}
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_MAPTOOL, TEXT("Layer_Parse"), TEXT("Prototype_GameObject_Tool_NonAnim"), &tempDesc)))
		{
			MB("Failed to Add_Clone.");
			fileStream.close();
			return;
		}
	}
	fileStream.close();

	Load_Cells(); 
}

void CImGui_MapTool::Save_Cells()
{
	if (vecCells.empty())
		return;

	Erase_NullPointers();

	string strCurLevel = vecLevelName[iLevelIndex + (_int)(LEVEL_GAMEPLAY)];

	string tempFileName = "temp_" + strCurLevel + "_Cells.txt";
	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		string tempstr = "Failed to Open: " + tempFileName;
		MB(tempstr);
		return;
	}

	CGameObject* pGrid = m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Prototype_GameObject_Grid"));
	CVIBuffer_Terrain* pTerrainBuffer = dynamic_cast<CVIBuffer_Terrain*>(pGrid->Get_Component(TEXT("Com_VIBuffer")));
	_int iX = pTerrainBuffer->Get_NumVerticesX();
	_int iZ = pTerrainBuffer->Get_NumVerticesZ();



	for (auto& cell : vecCells)
	{
		if (cell == nullptr)
			continue;

		_int iTerrainIndex = cell->Get_TerrainIndex();
		outputFile.write(reinterpret_cast<const char*>(&iTerrainIndex), sizeof(iTerrainIndex));
		for (_int i = 0; i < 3; i++)
		{
			_float3 vPoint{};
			 ::XMStoreFloat3(&vPoint, cell->Get_Point((CCell::POINT)i));
			 outputFile.write(reinterpret_cast<const char*>(&vPoint), sizeof(vPoint));
		}


		_int iNeighborIndex[3] = { -1, -1, -1 };
		for (_int j = 0; j < 3; j++)
			iNeighborIndex[j] = cell->Get_NeighborIndex((CCell::LINE)j);

		if (iNeighborIndex[0] == -1 || iNeighborIndex[1] || -1 && iNeighborIndex[2] || -1) // NeighborIndex가 하나라도 -1인 경우
		{
			if (iTerrainIndex % 2 == 0) // 아래쪽 삼각형이면
			{
				if (iNeighborIndex[0] == -1) // iTerrainIndex + 1  -> 필연적으로 존재
					cell->Set_NeighborIndex(CCell::LINE_AB, Check_PotentialNeighbor(iTerrainIndex + 1));
				
				if (iTerrainIndex >= iX * 2)
				{
					if (iNeighborIndex[1] == -1) // iTerrainIndex - iX * 2  -> 가장 아랫줄에 있을 경우 존재하지 않음
						cell->Set_NeighborIndex(CCell::LINE_BC, Check_PotentialNeighbor(iTerrainIndex - iX * 2));
				}
				
				if (iTerrainIndex % iX * 2 != 0)
				{
					if (iNeighborIndex[2] == -1) // iTerrainIndex - 1  ->  가장 왼쪽에 있을 경우 존재하지 않음
						cell->Set_NeighborIndex(CCell::LINE_CA, Check_PotentialNeighbor(iTerrainIndex - 1));
				}
			}		
			else						// 위쪽 삼각형이면
			{
				if (iTerrainIndex < iX * 2 * (iZ - 1))
				{
					if (iNeighborIndex[0] == -1) // iTerrainIndex + iX * 2  -> 가장 윗줄에 있을경우 존재하지 않음
						cell->Set_NeighborIndex(CCell::LINE_AB, Check_PotentialNeighbor(iTerrainIndex + iX * 2));
				}
				
				if (iTerrainIndex % iX * 2 != (iX * 2) - 1)
					if (iNeighborIndex[1] == -1)
						cell->Set_NeighborIndex(CCell::LINE_BC, Check_PotentialNeighbor(iTerrainIndex + 1));

				if (iNeighborIndex[2] == -1)
					cell->Set_NeighborIndex(CCell::LINE_CA, Check_PotentialNeighbor(iTerrainIndex - 1));
			}
		}

		for (_int ii = 0; ii < 3; ii++)
			outputFile.write(reinterpret_cast<const char*>(&iNeighborIndex[ii]), sizeof(iNeighborIndex[ii]));
	}
	
	outputFile.close();

	if (!outputFile)
	{
		string errorStr = "Failed to write data to " + tempFileName;
		MB(errorStr);
		remove(tempFileName.c_str()); // 임시파일 삭제
		return;
	}

	// 현재시간 받아오기
	auto now = chrono::system_clock::now();
	time_t currentTime = chrono::system_clock::to_time_t(now);

	struct tm timeinfo;
	localtime_s(&timeinfo, &currentTime);

	// 현재 시간을 문자열로 변환
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);

	string fileName_Time = "../../../objects_txt/" + string(buffer) + "_" + strCurLevel + "_Cells.txt";
	string fileName = "../../../objects_txt/" + strCurLevel + "_Cells.txt";
	if (rename(fileName.c_str(), fileName_Time.c_str()) != 0)
	{
		MB("Failed to rename original file.");
		return;
	}

	if (rename(tempFileName.c_str(), fileName.c_str()) != 0) // 임시파일 이름을 level 이름으로 변경
	{
		string errorStr2 = "Failed to rename " + tempFileName;
		MB(errorStr2);
		remove(tempFileName.c_str()); // 임시파일 삭제
		return;
	}

	MB(strCurLevel + "_Cells.txt Saved.");
}

void CImGui_MapTool::Load_Cells()
{
	for (auto& cell : vecCells)
		Safe_Release(cell);
	vecCells.clear();

	string strFileName = "../../../objects_txt/" + strCurLevel + "_Cells.txt";
	fstream fileStream(strFileName, ios::in | ios::binary);
	if (fileStream.is_open() == false)
	{
		MB("Failed to open CurLevel_Cells.txt");
		return;
	}

	_int iTerrainIndex = -1;
	_float3 vPoints[3];
	_int iNeighborIndices[3];
	while (!fileStream.eof())
	{
		fileStream.read(reinterpret_cast<char*>(&iTerrainIndex), sizeof(iTerrainIndex));

		for(_int i = 0; i< 3; i++)
			fileStream.read(reinterpret_cast<char*>(&vPoints[i]), sizeof(vPoints[i]));

		for (_int j = 0; j < 3; j++)
			fileStream.read(reinterpret_cast<char*>(&iNeighborIndices[j]), sizeof(iNeighborIndices[j]));

		if (fileStream.eof())
			break;

		CCell* pCell = CCell::Create(m_pDevice, m_pContext, iTerrainIndex, vPoints, iNeighborIndices, vecCells.size());
		if (nullptr == pCell)
			break;

		vecCells.emplace_back(pCell);
	}
	fileStream.close();
	bNavigationMode = true;
}

CCell* CImGui_MapTool::Pick_Cell(_Out_ _int& iIndex)
{
	if (vecCells.empty())
		return nullptr;

	CGrid* pGrid = dynamic_cast<CGrid*>(m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Prototype_GameObject_Grid")));
	CTransform* pGridTransform = dynamic_cast<CTransform*>(pGrid->Get_Component(g_strTransformTag));

	_float3 vRayDir, vRayPos;
	m_pGameInstance->Transform_PickingToLocalSpace(pGridTransform, &vRayDir, &vRayPos);

	_vector vPoints[3];
	_float fDis{};

	for (_int i = 0; i< vecCells.size(); i++)
	{
		if (vecCells[i] == nullptr)
			continue;

		for (_int j = 0; j < CCell::POINT_END; j++)
			vPoints[j] = vecCells[i]->Get_Point((CCell::POINT)j);

		_vector vecRayDir = ::XMVector3Normalize(::XMLoadFloat3(&vRayDir));
		_vector vecRayPos = ::XMLoadFloat3(&vRayPos);
		if (::TriangleTests::Intersects(vecRayPos, vecRayDir, vPoints[0], vPoints[1], vPoints[2], fDis))
		{
			iIndex = i;
			return vecCells[i];
		}
	}

	return nullptr; 
}

void CImGui_MapTool::Add_Cells()
{
	CGameObject* pGrid = m_pGameInstance->Get_GameObject(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Prototype_GameObject_Grid"));
	CVIBuffer_Terrain* pTerrainBuffer = dynamic_cast<CVIBuffer_Terrain*>(pGrid->Get_Component(TEXT("Com_VIBuffer")));
	CTransform* pGridTransform = dynamic_cast<CTransform*>(pGrid->Get_Component(g_strTransformTag));	
	//if (vecCellOptions.empty())
	//	vecCellOptions.resize(pTerrainBuffer->Get_NumVerticesX() * pTerrainBuffer->Get_NumVerticesZ() * 2);

	_float3 vPoints[3];
	_int iTriangleIndex = pTerrainBuffer->Get_TriangleIndex(pGridTransform, vPoints[0], vPoints[1], vPoints[2]);
	if (iTriangleIndex == -1)
		return;

	if (Check_ExistingCell(vPoints[0], vPoints[1], vPoints[2]) == true)
		return;
	
	_int iNeighborIndices[3] = { -1,-1,-1 };
	CCell* pCell = CCell::Create(m_pDevice, m_pContext, iTriangleIndex, vPoints, iNeighborIndices, vecCells.size());
	vecCells.emplace_back(pCell);
}

_bool CImGui_MapTool::Check_ExistingCell(_float3 fA, _float3 fB, _float3 fC)
{
	if (vecCells.empty())
		return false;

	for (auto& cell : vecCells)
	{
		if (cell == nullptr)
			continue;
		if (::XMVector3Equal(cell->Get_Point(CCell::POINT_A), ::XMLoadFloat3(&fA)) == false)
			continue;
		if (::XMVector3Equal(cell->Get_Point(CCell::POINT_B), ::XMLoadFloat3(&fB)) == false)
			continue;
		if (::XMVector3Equal(cell->Get_Point(CCell::POINT_C), ::XMLoadFloat3(&fC)) == false)
			continue;

		return true;
	}
	return _bool();
}

_int CImGui_MapTool::Check_PotentialNeighbor(_int iPotentialNeighbor)
{
	for (auto& cell : vecCells)
	{
		if (cell == nullptr)
			continue;

		if (iPotentialNeighbor == cell->Get_TerrainIndex())
			return cell->Get_Index();
	}

	return -1;
}

void CImGui_MapTool::Erase_NullPointers()
{
	for (_int i = vecCells.size() - 1; i >= 0; --i)
	{
		if (vecCells[i] == nullptr)
			vecCells.erase(vecCells.begin() + 1);
	}
}

CImGui_MapTool* CImGui_MapTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CImGui_MapTool* pInstance = new CImGui_MapTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CImGui_MapTool"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CImGui_MapTool::Free()
{
	__super::Free();
	Safe_Release(m_pShader);
	for (auto& cell : vecCells)
		Safe_Release(cell);
	vecCells.clear();
}
