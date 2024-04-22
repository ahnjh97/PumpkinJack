#include "stdafx.h"
#include "SlimeBossHead.h"
#include "Player.h"

CSlimeBossHead::CSlimeBossHead(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSlimeBossHead::CSlimeBossHead(const CSlimeBossHead& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSlimeBossHead::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlimeBossHead::Initialize(void* pArg)
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

	return S_OK;
}

void CSlimeBossHead::Tick(_float fTimeDelta)
{
	if (m_pPlayer == nullptr)
	{
		m_pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
		m_pPlayerWorldMat = m_pPlayer->Get_PlayerWorldMatrixPtr();
		if (m_pPlayer != nullptr)
			Safe_AddRef(m_pPlayer);
	}

	if (m_pPlayer == nullptr)
		return;

	m_bInSlimeBossRoom = m_pPlayer->Get_InSlimeBossRoom();

	if (m_bInSlimeBossRoom == false && m_bPreInSlimeBossRoom == true) // 플레이어가 SlimeBossRoom 탈출시
	{
		Set_Dead();
		return;
	}
	
	if (m_bInSlimeBossRoom == false)
		return;

	m_fTime += fTimeDelta;
	m_fDamageTime += fTimeDelta;
	if (m_eState == HEAD_IDLE)
		m_fIdleTime += fTimeDelta;
	else
		m_fIdleTime = 0.f;

	m_bCollision = { false };

	if (m_pPlayer && m_eState == HEAD_DASH)
	{
		CCollider* pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));
		if (m_pColliderCom->Intersect(pPlayerCollider))
		{
			if (m_fDamageTime >= 1.f)
			{
				m_pPlayer->Deduct_HP(20);
				m_fDamageTime = 0.f;
				m_bCollision = true;
			}
		}
	}

	list<CGameObject*> pillar2List = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar2"));
	if (!pillar2List.empty())
	{
		for (auto& pillar2 : pillar2List)
		{
			if (pillar2 == nullptr)
				continue;

			CCollider* pPillar2Collider = dynamic_cast<CCollider*>(pillar2->Get_Component(TEXT("Com_Collider")));

			if (m_pColliderCom->Intersect(pPillar2Collider))
			{
				m_bCollision = true;
				break;
			}
		}
	}

	if (m_bCollision) // 방 밖으로 못가도록 제한시켜야함 
		m_eState = HEAD_IDLE;

	if (m_fIdleTime >= 1.5f)
		m_eState = HEAD_TURN;

	StateToAction(fTimeDelta);

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CSlimeBossHead::Late_Tick(_float fTimeDelta)
{
	m_bPreInSlimeBossRoom = m_bInSlimeBossRoom;
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSlimeBossHead::Render()
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
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif
	return S_OK;
}

void CSlimeBossHead::StateToAction(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
	if (m_eState == HEAD_IDLE) // 도리도리
	{
		m_fShakeTime += fTimeDelta;
		_float fAngle = 0.34f;
		if (m_ePreState != HEAD_IDLE)
		{
			m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), fAngle * 0.5f);
			m_fShakeTime = 0.1f;
			m_fShakeDir = -1.f;
		}
			
		if (m_fShakeTime >= 0.2f)
		{
			m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), fAngle * m_fShakeDir);
			m_fShakeTime = 0.f;
			m_fShakeDir = -m_fShakeDir;
		}
	}

	else if (m_eState == HEAD_TURN) // 회전
	{
		m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), fTimeDelta * 2.f);
		_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
		_vector vPlayerPos = XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1.f);

		_float4 vTargetRight, vTargetLook;
		m_pTransformCom->Get_Look_LandObject(vPlayerPos, vTargetRight, vTargetLook);

		_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, XMLoadFloat4(&vTargetLook)));
		_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, XMLoadFloat4(&vTargetLook)));
		if (fY < 0)
			fAngle = -fAngle;
		if (abs(fAngle) <= XMConvertToRadians(2.f))
		{
			m_eState = HEAD_DASH;
			m_pTransformCom->Look_At(vPlayerPos);
		}
	}

	else if (m_eState == HEAD_DASH) // 돌진
	{
		_float fMaxSpeed = 52.f;
		m_fDashTime += fTimeDelta;
		m_pTransformCom->Move(cos(m_fDashTime * 0.7f) * fMaxSpeed * fTimeDelta, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), m_pNavigationCom);

		if (m_fDashTime >= 1.f || m_bCollision)
		{
			m_eState = HEAD_IDLE;
			m_fDashTime = 0.f;
		}
	}

	m_ePreState = m_eState;
}

HRESULT CSlimeBossHead::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.fRadius = 6.f;
	ColliderDesc.vCenter = _float3(0.f, 0, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC	NavigationDesc{};
	NavigationDesc.iCurrentIndex = 0;

	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, TEXT("Prototype_Component_Navigation_SlimeBossRoom"),
		TEXT("Com_Navigation_SlimeBossRoom"), (CComponent**)&m_pNavigationCom, &NavigationDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeBossHead::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CSlimeBossHead* CSlimeBossHead::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlimeBossHead* pInstance = new CSlimeBossHead(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBossHead"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CSlimeBossHead::Clone(void* pArg)
{
	CSlimeBossHead* pInstance = new CSlimeBossHead(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBossHead"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSlimeBossHead::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);
	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
