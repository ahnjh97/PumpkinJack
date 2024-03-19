#include "stdafx.h"
#include "ImGui_MapTool.h"
#include "GameInstance.h"
#include "Monster.h"
#include "GameObject.h"
#include <fstream>
#include "ImGuizmo.h"
#include <locale>
#include <codecvt>

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
static _int iLevelIndex = 0;
static _int AnimationIdx = -1;

HRESULT CImGui_MapTool::Initialize()
{
	__super::Initialize(g_hWnd);
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

	if (m_pGameInstance->MouseDown(DIMKS_RBUTTON))
		OnRightClick(); // 피킹으로 객체생성

	if (!ImGui::IsAnyItemHovered() && m_pGameInstance->MouseDown(DIMKS_LBUTTON))
		OnLeftClick(); // 피킹으로 객체 고르기

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

		Edit_Object(); // 선택한 객체 월드행렬 수정
	}

	
	if (m_pGameInstance->KeyDown(DIK_DELETE))
		On_DIK_Delete();


	if ((m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) & 0x80) && m_pGameInstance->KeyDown(DIK_S))
		Save_Level();

	if (bIsLevelOpen == false)
	{
		Load_Level();
		bIsLevelOpen = true;
	}
}

HRESULT CImGui_MapTool::Render()
{
	if (bCallNewFrame)
		__super::Render();

	return S_OK;
}

void CImGui_MapTool::OnRightClick()
{
	const CVIBuffer_Terrain* pTerrainBuf = dynamic_cast<const CVIBuffer_Terrain*>(m_pGameInstance->Get_Component(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Com_VIBuffer")));
	if (!pTerrainBuf)
	{
		MSG_BOX(TEXT("Grid-> Failed to get : TerrainBuffer "));
		return;
	}
	const CTransform* pTransform = dynamic_cast<const CTransform*>(m_pGameInstance->Get_Component(LEVEL_MAPTOOL, TEXT("Layer_Grid"), g_strTransformTag));
	if (!pTransform)
	{
		MSG_BOX(TEXT("Grid-> Failed to get : pTransform"));
		return;
	}

	m_vPickPos = pTerrainBuf->Get_PickPos(pTransform);

	if (!XMVector3Equal(XMLoadFloat3(&m_vPickPos), XMVectorSet(0.f, 0.f, 0.f, 0.f)))
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

void CImGui_MapTool::OnLeftClick()
{
	CGameObject* pObj = m_pGameInstance->Select_ModelByPicking(m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Parse"));
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

void CImGui_MapTool::On_DIK_Escape()
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
}
