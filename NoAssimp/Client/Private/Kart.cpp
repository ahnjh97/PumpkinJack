#include "stdafx.h"
#include "Kart.h"
#include "Player.h"
#include "Rail.h"
CKart::CKart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CKart::CKart(const CKart& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CKart::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKart::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	m_vFrontDir = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	m_vRearDir = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	return S_OK;
}

void CKart::Tick(_float fTimeDelta)
{
	if(m_bExplode)
		m_fTime += fTimeDelta;
	
	if (m_fTime >= 3.f)
	{
		list<CGameObject*> crateBoxList = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, TEXT("Layer_CrateBox"));
		if (!crateBoxList.empty())
		{
			for (auto& crateBox : crateBoxList)
			{
				if (crateBox != nullptr) {
					crateBox->Set_Dead();
					Add_Explosion(crateBox);
				}
					
			}
		}
		CGameObject* pMinePlatform = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_MinePlatform"), TEXT("Prototype_GameObject_Monster"));
		if (pMinePlatform != nullptr) {
			pMinePlatform->Set_Dead();
			Add_Explosion(pMinePlatform);
		}
			

		CGameObject* pBallistaBase = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_BallistaBase"), TEXT("Prototype_GameObject_Monster"));
		if (pBallistaBase != nullptr) {
			pBallistaBase->Set_Dead();
			Add_Explosion(pBallistaBase);
		}
			

		CGameObject* pBallista = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Ballista"), TEXT("Prototype_GameObject_Ballista"));
		if (pBallista != nullptr) {
			pBallista->Set_Dead();
			Add_Explosion(pBallista);
		}
			

		CGameObject* pBallistaRotator = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_BallistaRotator"), TEXT("Prototype_GameObject_BallistaRotator"));
		if (pBallistaRotator != nullptr) {
			pBallistaRotator->Set_Dead();
			Add_Explosion(pBallistaRotator);
		}
			
	
		CGameObject* pSkullCrossBow = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_SkullCrossBow"), TEXT("Prototype_GameObject_SkullCrossBow"));
		if (pSkullCrossBow != nullptr) {
			pSkullCrossBow->Set_Dead();
			Add_Explosion(pSkullCrossBow);
		}
			
		Set_Dead();
		Add_Explosion(this);
	}

	for (_int i = 0; i < COLLIDER_END; i++)
	{
		if (m_pColliderCom[i] != nullptr)
			m_pColliderCom[i]->Tick(m_pTransformCom->Get_WorldMatrix());
	}

	if (m_bPush == true)
	{
		m_vDirection = ::XMVector3Normalize(m_vFrontDir + m_vRearDir);
		m_pTransformCom->Move(fTimeDelta, m_vDirection);
		Turn_Kart(fTimeDelta);
	}
}

void CKart::Late_Tick(_float fTimeDelta)
{
	_vector vFrontDir = Check_Direction(fTimeDelta, COLLIDER_FRONT);
	if (!XMVector3Equal(vFrontDir, XMVectorSet(0, 0, 0, 0)))
		m_vFrontDir = vFrontDir;

	_vector vRearDir = Check_Direction(fTimeDelta, COLLIDER_REAR);
	if (!XMVector3Equal(vRearDir, XMVectorSet(0, 0, 0, 0)))
		m_vRearDir = vRearDir;

	if (false == m_bExplode)
		m_bExplode = Check_ExplosionTrigger();

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CKart::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	for (_int i = 0; i < COLLIDER_END; i++)
	{
		if(m_pColliderCom[i] != nullptr)
			m_pGameInstance->Add_DebugComponents(m_pColliderCom[i]);
	}
#endif

	return S_OK;
}

