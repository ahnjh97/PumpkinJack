#include "stdafx.h"
#include "Wizard.h"
#include "Player.h"

CWizard::CWizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CWizard::CWizard(const CWizard& rhs)
	: CGameObject{ rhs }
{

}

INFO_WIZARD CWizard::Get_WizardInfo()
{
	INFO_WIZARD tInfo = { m_eState, m_pModelCom->Get_AnimationName(), m_iWizardPhase, m_iAttackCount, m_iAttackFinishCount, m_eSpell, m_fIdleTime };
	return tInfo;
}

HRESULT CWizard::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWizard::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	m_pModelCom->Set_Animation(0, true);
	m_pModelCom->Set_LerpTime(0.1f);
	SetUp_WizardAnimSpeed();
	m_iHp = 300;

	return S_OK;
}

void CWizard::Tick(_float fTimeDelta)
{
	Get_PlayerPtr();
	if (WIZARD_DIE == m_eState && m_pModelCom->isFinished())
		Set_Dead();		

	Check_WizardHp();

	TurnTowards_Player(fTimeDelta);
	Link_State(fTimeDelta);
	StateToAnimation();

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CWizard::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CWizard::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(1))) // NoCull
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CWizard::Get_PlayerPtr()
{
	if (m_pPlayer == nullptr)	{
		m_pPlayer = m_pGameInstance->Get_Player(LEVEL_WIZARD);
		if (m_pPlayer != nullptr)
		{
			Safe_AddRef(m_pPlayer);
			CTransform* pPlayerTransform = dynamic_cast<CTransform*>(m_pPlayer->Get_Component(g_strTransformTag));
			m_pPlayerWorldMatrix = pPlayerTransform->Get_WorldFloat4x4_Ptr();
		}
	}
}

void CWizard::Check_WizardHp()
{
	if (m_iHp <= 0) {
		m_eState = WIZARD_DIE;
		return;
	}

	if (WIZARD_HIT == m_eState || WIZARD_IDLE == m_eState) {
		if ((m_iHp > 100 && m_iHp <= 200 && m_iWizardPhase == 0) || (m_iHp > 0 && m_iHp <= 100 && m_iWizardPhase == 1)) {
			m_eState = WIZARD_AIR_IDLE;
			m_iWizardPhase++;
			m_bTowerSpawn = false;
		}
	}
}

void CWizard::Link_State(_float fTimeDelta)
{
	if (SPELL_LIGHTNING == m_eSpell){
		_int iAnimPlayCount = m_pModelCom->Get_AnimPlayCount(WIZARD_ATTACK);
		if (iAnimPlayCount == m_iAttackCount) {
			m_eState = WIZARD_ATTACK;
			m_iAttackCount++;
			m_iAttackFinishCount++;
			Lightning();
		}

	}

	if (SPELL_TORNADO == m_eSpell || SPELL_HOLEWALL == m_eSpell) {
		if (WIZARD_AIR_IDLE == m_eState) {
			m_fIdleTime += fTimeDelta;
			if (m_fIdleTime >= 0.8f) {
				m_eState = WIZARD_ATTACK;
				m_iAttackCount++;
				m_fIdleTime = 0.f;
			}
		}
	}

	if (WIZARD_ATTACK == m_eState) {
		if (m_iAttackCount > m_iAttackFinishCount) {
			switch (m_eSpell)
			{
			case SPELL_TORNADO:
				Tornado();
				break;
			case SPELL_HOLEWALL:
				HoleWall();
				break;
			}
		}

		if (m_pModelCom->isFinished()) {
			m_eState = WIZARD_AIR_IDLE;
			m_iAttackFinishCount++;
			m_fIdleTime = 0.f;
		}
	}

	if (SPELL_END != m_eSpell) {
		if (m_iAttackFinishCount >= m_vecSpellThresholds[m_eSpell]) {
			m_eSpell = (SPELL)((_int)m_eSpell + 1);
			m_iAttackCount = 0;
			m_iAttackFinishCount = 0;
			m_pModelCom->Reset_AnimPlayCount(WIZARD_ATTACK);
			m_pModelCom->Reset_Animation(string("Air_Attack"));
			m_eState = WIZARD_AIR_IDLE;

			if (m_eSpell == SPELL_END)
				m_eState = WIZARD_CHARGE;
		}
	}

	if (WIZARD_CHARGE == m_eState) {
		if (!m_bTowerSpawn)	{
			Tower();
			m_bTowerSpawn = true;
		}
	}

	m_ePreState = m_eState;
}

void CWizard::StateToAnimation()
{
	_bool bLoop = true;
	_uint iIndex = m_eState;
	if (m_eState == WIZARD_ATTACK || m_eState == WIZARD_DIE || m_eState == WIZARD_DIE2 || m_eState == WIZARD_HIT)
		bLoop = false;

	if (SPELL_LIGHTNING == m_eSpell && WIZARD_ATTACK == m_eState) // Lightning
		bLoop = true;
	m_pModelCom->Change_Animation_ByIndex(iIndex, bLoop);
	// 0.Air_Attack  1.Air_Charge  2.Air_Idle  3.Die  4.Die2  5.Hit  6.Idle  7.Weak
}

void CWizard::TurnTowards_Player(_float fTimeDelta)
{
	if (WIZARD_DIE == m_eState)
		return;

	_vector vPlayerPos = XMVectorSet(m_pPlayerWorldMatrix->_41, m_pPlayerWorldMatrix->_42, m_pPlayerWorldMatrix->_43, 1);
	m_pTransformCom->Look_At_ForLandObject(vPlayerPos);
}

void CWizard::Lightning()
{
	
}

void CWizard::FireBall()
{
}

void CWizard::Tornado()
{
}

void CWizard::HoleWall()
{
}

void CWizard::Tower()
{
}

HRESULT CWizard::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Shader */
	wstring wstrShaderTag  = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.vCenter = _float3(0.f, 0, 0.f);
	ColliderDesc.fRadius = 15.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CWizard::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_LightDesc(0);
	if (nullptr == pLightDesc)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

void CWizard::SetUp_WizardAnimSpeed()
{
	m_pModelCom->Set_AnimSpeed(WIZARD_ATTACK, 1.7f);
	m_pModelCom->Set_AnimSpeed(WIZARD_AIR_IDLE, 1.3f);
}

CWizard* CWizard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWizard* pInstance = new CWizard(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CWizard"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CWizard::Clone(void* pArg)
{
	CWizard* pInstance = new CWizard(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CWizard"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWizard::Free()
{
	__super::Free();
	
	Safe_Release(m_pPlayer);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
