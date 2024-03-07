#include "stdafx.h"
#include "ImGui_Client.h"
#include "GameInstance.h"
#include "Monster.h"
#include "GameObject.h"
#include <fstream>
#include "ImGuizmo.h"
#include <locale>
#include <codecvt>

CImGui_Client::CImGui_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMyImGui{ pDevice, pContext }
{
}

HRESULT CImGui_Client::Initialize()
{
	__super::Initialize(g_hWnd);
	return S_OK;
}

void CImGui_Client::Tick(_float fTimeDelta)
{
	bCallNewFrame = false;
	__super::Tick(fTimeDelta);
	bCallNewFrame = true;

	bool show_demo_window = true;
	bool show_another_window = true;

	static _bool Is_IMGUI_Switch = true;
	static _bool IsMouseTrigger = false;

	static _int iSettingTransformSelct = { 0 };	/* 위치, 회전, 크기 인덱스 :: Transform용 */
	static _int iSelectTileSetting = { 0 };		/* 라디오 버튼 인덱스 :: 타일용 */

	if (true == ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
		true == ImGui::IsAnyItemHovered() ||
		true == ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		IsMouseTrigger = false;
	else
		IsMouseTrigger = true;

	OnRightClick();

	if (!ImGui::IsAnyItemHovered() && m_pGameInstance->MouseDown(DIMKS_LBUTTON)) // 모델선택: 
	{
		CGameObject* pObj = m_pGameInstance->Select_ModelByPicking(m_pGameInstance->Get_CurrentLevelID());
		if (pObj != nullptr)
		{
			if (pObj != m_pPickedObject)
			{
				bAnimWindow = false;
				if (!vecAnimNames.empty())
				{
					for (const char* animName : vecAnimNames)
						free(const_cast<char*>(animName));  // strdup으로 복사한 메모리를 free로 해제 
					vecAnimNames.clear();
				}

				CModel* pModel = dynamic_cast<CModel*>(pObj->Get_Component(TEXT("Com_Model")));
				string strModelName;
				if (pModel != nullptr)
					strModelName = pModel->Get_ModelName();

				for (auto AnimModelsIter : vecAnimModels)
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
					for (auto AnimationIter : vecAnimations)
						vecAnimNames.push_back(_strdup(AnimationIter->Get_Name().c_str()));
					
				}
			}

			m_pPickedObject = pObj;
		}
	}

	if (m_pGameInstance->KeyDown(DIK_ESCAPE)) // esc누르면 선택 취소
		m_pPickedObject = nullptr;

	if (m_pGameInstance->KeyDown(DIK_TAB))
	{
		Is_IMGUI_Switch = !Is_IMGUI_Switch;
	}

	if (true == Is_IMGUI_Switch)
	{
		ImGui::Begin(u8"Models");
		ImGui::SeparatorText("Level");
		static _int iLevelIndex = 0;
		for (_int i = LEVEL_GAMEPLAY; i < LEVEL_END; i++)
		{
			if (ImGui::RadioButton(vecLevelName[i].c_str(), iLevelIndex == i - LEVEL_GAMEPLAY))
			{
				iLevelIndex = i - LEVEL_GAMEPLAY; // 선택 시 실행할 로직 추가
			}
			if (i % 2 == 0)
				ImGui::SameLine();
		}
		ImGui::NewLine();

		// ----------------------Anim---------------------------------
		static int AnimModelIdx = -1;
		static int NonAnimModelIdx = -1;
		ImGui::SeparatorText("Anim");
		ImGui::SetNextItemWidth(250.0f);
		if(ImGui::ListBox("##Anim", &AnimModelIdx, vecAnimModels.data(), vecAnimModels.size(), 5))
		{
			strCurModel = vecAnimModels[AnimModelIdx];
			NonAnimModelIdx = -1;
		}
		ImGui::NewLine();
		// ----------------------NonAnim-------------------------------
		ImGui::SeparatorText("NonAnim");
		ImGui::SetNextItemWidth(250.0f);
		if(ImGui::ListBox("##NonAnim", &NonAnimModelIdx, vecNonAnimModels.data(), vecNonAnimModels.size(), 9))
		{
			strCurModel = vecNonAnimModels[NonAnimModelIdx];
			AnimModelIdx = -1;
		}
		// --------------------Animation-------------------------------
		if (bAnimWindow && m_pPickedObject)
		{
			ImGui::Begin(u8"Animation");
			static int AnimationIdx = -1;
			ImGui::SetNextItemWidth(450.0f);
			if (ImGui::ListBox("##Animation", &AnimationIdx, vecAnimNames.data(), vecAnimNames.size(), 9))
			{
				strCurModel = vecAnimNames[AnimationIdx];
			}
			ImGui::End();
		}
		
		ImGui::End();
	
		//---------------------- ImGuizmo-------------------------------
		Edit_Object(); // 선택한 객체 월드행렬 수정
	}



	if ((m_pGameInstance->Get_DIKeyState(DIK_LCONTROL) & 0x80) && m_pGameInstance->KeyDown(DIK_S))
	{
		for (_uint i = LEVEL_GAMEPLAY; i < LEVEL_END; i++)
			m_pGameInstance->Binarize_Objects(vecLevelName[i]);
	}		
}

HRESULT CImGui_Client::Render()
{
	if (bCallNewFrame)
	{
		__super::Render();
	}

	return S_OK;
}

void CImGui_Client::OnRightClick()
{
	if (m_pGameInstance->MouseDown(DIMKS_RBUTTON))
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
			wstring wstrPrototypeTag = TEXT("Prototype_GameObject_Tool_NonAnim"); 
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_MAPTOOL, TEXT("Layer_Parse"), wstrPrototypeTag, &tempDesc)))
				return;
			m_bIsPicking = false;
		}
	}
}

void CImGui_Client::Edit_Object()
{
	if (m_pPickedObject != nullptr)
	{
		CTransform* pTransform = dynamic_cast<CTransform*>(m_pPickedObject->Get_Component(g_strTransformTag));
		if (pTransform != nullptr)
		{
			_float4x4 tempMatrix = pTransform->Get_WorldFloat4x4();
			EditGuizmo(tempMatrix); // 선택한 모델의 월드행렬을 수정 
			pTransform->Set_WorldMatrix(tempMatrix);
		}
	}
	else
		EditGuizmo(_float4x4());
}

void CImGui_Client::See_Animation()
{
	
}

CImGui_Client* CImGui_Client::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CImGui_Client* pInstance = new CImGui_Client(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CImGui_Client"));
		Safe_Release<CImGui_Client*>(pInstance);
	}

	return pInstance;
}

void CImGui_Client::Free()
{

	__super::Free();
}