_vector CKart::Check_Direction(_float fTimeDelta, KART_COLLIDER eKartCollider)
{
	list<CGameObject*> railList = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, TEXT("Layer_Rail"));
	if (railList.empty())
		return _vector();

	_int iMax = 0;

	CRail* pTargetRail = { nullptr };
	for (auto& rail : railList)
	{
		if (rail == nullptr)
			continue;

		CCollider* pRailCollider = { nullptr };
		for (_int i = 0; i < 7; i++)
		{
			wstring wstrColliderTag = L"Com_Collider" + to_wstring(i);
			pRailCollider = dynamic_cast<CCollider*>(rail->Get_Component(wstrColliderTag));
			if (pRailCollider == nullptr)
				continue;

			if (m_pColliderCom[eKartCollider]->Intersect(pRailCollider))
			{
				if (i >= iMax)
				{
					pTargetRail = dynamic_cast<CRail*>(rail);
					iMax = i;
				}
			}		
		}
	}

	if(pTargetRail == nullptr)
		return _vector();

	if (m_bPush == true)
	{
		_vector vRight = m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT);
		_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		if (pTargetRail->Get_ParseData().wstrModelTag == TEXT("Prototype_Component_Model_RailLeft60"))
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vRight * fTimeDelta * 0.26f);
		else if (pTargetRail->Get_ParseData().wstrModelTag == TEXT("Prototype_Component_Model_RailRight60"))
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos - vRight * fTimeDelta * 0.26f);
		else if (pTargetRail->Get_ParseData().wstrModelTag == TEXT("Prototype_Component_Model_RailLeft90"))
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos + vRight * fTimeDelta * 0.29f);
		else if (pTargetRail->Get_ParseData().wstrModelTag == TEXT("Prototype_Component_Model_RailRight90"))
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos - vRight * fTimeDelta * 0.29f);
	}
		
	return pTargetRail->Compute_Direction(iMax);
}

void CKart::Turn_Kart(_float fTimeDelta)
{
	_vector	vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, m_vDirection));
	_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, m_vDirection));
	if (fY < 0)
		fAngle = -fAngle;
	if (abs(fAngle) >= XMConvertToRadians(2.f))
		m_pTransformCom->Turn(::XMVectorSet(0, 1, 0, 0), fTimeDelta * fAngle * 7.f);
}

_bool CKart::Check_ExplosionTrigger()
{
	list<CGameObject*> crateBoxList = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, TEXT("Layer_CrateBox"));
	if (crateBoxList.empty())
		return false;

	CCollider* pCrateBoxCollider = { nullptr };
	for (auto& crateBox : crateBoxList)
	{
		if (crateBox == nullptr)
			continue;

		pCrateBoxCollider = dynamic_cast<CCollider*>(crateBox->Get_Component(TEXT("Com_Collider")));
		if (pCrateBoxCollider == nullptr)
			continue;

		if (m_pColliderCom[COLLIDER_WALL]->Intersect(pCrateBoxCollider))
			return true;
	}
		
	return _bool();
}

void CKart::Add_Explosion(CGameObject* pGameObject)
{
	CTransform* pTransform = dynamic_cast<CTransform*>(pGameObject->Get_Component(g_strTransformTag));
	if (nullptr == pTransform)
		return;

	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = m_iLevel;
	tDesc.matWorld = pTransform->Get_WorldFloat4x4();

	if (FAILED(m_pGameInstance->Add_Clone(m_iLevel, TEXT("Layer_Explosion"), TEXT("Prototype_GameObject_Explosion"), &tDesc)))
		return;
}

HRESULT CKart::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Lighted");
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider_Wall */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(8, 8, 9.2f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 1.7f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_Wall"), (CComponent**)&m_pColliderCom[COLLIDER_WALL], &ColliderDesc)))
		return E_FAIL;

	/* For.Com_Collider_Push */
	ColliderDesc.vSize = _float3(6.5f, 8, 8.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, -0.3f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_Push"), (CComponent**)&m_pColliderCom[COLLIDER_PUSH], &ColliderDesc)))
		return E_FAIL;

	/* For.Com_Collider_Front */
	ColliderDesc.vSize = _float3(6.f, 3, 2.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f - 2.f, 2.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_Front"), (CComponent**)&m_pColliderCom[COLLIDER_FRONT], &ColliderDesc)))
		return E_FAIL;

	/* For.Com_Collider_Rear */
	ColliderDesc.vSize = _float3(6.f, 3, 2.f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f - 2.f, -2.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider_Rear"), (CComponent**)&m_pColliderCom[COLLIDER_REAR], &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CKart::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCamFar", m_pGameInstance->Get_CamFarPtr(), sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CKart* CKart::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKart* pInstance = new CKart(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CKart"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CKart::Clone(void* pArg)
{
	CKart* pInstance = new CKart(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CKart"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKart::Free()
{
	__super::Free();

	for (_int i = 0; i < COLLIDER_END; i++)
		Safe_Release(m_pColliderCom[i]);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
