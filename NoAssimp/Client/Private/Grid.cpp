#include "stdafx.h"
#include "Grid.h"

CGrid::CGrid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CGrid::CGrid(const CGrid& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CGrid::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGrid::Initialize(void* pArg)
{
	GRID_DESC		GridDesc{};
	if (pArg)
		GridDesc = *(GRID_DESC*)pArg;
	GridDesc.fSpeedPerSec = 10.f;
	GridDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_iGridIndex = GridDesc.iGridIndex;
	
	if (FAILED(__super::Initialize(&GridDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(GridDesc.wstrCurLevel)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(GridDesc.matWorld);
	return S_OK;
}

void CGrid::Tick(_float fTimeDelta)
{
	
}

void CGrid::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CGrid::Render()
{
	if (m_bHide)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iPassIndex)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGrid::Add_Components(const wstring& wstrCurLevel)
{
	///* For.Com_Shader */
	//if (wstrCurLevel == TEXT("Level_GamePlay"))
	//{
	//	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Shader_VtxNorTex_"),
	//		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
	//		return E_FAIL;
	//}
	//else {
		if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Shader_VtxNorTex"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
			return E_FAIL;
	//}



	/* For.Com_Texture_Diffuse */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Texture_Ground"),
		TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
		return E_FAIL;

	/* For.Com_Texture_Mask */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Texture_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TYPE_MASK])))
		return E_FAIL;

	/* For.Com_Texture_Water */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Texture_Water"),
		TEXT("Com_Texture_Water"), (CComponent**)&m_pTextureCom[TYPE_WATER])))
		return E_FAIL;

	if (m_iGridIndex == 1)
	{
		/* For.Com_VIBuffer */
		if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_VIBuffer_Terrain_SlimeBossRoom"),
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
			return E_FAIL;
	}
	else
	{
		wstring wstrLevelName = wstrCurLevel.substr(6);
		/* For.Com_VIBuffer */
		if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_VIBuffer_Terrain_" + wstrLevelName),
			TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CGrid::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TYPE_DIFFUSE]->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TYPE_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TYPE_WATER]->Bind_ShaderResource(m_pShaderCom, "g_WaterTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

CGrid* CGrid::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CGrid* pInstance = new CGrid(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CGrid"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CGrid::Clone(void* pArg)
{
	CGrid* pInstance = new CGrid(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CGrid"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CGrid::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for (size_t i = 0; i < TYPE_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pVIBufferCom);
}
