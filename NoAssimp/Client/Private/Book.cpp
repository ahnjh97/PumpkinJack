#include "stdafx.h"
#include "Instance_Model.h"
#include "Book.h"

CBook::CBook(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CBook::CBook(const CBook& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CBook::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBook::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; 
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	return S_OK;
}

void CBook::Tick(_float fTimeDelta)
{
	//m_pInstanceModelCom->Spread(fTimeDelta);
}

void CBook::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CBook::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pInstanceModelCom->Get_NumInstanceMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pInstanceModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0))) 
			return E_FAIL;

		m_pInstanceModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CBook::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxInstance_Mesh"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Instance_Model */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Instance_Model"),
		TEXT("Com_Instance_Model"), (CComponent**)&m_pInstanceModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBook::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CBook* CBook::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBook* pInstance = new CBook(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBook"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBook::Clone(void* pArg)
{
	CBook* pInstance = new CBook(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CBook"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBook::Free()
{
	__super::Free();

	Safe_Release(m_pInstanceModelCom);
	Safe_Release(m_pShaderCom);
}
