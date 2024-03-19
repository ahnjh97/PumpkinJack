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
	GAMEOBJECT_DESC		GameObjectDesc{};

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

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
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	////Wireframe 모드
	//ID3D11RasterizerState* wireframeState;
	//D3D11_RASTERIZER_DESC rasterizerDesc;
	//ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME; // Wireframe으로 설정
	//rasterizerDesc.FrontCounterClockwise = FALSE;
	//if (FAILED(m_pDevice->CreateRasterizerState(&rasterizerDesc, &wireframeState)))
	//	return E_FAIL;
	//m_pContext->RSSetState(wireframeState);

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	m_pContext->RSSetState(NULL);

	return S_OK;
}

HRESULT CGrid::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture_Diffuse */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Texture_Ground"),
		TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
		return E_FAIL;

	/* For.Com_Texture_Mask */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Texture_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TYPE_MASK])))
		return E_FAIL;

	/* For.Com_Texture_Brush */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_Texture_Brush"),
		TEXT("Com_Texture_Brush"), (CComponent**)&m_pTextureCom[TYPE_BRUSH])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, TEXT("Prototype_Component_VIBuffer_Grid"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;


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

	if (FAILED(m_pTextureCom[TYPE_BRUSH]->Bind_ShaderResource(m_pShaderCom, "g_BrushTexture", 0)))
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
