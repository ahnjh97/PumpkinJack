#include "stdafx.h"
#include "TeslaTower.h"

CTeslaTower::CTeslaTower(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTeslaTower::CTeslaTower(const CTeslaTower& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTeslaTower::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTeslaTower::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	m_iHp = 30;

	return S_OK;
}

void CTeslaTower::Tick(_float fTimeDelta)
{
	if (m_iHp <= 0) {
		Set_Dead();
		return;
	}
		
	if (m_pTransformCom->Get_Pos_Float4().y < 0)
		m_pTransformCom->Fall(-fTimeDelta * 2.5f);

	if (m_pColliderCom != nullptr)
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CTeslaTower::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTeslaTower::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0))) // Default
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pColliderCom)
		m_pColliderCom->Render();
#endif // _DEBUG

	return S_OK;
}

HRESULT CTeslaTower::Add_Components(LEVEL eLevel)
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_TeslaTower");
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.vCenter = _float3(0.f, 5.f, 0.f);
	ColliderDesc.fRadius = 8.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTeslaTower::Bind_ShaderResources()
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

CTeslaTower* CTeslaTower::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTeslaTower* pInstance = new CTeslaTower(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTeslaTower"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CTeslaTower::Clone(void* pArg)
{
	CTeslaTower* pInstance = new CTeslaTower(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTeslaTower"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTeslaTower::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
