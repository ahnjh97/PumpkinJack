#include "stdafx.h"
#include "Soldier.h"
#include "SoldierWeapon.h"

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

	if (FAILED(Add_PartObjects(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	if (m_pModelCom != nullptr)
		Soldier_AnimSpeed();

	m_pModelCom->Set_Animation(10, false); // SPAWN
	m_iPreHp = m_iHp = 40;
	m_pNavigationCom->Find_CurrentIndex(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
	return S_OK;
}

void CSoldier::Tick(_float fTimeDelta)
{		
	if (nullptr == m_pPlayerWorldMat) {
		Get_PlayerWorldMatPtr();
		if (nullptr == m_pPlayerWorldMat)
			return;
	}

	if (m_eState == SOLDIER_IDLE)
		m_fIdleTime += fTimeDelta;
	else
		m_fIdleTime = 0.f;

	Check_KartCol(fTimeDelta);

	LinkState();
	StateToAnimation();

	if (m_eState == SOLDIER_IDLE || m_eState == SOLDIER_RUN || m_eState == SOLDIER_SPAWN || m_eState == SOLDIER_SPIN)	{
		if (true == m_pModelCom->Get_IsLerping())		{
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
		else {
			if (m_eState == SOLDIER_SPIN) {
				_float fPlayRatio = m_pModelCom->Get_AnimPlayRatio();
				if (fPlayRatio >= 0.4f && fPlayRatio <= 0.7f)
					m_pTransformCom->Look_At_ForLandObject(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1));
			}
			else
				m_pTransformCom->Look_At_ForLandObject(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1));
		}
	}
	else if (SOLDIER_HIT == m_eState) {
		if(m_pModelCom->Get_AnimPlayRatio() < 0.15f)
			m_pTransformCom->Look_At_ForLandObject(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1));
	}



	if (m_eState == SOLDIER_RUN)
		m_pTransformCom->Move(fTimeDelta * 2.8f, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), m_pNavigationCom);

	else if (m_eState == SOLDIER_SPIN)
	{
		_float fPlayRatio = m_pModelCom->Get_AnimPlayRatio();
		if (fPlayRatio >= 0.4f && fPlayRatio <= 0.7f) {
			if(CalculateDistanceFromPlayer() > 5.f)
				m_pTransformCom->Move(fTimeDelta * 1.9f, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), m_pNavigationCom);
		}
	}

	m_pTransformCom->Set_Height(m_pNavigationCom);

	if (SOLDIER_ATTACK == m_eState)
		m_iDamage = 10;
	else if (SOLDIER_SPIN == m_eState)
		m_iDamage = 5;
	
	if(SOLDIER_ATTACK == m_eState || SOLDIER_SPIN == m_eState)
		ActivateSoldierWeapon();

	m_pPart_Weapon->Tick(fTimeDelta);

	for(_int i = 0; i < COLLIDER_END; i++)
		m_pColliderCom[i]->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CSoldier::Late_Tick(_float fTimeDelta)
{
	m_pPart_Weapon->Late_Tick(fTimeDelta);
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_iPreHp = m_iHp;

	if ((SOLDIER_ATTACK == m_eState || SOLDIER_SPIN == m_eState) && m_pModelCom->isFinished()) {
		CSoldierWeapon* pSoldierWeapon = dynamic_cast<CSoldierWeapon*>(m_pPart_Weapon);
		pSoldierWeapon->Reset_Damagable();
	}
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
	m_pColliderCom[DEFAULT]->Render();
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
	Safe_AddRef(m_pPlayer);
}

void CSoldier::LinkState()
{
	if (m_eState == SOLDIER_DEATH)
	{
		if (m_pModelCom->isFinished())
		{
			DeathEffect();
			Set_Dead();
		}
		return;
	}

	if (m_iHp<=0)
	{
		m_eState = SOLDIER_DEATH;
		return;
	}

	if (m_iPreHp != m_iHp)
	{
		if (SOLDIER_HIT == m_eState)
			m_pModelCom->Reset_Animation(string("chevalier_Anim_hit"));

		if (SOLDIER_SPIN == m_eState) {
			if(m_pModelCom->Get_AnimPlayRatio() > 0.7f)
				m_eState = SOLDIER_HIT;
		}
		else
			m_eState = SOLDIER_HIT;
	}

	if (m_eState == SOLDIER_IDLE && m_fIdleTime >= 0.2f && m_iHp > 0)
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

	if ((m_eState == SOLDIER_ATTACK || m_eState == SOLDIER_SPIN || m_eState == SOLDIER_SPAWN || m_eState == SOLDIER_HIT) && m_pModelCom->isFinished())
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
		strAnim = "chevalier_Anim_death";
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
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("chevalier_Anim_attack"), 1.1f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("chevalier_tourbilol"), 1.1f);
}

