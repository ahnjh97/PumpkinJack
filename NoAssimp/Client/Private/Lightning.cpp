#include "stdafx.h"
#include "Lightning.h"

CLightning::CLightning(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLightning::CLightning(const CLightning& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CLightning::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLightning::Initialize(void* pArg)
{
	LIGHTNING_DESC tLightningDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		tLightningDesc = *(LIGHTNING_DESC*)pArg;

	m_pTargetWorldMat = tLightningDesc.pTargetWorldMat;
	m_iLevel = tLightningDesc.iLevel;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(tLightningDesc.iLevel), tLightningDesc.iLightingIndex)))
		return E_FAIL;

	m_iLightningIndex = tLightningDesc.iLightingIndex;

	if (tLightningDesc.iLightingIndex == 0 || tLightningDesc.iLightingIndex == 2)
		m_iSpriteEnd = 9;
	else if (tLightningDesc.iLightingIndex == 1)
		m_iSpriteEnd = 20;

	_float fScale = 30.f;
	m_pTransformCom->Set_Scaled(fScale, fScale, fScale);

	if (m_pTargetWorldMat != nullptr)
		m_pTransformCom->Set_Pos(XMVectorSet(m_pTargetWorldMat->_41, m_pTargetWorldMat->_42, m_pTargetWorldMat->_43, 1));
	else
		m_pTransformCom->Set_Pos(_float4(tLightningDesc.matWorld._41, tLightningDesc.matWorld._42 + fScale * 0.48f, tLightningDesc.matWorld._43, 1));

	return S_OK;
}

void CLightning::Tick(_float fTimeDelta)
{
	if (m_iSpriteIndex >= m_iSpriteEnd)
		Set_Dead();

	m_pTransformCom->Look_At(m_pGameInstance->Get_CamPosition_Vector());

	m_fTime += fTimeDelta;

	if (m_fTime > 0.08f) {
		m_iSpriteIndex++;
		m_fTime = 0;
	}
}

void CLightning::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}
HRESULT CLightning::Render()
{
	if (m_bDead)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(5))) // Lightning
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLightning::Add_Components(LEVEL eLevel, _uint _iLightingIndex)
{	
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Lightning"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_LIGHTNING])))
		return E_FAIL;

	/* For.Com_Texture_Noise */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_ElecNoise"),
		TEXT("Com_Texture_Noise"), (CComponent**)&m_pTextureCom[TEX_NOISE])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLightning::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iSpriteIndex", &m_iSpriteIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iLightningIndex", &m_iLightningIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_LIGHTNING]->Bind_ShaderResources(m_pShaderCom, "g_Lightning")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_ElecNoise")))
		return E_FAIL;

	return S_OK;
}

CLightning* CLightning::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLightning* pInstance = new CLightning(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLightning"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLightning::Clone(void* pArg)
{
	CLightning* pInstance = new CLightning(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLightning"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLightning::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pShaderCom);
}
