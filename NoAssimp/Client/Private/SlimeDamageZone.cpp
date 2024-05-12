#include "stdafx.h"
#include "SlimeDamageZone.h"
#include <Player.h>

CSlimeDamageZone::CSlimeDamageZone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSlimeDamageZone::CSlimeDamageZone(const CSlimeDamageZone& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSlimeDamageZone::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlimeDamageZone::Initialize(void* pArg)
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

	m_pTransformCom->Rotation(XMVectorSet(0, 1, 0, 0), XMConvertToRadians(m_pGameInstance->RandomNumber(0, 360)));
	return S_OK;
}

void CSlimeDamageZone::Tick(_float fTimeDelta)
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
	if (pPlayer->Get_InSlimeBossRoom())
		Set_Dead();

	if (m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y <= 0)
		m_pTransformCom->Fall(-fTimeDelta * 1.1f);

	m_fTime += fTimeDelta;
	m_fDamageTime += fTimeDelta;

	if (pPlayer)
	{
		CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
		if (m_pColliderCom->Intersect(pPlayerCollider))
		{
			if (m_fDamageTime >= 1.f)
			{
				pPlayer->Deduct_HP(10);
				m_fDamageTime = 0.f;
			}
		}
	}

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CSlimeDamageZone::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CSlimeDamageZone::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(12))) // Knife
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif

	return S_OK;
}

HRESULT CSlimeDamageZone::Add_Components(const wstring& _wstrModelTag)
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
	ColliderDesc.fRadius = 12.5f;
	ColliderDesc.vCenter = _float3(0.f, 0, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeDamageZone::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
		return E_FAIL;

	return S_OK;
}

CSlimeDamageZone* CSlimeDamageZone::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlimeDamageZone* pInstance = new CSlimeDamageZone(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeDamageZone"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CSlimeDamageZone::Clone(void* pArg)
{
	CSlimeDamageZone* pInstance = new CSlimeDamageZone(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeDamageZone"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSlimeDamageZone::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
