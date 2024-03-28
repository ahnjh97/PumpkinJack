#include "stdafx.h"
#include "GameInstance.h"
#include "ImGui_Client.h"
#include "Body.h"

CImGui_Client::CImGui_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMyImGui{ pDevice, pContext }
{
}

HRESULT CImGui_Client::Initialize()
{
	__super::Initialize(g_hWnd);
	return S_OK;
}

static _bool Is_IMGUI_Switch = true;
static _bool IsMouseTrigger = false;

void CImGui_Client::Tick(_float fTimeDelta)
{
	Settings(fTimeDelta);
	OnKeyInput();
	if (true == Is_IMGUI_Switch && m_pPlayer)
	{
		ImGui::Begin("Jack");

		ConvertToString();

		ImGui::End();
	}
}

HRESULT CImGui_Client::Render()
{
	if (bCallNewFrame)
		__super::Render();

	return S_OK;
}

void CImGui_Client::Settings(_float fTimeDelta)
{
	bCallNewFrame = false;
	__super::Tick(fTimeDelta);
	bCallNewFrame = true;

	if (true == ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) ||
		true == ImGui::IsAnyItemHovered() ||
		true == ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		IsMouseTrigger = false;
	else
		IsMouseTrigger = true;

	if (m_pPlayer == nullptr)
	{
		if (*m_pGameInstance->Get_CurrentLevelID() < LEVEL_GAMEPLAY || *m_pGameInstance->Get_CurrentLevelID() > LEVEL_SLIMEBOSS)
			return;

		m_pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player")));
		if(m_pPlayer != nullptr)
			Safe_AddRef(m_pPlayer);
	}
}

void CImGui_Client::OnKeyInput()
{
	if ((m_pGameInstance->Get_DIKeyState(DIK_LSHIFT) & 0x80) && (m_pGameInstance->KeyDown(DIK_TAB)))
	{
		Is_IMGUI_Switch = !Is_IMGUI_Switch; // ImGui 보이기 / 숨기기 토글
		m_pGameInstance->Toggle_Toggle();
	}

	if (m_pGameInstance->KeyDown(DIK_Q))
		m_pGameInstance->Toggle_NavigationMode();
}

void CImGui_Client::ConvertToString()
{
	CTransform* pPlayerTransform = dynamic_cast<CTransform*>(m_pPlayer->Get_Component(g_strTransformTag));
	_float4 vPlayerPos = pPlayerTransform->Get_State_Float4(CTransform::STATE_POSITION);
	string strPlayerPos = "PlayerPos : " + to_string(vPlayerPos.x).erase(to_string(vPlayerPos.x).size() - 4) + ", "
		+ to_string(vPlayerPos.y).erase(to_string(vPlayerPos.y).size() - 4) + ", " + to_string(vPlayerPos.z).erase(to_string(vPlayerPos.z).size() - 4);
	_float4 vCamPos = m_pGameInstance->Get_CamPosition_Float4();
	string strCamPos = "CamPos : " + to_string(vCamPos.x).erase(to_string(vCamPos.x).size() - 4) + ", "
		+ to_string(vCamPos.y).erase(to_string(vCamPos.y).size() - 4) + ", " + to_string(vCamPos.z).erase(to_string(vCamPos.z).size() - 4);
	_float3 vDir = m_pPlayer->Get_TargetDir();
	string strDir = "Direction : " + to_string(vDir.x).erase(to_string(vDir.x).size() - 4) + ", "
		+ to_string(vDir.y).erase(to_string(vDir.y).size() - 4) + ", " + to_string(vDir.z).erase(to_string(vDir.z).size() - 4);
	CBody* pBody = dynamic_cast<CBody*>((m_pPlayer->Get_Part(TEXT("Part_Body"))));
	CModel* pPlayerModel = dynamic_cast<CModel*>(pBody->Get_Component(TEXT("Com_Model")));
	string strAnimName = "Anim : "  + pPlayerModel->Get_AnimationName();
	STATE eState = m_pPlayer->Get_State();
	string strState = "State : " + m_vecState[eState];
	WEAPON eWeapon = m_pPlayer->Get_Weapon();
	string strWeapon = "Weapon : " + m_vecWeapon[eWeapon];
	_uint iLastAttack = m_pPlayer->Get_Attack();
	string strAttack = "Attack : Attack" + to_string(iLastAttack);
	_bool bIsJumping = m_pPlayer->Get_IsJumping();
	string strIsJumping = "IsJumping : ";
	if (bIsJumping)
		strIsJumping += "TRUE";
	else
		strIsJumping += "FALSE";
	_float4 vWeaponPos = m_pPlayer->Get_WeaponPos(eWeapon);
	string strWeaponPos = "WeaponPos : " + to_string(vWeaponPos.x).erase(to_string(vWeaponPos.x).size() - 4) + ", "
		+ to_string(vWeaponPos.y).erase(to_string(vWeaponPos.y).size() - 4) + ", " + to_string(vWeaponPos.z).erase(to_string(vWeaponPos.z).size() - 4);

	ImGui::Text(strPlayerPos.c_str());
	ImGui::Text(strCamPos.c_str());
	ImGui::Text(strWeaponPos.c_str());
	ImGui::Text(strAnimName.c_str());
	ImGui::Text(strDir.c_str());
	ImGui::Text(strState.c_str());
	ImGui::Text(strWeapon.c_str());
	ImGui::Text(strAttack.c_str());
	ImGui::Text(strIsJumping.c_str());
}

CImGui_Client* CImGui_Client::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CImGui_Client* pInstance = new CImGui_Client(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CImGui_Client"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CImGui_Client::Free()
{
	Safe_Release(m_pPlayer);
	__super::Free();
}