void CSoldier::DeathEffect()
{
}

void CSoldier::ActivateSoldierWeapon()
{
	CSoldierWeapon* pSoldierWeapon = dynamic_cast<CSoldierWeapon*>(m_pPart_Weapon);
	pSoldierWeapon->DeActivate_WeaponCollider();
	_float fAnimPlayRatio = m_pModelCom->Get_AnimPlayRatio();

	if (SOLDIER_ATTACK == m_eState)
	{
		if (fAnimPlayRatio > 0.2f && fAnimPlayRatio < 0.24f)
			pSoldierWeapon->Activate_WeaponCollider();
	}
	else if (SOLDIER_SPIN == m_eState)
	{
		if (fAnimPlayRatio < 0.3f)
			m_bResetSpinAttack = true;

		if (fAnimPlayRatio > 0.32f && fAnimPlayRatio < 0.75f) // 0.5f 마다 초기화. 총 7번 초기화
			pSoldierWeapon->Activate_WeaponCollider();

		if ((fAnimPlayRatio > 0.38f && fAnimPlayRatio < 0.43f) || (fAnimPlayRatio > 0.48f && fAnimPlayRatio < 0.53f)
			|| (fAnimPlayRatio > 0.58f && fAnimPlayRatio < 0.63f) || (fAnimPlayRatio > 0.68f && fAnimPlayRatio < 0.73f)) {
			if (m_bResetSpinAttack) {
				pSoldierWeapon->Reset_Damagable();
				m_bResetSpinAttack = false;
			}
		}
		else if ((fAnimPlayRatio > 0.43f && fAnimPlayRatio < 0.48f) || (fAnimPlayRatio > 0.53f && fAnimPlayRatio < 0.58f)
			|| (fAnimPlayRatio > 0.63f && fAnimPlayRatio < 0.68f)) {
			if (!m_bResetSpinAttack) {
				pSoldierWeapon->Reset_Damagable();
				m_bResetSpinAttack = true;
			}
		}
	}
}

_bool CSoldier::IsPlayerInRange()
{
	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));

	if (m_pColliderCom[RANGE]->Intersect(pPlayerCollider))
		return true;

	return _bool();
}

_bool CSoldier::Check_KartCol(_float fTimeDelta)
{
	CGameObject* pKart = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Kart"), TEXT("Prototype_GameObject_Kart"));
	if (nullptr == pKart)
		return false;

	CCollider* pKartCollider = dynamic_cast<CCollider*>(pKart->Get_Component(TEXT("Com_Collider_Wall")));
	if (nullptr == pKartCollider)
		return false;

	_float3 vSrcPosFloat = m_pColliderCom[DEFAULT]->Get_Center(CCollider::TYPE_OBB);
	_vector vSrsPos = XMLoadFloat3(&vSrcPosFloat);
	if (m_pColliderCom[DEFAULT]->Intersect(pKartCollider))	{
		_float3 vDstPosFloat = pKartCollider->Get_Center(CCollider::TYPE_OBB);
		_vector vDstPos = XMLoadFloat3(&vDstPosFloat);

		_vector vDir = XMVector3Normalize(vSrsPos - vDstPos);

		m_pTransformCom->Move(fTimeDelta * 3.f, vDir, m_pNavigationCom);
	}
	return _bool();
}

_float CSoldier::CalculateDistanceFromPlayer()
{
	_vector vPlayerPos = XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1);
	_vector vSoldierPos = m_pTransformCom->Get_Pos_Vector();

	_vector vSubtraction = vPlayerPos - vSoldierPos;

	_float fDis = XMVectorGetX(XMVector3Length(vSubtraction));
	return fDis;
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

	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC			NavigationDesc{};
	NavigationDesc.iCurrentIndex = 0;
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Navigation") + to_wstring(eLevel),
		TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom, &NavigationDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSoldier::Add_PartObjects(LEVEL eLevel)
{
	/* For.Part_SoldierWeapon */
	CPart* pSoldierWeapon = { nullptr };
	CSoldierWeapon::SOLDIER_WEAPON_DESC	SoldierWeaponDesc{};
	SoldierWeaponDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SoldierWeaponDesc.pSocket = m_pModelCom->Get_BonePtr("arme");
	SoldierWeaponDesc.pState = &m_eState;
	SoldierWeaponDesc.pDamage = &m_iDamage;
	SoldierWeaponDesc.iLevel = eLevel;
	pSoldierWeapon = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_SoldierWeapon"), &SoldierWeaponDesc));
	if (nullptr == pSoldierWeapon)
		return E_FAIL;

	m_pPart_Weapon = pSoldierWeapon;
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
	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pPart_Weapon);
	for(_int i = 0; i<COLLIDER_END; i++)
		Safe_Release(m_pColliderCom[i]);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
