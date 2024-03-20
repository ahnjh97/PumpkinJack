#include "stdafx.h"
#include "LifeBar.h"

CLifeBar::CLifeBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLifeBar::CLifeBar(const CLifeBar& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CLifeBar::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLifeBar::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_fZ = GameObjectDesc.matWorld._43;
	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;
	if (m_fZ == fZ_LifeBarMask)
	{
		m_fSizeX = 320;
		m_fSizeY = 55;
		m_fX -= 430;
		m_fY += 290;
	}
	//else if (m_fZ == fZ_LifeBar)
	//{
	//	m_fSizeX = m_fSizeY = 340;
	//	m_fX -= 430;
	//	m_fY += 300;
	//}
	else if (m_fZ == fZ_CrowLifeBar)
	{
		m_fSizeX = 323.5f;
		m_fSizeY = 100;
		m_fX -= 426.8f;
		m_fY += 256.f;
	}

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX - g_iWinSizeX * 0.5f,
		-m_fY + g_iWinSizeY * 0.5f,
		m_fZ,
		1.f
	));

	::XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	::XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	return S_OK;
}

void CLifeBar::Tick(_float fTimeDelta)
{
	
	m_fOffset_Y += 0.002f;
}

void CLifeBar::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);	
}

HRESULT CLifeBar::Render()
{
	_float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pContext->OMSetBlendState(m_pGameInstance->Get_BlendState(), blendFactor, 0xffffffff);

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLifeBar::Add_Components()
{
	if (m_fZ == fZ_LifeBarMask)
	{
		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LifeBarMask"),
			TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TYPE_MASK])))
			return E_FAIL;
		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LifeBar"),
			TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
			return E_FAIL;

		/* For.Com_Shader */
		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_LifeBar"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
			return E_FAIL;
	}
	else if (m_fZ == fZ_CrowLifeBar)
	{
		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_CrowLifeBar"),
			TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
			return E_FAIL;
		/* For.Com_Shader */
		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_CrowLifeBar"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
			return E_FAIL;
	}

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLifeBar::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
	//	return E_FAIL;
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TYPE_DIFFUSE]->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (m_fZ == fZ_LifeBarMask)
	{
		if (FAILED(m_pTextureCom[TYPE_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_Offset_Y", &m_fOffset_Y, sizeof(_float))))
			return E_FAIL;
	}

	return S_OK;
}



CLifeBar* CLifeBar::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLifeBar* pInstance = new CLifeBar(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLifeBar"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLifeBar::Clone(void* pArg)
{
	CLifeBar* pInstance = new CLifeBar(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CLifeBar"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLifeBar::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for (size_t i = 0; i < TYPE_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pVIBufferCom);
}
