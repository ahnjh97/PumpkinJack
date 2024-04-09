#include "stdafx.h"
#include "Arrow.h"
#include "Ballista.h"
#include "Player.h"

CArrow::CArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CArrow::CArrow(const CArrow& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CArrow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CArrow::Initialize(void* pArg)
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

void CArrow::Tick(_float fTimeDelta)
{
	if (m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y <= -15.f)
		Set_Dead();

	m_pTransformCom->Go_Straight(fTimeDelta * 16.f);

	m_fTime += fTimeDelta;

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_GAMEPLAY));
	if (pPlayer) 
	{
		CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
		if (!m_pColliderCom->Check_AlreadyCollided(pPlayerCollider)) 
		{
			if (m_pColliderCom->Intersect(pPlayerCollider))
				pPlayer->Deduct_HP(15);
		}
	}

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CArrow::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CArrow::Render()
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
	m_pColliderCom->Render();
#endif

	return S_OK;
}

HRESULT CArrow::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Lighted");
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(2, 2, 24.f);
	ColliderDesc.vCenter = _float3(0.f, 0.f, -ColliderDesc.vSize.z * 0.5f + 6.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CArrow::Bind_ShaderResources()
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

CArrow* CArrow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CArrow* pInstance = new CArrow(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CArrow"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CArrow::Clone(void* pArg)
{
	CArrow* pInstance = new CArrow(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CArrow"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CArrow::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
