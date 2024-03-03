#include "stdafx.h"
#include "ImGui_Client.h"
#include "GameInstance.h"
#include "Monster.h"

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
	if (Check_Pick())
	{
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_MAPTOOL, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Monster"))))
			return;

		MSG_BOX(TEXT("Monster Added"));
	}

	bCallNewFrame = false;
	__super::Tick(fTimeDelta);
	bCallNewFrame = true;

	bool show_demo_window = true;
	bool show_another_window = true;

	static _bool Is_IMGUI_Switch = false;
	static _bool IsMouseTrigger = false;

	static _int iSettingTransformSelct = { 0 };	/* 위치, 회전, 크기 인덱스 :: Transform용 */
	static _int iSelectTileSetting = { 0 };		/* 라디오 버튼 인덱스 :: 타일용 */

	if (true == ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
		true == ImGui::IsAnyItemHovered() ||
		true == ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		IsMouseTrigger = false;
	else
		IsMouseTrigger = true;

	if (m_pGameInstance->KeyDown(DIK_TAB))
	{
		Is_IMGUI_Switch = !Is_IMGUI_Switch;
	}

	if (true == Is_IMGUI_Switch)
	{
		ImGui::Begin(u8"Tool");
		ImGui::Text("X: % .2f\tY:% .2f\tZ:% .2f", m_vPickPos.x, m_vPickPos.y, m_vPickPos.z);
		ImGui::End();
	}

	/*CGameObject* pMonster = *m_pGameInstance->Get_LayerList(LEVEL_EDITOR, TEXT("Layer_Monster"))->begin();
	CTransform* transform = (CTransform*)pMonster->Get_Component(g_strTransformTag);
	EditGuizmo(transform->Get_WorldFloat4x4());*/
}

HRESULT CImGui_Client::Render()
{
	if (bCallNewFrame)
	{
		__super::Render();
	}

	return S_OK;
}

_bool CImGui_Client::Check_Pick()
{
	if (m_pGameInstance->MouseDown(DIMKS_RBUTTON))
	{
		const CVIBuffer_Terrain* pTerrainBuf = dynamic_cast<const CVIBuffer_Terrain*>(m_pGameInstance->Get_Component(LEVEL_MAPTOOL, TEXT("Layer_Grid"), TEXT("Com_VIBuffer")));
		if (!pTerrainBuf)
		{
			MSG_BOX(TEXT("Grid-> Failed to get : TerrainBuffer "));
			return false;
		}
		const CTransform* pTransform = dynamic_cast<const CTransform*>(m_pGameInstance->Get_Component(LEVEL_MAPTOOL, TEXT("Layer_Grid"), g_strTransformTag));
		if (!pTransform)
		{
			MSG_BOX(TEXT("Grid-> Failed to get : pTransform"));
			return false;
		}

		m_vPickPos = pTerrainBuf->Pick_Pos(pTransform);

		if (!XMVector3Equal(XMLoadFloat3(&m_vPickPos), XMVectorSet(0.f, 0.f, 0.f, 0.f)))
		{
			return true;
		}
	}
	return false;
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
