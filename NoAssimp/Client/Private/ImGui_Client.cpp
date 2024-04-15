#include "stdafx.h"
#include "GameInstance.h"
#include "ImGui_Client.h"
#include "Body.h"
#include "Wizard.h"

CImGui_Client::CImGui_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMyImGui{ pDevice, pContext }
{
}

HRESULT CImGui_Client::Initialize()
{
	__super::Initialize(g_hWnd);
	m_pCurLevelIndex = m_pGameInstance->Get_CurrentLevelID();
	return S_OK;
}

static _bool Is_IMGUI_Switch = true;
static _bool IsMouseTrigger = false;

void CImGui_Client::Tick(_float fTimeDelta)
{
	Settings(fTimeDelta);
	OnKeyInput();
	if (true == Is_IMGUI_Switch && m_pPlayer && *m_pCurLevelIndex != LEVEL_LOADING)
	{
		ImGui::Begin("Jack");

		ConvertToString();

		ImGui::End();
	}

	if (true == Is_IMGUI_Switch && m_pPlayer && *m_pCurLevelIndex == LEVEL_WIZARD) {
		ImGui::Begin("Wizard");
		WizardDebug();
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

	if (m_pPlayer == nullptr) {
		if (*m_pGameInstance->Get_CurrentLevelID() < LEVEL_GAMEPLAY || *m_pGameInstance->Get_CurrentLevelID() > LEVEL_WIZARD)
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
	_bool bGroundCol = m_pPlayer->Get_GroundCol();
	string strGroundCol = "GroundCol : ";
	if (bGroundCol)
		strGroundCol += "TRUE";
	else
		strGroundCol += "FALSE";
	_bool bIsJumping = m_pPlayer->Get_IsJumping();
	string strIsJumping = "IsJumping : ";
	if (bIsJumping)
		strIsJumping += "TRUE";
	else
		strIsJumping += "FALSE";
	_float4 vWeaponPos = m_pPlayer->Get_WeaponPos(eWeapon);
	string strWeaponPos = "WeaponPos : " + to_string(vWeaponPos.x).erase(to_string(vWeaponPos.x).size() - 4) + ", "
		+ to_string(vWeaponPos.y).erase(to_string(vWeaponPos.y).size() - 4) + ", " + to_string(vWeaponPos.z).erase(to_string(vWeaponPos.z).size() - 4);

	CNavigation* pPlayerNavigation = dynamic_cast<CNavigation*>(m_pPlayer->Get_Component(TEXT("Com_Navigation")));
	string strCellIndex;
	if(pPlayerNavigation != nullptr)
		strCellIndex = "CurrentCell : " + to_string(pPlayerNavigation->Get_CurrentIndex());
	CNavigation* pPlayerNavigationSBR = dynamic_cast<CNavigation*>(m_pPlayer->Get_Component(TEXT("Com_Navigation_SlimeBossRoom")));
	string strCellIndexSBR;
	if(pPlayerNavigationSBR != nullptr)
		strCellIndexSBR = "CurrentCell_SBR : " + to_string(pPlayerNavigationSBR->Get_CurrentIndex());
	ImGui::Text(strPlayerPos.c_str());
	ImGui::Text(strCamPos.c_str());
	ImGui::Text(strWeaponPos.c_str());
	ImGui::Text(strAnimName.c_str());
	ImGui::Text(strDir.c_str());
	ImGui::Text(strState.c_str());
	ImGui::Text(strWeapon.c_str());
	ImGui::Text(strAttack.c_str());
	ImGui::Text(strGroundCol.c_str());
	ImGui::Text(strIsJumping.c_str());
	ImGui::Text(strCellIndex.c_str());
	if (pPlayerNavigationSBR != nullptr)
		ImGui::Text(strCellIndexSBR.c_str());
}

void CImGui_Client::WizardDebug()
{
	CWizard* pWizard = dynamic_cast<CWizard*>(m_pGameInstance->Get_GameObject(LEVEL_WIZARD, TEXT("Layer_Wizard"), TEXT("Prototype_GameObject_Wizard")));
	if (nullptr == pWizard)
		return;

	Safe_AddRef(pWizard);
	INFO_WIZARD tWizardInfo = pWizard->Get_WizardInfo();
	string strState = "State: " + m_vecWizardState[tWizardInfo.eState];
	string strCurAnim = "CurAnim: " + tWizardInfo.strCurAnim;
	string strWizardPhase = "Phase: " + to_string(tWizardInfo.iWizardPhase);
	string strAttackCount = "AttackCount: " + to_string(tWizardInfo.iAttackCount);
	string strAttackFinishCount = "AttackFinishCount: " + to_string(tWizardInfo.iAttackFinishCount);
	string strSpellIndex = "Spell: ";
	if (tWizardInfo.eSpell == 0)
		strSpellIndex += "LIGHTNING";
	else if(tWizardInfo.eSpell == 1)
		strSpellIndex += "TORNADO";
	else if(tWizardInfo.eSpell == 2)
		strSpellIndex += "HOLEWALL";
	else if (tWizardInfo.eSpell == 3)
		strSpellIndex += "SPELL_END";
	string strTime = "Time: " + to_string(tWizardInfo.fTime).erase(to_string(tWizardInfo.fTime).size() - 4);
	string strIdleTime = "IdleTime: " + to_string(tWizardInfo.fIdleTime).erase(to_string(tWizardInfo.fIdleTime).size() - 4);
	string strWaitTime = "WaitTime: " + to_string(tWizardInfo.fWaitTime).erase(to_string(tWizardInfo.fWaitTime).size() - 4);
	string strChargeTime = "ChargeTime: " + to_string(tWizardInfo.fChargeTime).erase(to_string(tWizardInfo.fChargeTime).size() - 4);

	ImGui::Text(strState.c_str());
	ImGui::Text(strCurAnim.c_str());
	ImGui::Text(strWizardPhase.c_str());
	ImGui::Text(strAttackCount.c_str());
	ImGui::Text(strAttackFinishCount.c_str());
	ImGui::Text(strSpellIndex.c_str());
	ImGui::Text(strTime.c_str());
	ImGui::Text(strIdleTime.c_str());
	ImGui::Text(strWaitTime.c_str());
	ImGui::Text(strChargeTime.c_str());

	Safe_Release(pWizard);
}

void CImGui_Client::Release_Player()
{
	if(m_pPlayer)
		Safe_Release(m_pPlayer);
	m_pPlayer = nullptr;
}

void CImGui_Client::Replace_Player(CGameObject* pPlayer)
{
	m_pPlayer = dynamic_cast<CPlayer*>(pPlayer);
	Safe_AddRef(m_pPlayer);
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
	__super::Free();
}
