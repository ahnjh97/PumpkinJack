#include "stdafx.h"
#include "SlimeBossHead.h"
#include <Player.h>

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
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
	m_bInSlimeBossRoom = pPlayer->Get_InSlimeBossRoom();

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

	if (pPlayer && m_eState == HEAD_DASH)
	{
		CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
		if (m_pColliderCom->Intersect(pPlayerCollider))
		{
			if (m_fDamageTime >= 1.f)
			{
				pPlayer->Reduce_HP(20);
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

	if (m_fIdleTime >= 1.8f)
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
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", i, aiTextureType_DIFFUSE);

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pColliderCom->Render();
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
			m_fShakeTime = 0.15f;
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

		CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
		_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
		_float fY = {};
		for (_int i = -30; i < 31; i++)
		{
			_matrix matRotation = XMMatrixRotationZ(XMConvertToRadians(i));
			_vector vNewLook = XMVector3Transform(vLook, matRotation); // 위아래로 회전시킨 Look을 검사
			vNewLook = XMVectorSetW(vNewLook, 0.f);
			if (pPlayerCollider->Intersect_Ray(vPos, XMVector3Normalize(vNewLook), fY))
			{
				XMStoreFloat4(&m_vDashDir, XMVector3Normalize(vNewLook));
				m_eState = HEAD_DASH;
			}
		}
	}

	else if (m_eState == HEAD_DASH) // 돌진
	{
		_float fMaxSpeed = 52.f;
		m_fDashTime += fTimeDelta;
		m_pTransformCom->Move(cos(m_fDashTime * 0.7f) * fMaxSpeed * fTimeDelta, XMLoadFloat4(&m_vDashDir));

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

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
