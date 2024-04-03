#include "stdafx.h"
#include "Soldier.h"

CSoldier::CSoldier(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSoldier::CSoldier(const CSoldier& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSoldier::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSoldier::Initialize(void* pArg)
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

	if (m_pModelCom != nullptr)
		Soldier_AnimSpeed();

	m_pModelCom->Set_Animation(10, false); // SPAWN
	return S_OK;
}

void CSoldier::Tick(_float fTimeDelta)
{
	if (nullptr == m_pPlayerWorldMat)
		Get_PlayerWorldMatPtr();
	
	if (nullptr == m_pPlayerWorldMat)
		return;

	if (true == m_pModelCom->isFinished())
		m_eState = SOLDIER_IDLE;

	if (m_eState == SOLDIER_IDLE)
		m_fIdleTime += fTimeDelta;
	else
		m_fIdleTime = 0.f;

	LinkState();
	StateToAnimation();

	if (m_eState == SOLDIER_IDLE || m_eState == SOLDIER_RUN || m_eState == SOLDIER_SPAWN || m_eState == SOLDIER_SPIN)
	{
		if (true == m_pModelCom->Get_IsLerping())
		{
			_float4 vRightFloat{}, vLookFloat{};
			m_pTransformCom->Get_Look_LandObject(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1), vRightFloat, vLookFloat);

			_vector	vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
			_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, XMLoadFloat4(&vLookFloat)));
			_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, XMLoadFloat4(&vLookFloat)));
			if (fY < 0)
				fAngle = -fAngle;
			if (abs(fAngle) >= XMConvertToRadians(2.f))
				m_pTransformCom->Turn(::XMVectorSet(0, 1, 0, 0), fTimeDelta * fAngle * 5.f);
		}
		else
		{
			if (m_eState == SOLDIER_SPIN)
			{
				_float fPlayRatio = m_pModelCom->Get_AnimPlayRatio(14);
				if (fPlayRatio >= 0.4f && fPlayRatio <= 0.7f)
					m_pTransformCom->Look_At_ForLandObject(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1));
			}
			else
				m_pTransformCom->Look_At_ForLandObject(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1));
		}
			
	}

	
	if (m_eState == SOLDIER_RUN)
		m_pTransformCom->Move(fTimeDelta * 2.8f, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));
	else if (m_eState == SOLDIER_SPIN)
	{
		_float fPlayRatio = m_pModelCom->Get_AnimPlayRatio(14);
		if(fPlayRatio >= 0.4f && fPlayRatio <= 0.7f)
			m_pTransformCom->Move(fTimeDelta * 1.9f, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));
	}
		

	for(_int i = 0; i < COLLIDER_END; i++)
		m_pColliderCom[i]->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CSoldier::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSoldier::Render()
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
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	for (_int i = 0; i < COLLIDER_END; i++)
		m_pColliderCom[i]->Render();
#endif

	return S_OK;
}

void CSoldier::Get_PlayerWorldMatPtr()
{
	m_pPlayer = m_pGameInstance->Get_Player(LEVEL_GAMEPLAY);
	if (nullptr == m_pPlayer)
		return;
	CTransform* pPlayerTransform = dynamic_cast<CTransform*>(m_pPlayer->Get_Component(g_strTransformTag));
	if (nullptr == pPlayerTransform)
		return;
	m_pPlayerWorldMat = pPlayerTransform->Get_WorldFloat4x4_Ptr();
}

void CSoldier::LinkState()
{
	if (m_eState == SOLDIER_DEATH && m_pModelCom->isFinished())
	{
		DeathEffect();
		Set_Dead();
	}

	if (m_iHp <= 0)
	{
		m_eState = SOLDIER_DEATH;
		return;
	}
		
	if (m_fIdleTime >= 0.2f)
		m_eState = SOLDIER_RUN;

	if (m_eState == SOLDIER_RUN && true == IsPlayerInRange())
	{
		if (!m_bSpinAttack)
		{
			m_eState = SOLDIER_ATTACK;
			m_bSpinAttack = true;
		}
		else
		{
			m_eState = SOLDIER_SPIN;
			m_bSpinAttack = false;
		}
	}

	if ((m_eState == SOLDIER_ATTACK || m_eState == SOLDIER_SPIN) && m_pModelCom->isFinished())
		m_eState = SOLDIER_IDLE;
}

void CSoldier::StateToAnimation()
{
	string strAnim;
	switch (m_eState)
	{
	case Client::SOLDIER_SPAWN:
		strAnim = "Chevalier_SPAWN";
		break;
	case Client::SOLDIER_IDLE:
		strAnim = "chevalier_Anim_idle";
		break;
	case Client::SOLDIER_RUN:
		strAnim = "chevalier_Anim_run";
		break;
	case Client::SOLDIER_ATTACK:
		strAnim = "chevalier_Anim_attack";
		break;
	case Client::SOLDIER_SPIN:
		strAnim = "chevalier_tourbilol";
		break;
	case Client::SOLDIER_HIT:
		strAnim = "chevalier_Anim_hit";
		break;
	case Client::SOLDIER_DEATH:
		strAnim = "chevalier_Anim_Death";
		break;
	}

	_bool bLoop;
	if (m_eState == SOLDIER_RUN || m_eState == SOLDIER_IDLE)
		bLoop = true;
	else
		bLoop = false;

	m_pModelCom->Change_Animation(strAnim, bLoop);
}

void CSoldier::Soldier_AnimSpeed()
{
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("chevalier_Anim_attack"), 1.5f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("chevalier_tourbilol"), 1.1f);
}

void CSoldier::DeathEffect()
{
}

_bool CSoldier::IsPlayerInRange()
{
	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));

	if (m_pColliderCom[RANGE]->Intersect(pPlayerCollider))
		return true;

	return _bool();
}

HRESULT CSoldier::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(5.5f, 9.f, 5.5f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom[DEFAULT], &ColliderDesc)))
		return E_FAIL;

	CBounding_Sphere::BOUNDING_SPHERE_DESC	RangeColliderDesc{};
	RangeColliderDesc.fRadius = 11.8f;
	RangeColliderDesc.vCenter = _float3(0.f, RangeColliderDesc.fRadius * 0.5f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider_Range"), (CComponent**)&m_pColliderCom[RANGE], &RangeColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSoldier::Bind_ShaderResources()
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

CSoldier* CSoldier::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSoldier* pInstance = new CSoldier(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSoldier"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CSoldier::Clone(void* pArg)
{
	CSoldier* pInstance = new CSoldier(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSoldier"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSoldier::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);
	for(_int i = 0; i<COLLIDER_END; i++)
		Safe_Release(m_pColliderCom[i]);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
