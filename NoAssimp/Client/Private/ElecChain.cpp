#include "stdafx.h"
#include "ElecChain.h"

CElecChain::CElecChain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CElecChain::CElecChain(const CElecChain& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CElecChain::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElecChain::Initialize(void* pArg)
{
	ELEC_BEAM_DESC tElecBeamDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		tElecBeamDesc = *(ELEC_BEAM_DESC*)pArg;

	m_iLevel = tElecBeamDesc.iLevel;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)tElecBeamDesc.iLevel)))
		return E_FAIL;

	m_iElecBeamIndex = tElecBeamDesc.iElecBeamIndex;

	_float fScale = 10.f;
		m_pTransformCom->Set_Scaled(fScale, fScale, fScale);

	m_pTransformCom->Set_Pos(_float4(tElecBeamDesc.matWorld._41, tElecBeamDesc.matWorld._42, tElecBeamDesc.matWorld._43, 1));

	if (m_iElecBeamIndex < 1) {
		if (FAILED(Add_ElecChain((LEVEL)tElecBeamDesc.iLevel, ++m_iElecBeamIndex)))
			return E_FAIL;
	}

	return S_OK;
}

void CElecChain::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	m_pTransformCom->LookAt_RightFixed(m_pGameInstance->Get_CamPosition_Vector());
}

void CElecChain::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CElecChain::Render()
{
	if (m_bDead)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(6))) // ElecBeam
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CElecChain::Add_ElecChain(LEVEL eLevel, _uint iBeamIndex)
{
	ELEC_BEAM_DESC tElecBeamDesc{};
	tElecBeamDesc.iLevel = eLevel;
	tElecBeamDesc.iElecBeamIndex = iBeamIndex;
	tElecBeamDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();

	_float4 vNewPos{};
	/*if (0 == iBeamIndex)
		XMStoreFloat4(&vNewPos, m_pTransformCom->Get_Pos_Vector() - m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT));
	else if(2 == iBeamIndex)*/
		XMStoreFloat4(&vNewPos, m_pTransformCom->Get_Pos_Vector() + m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT));

	tElecBeamDesc.matWorld._41 = vNewPos.x;
	tElecBeamDesc.matWorld._42 = vNewPos.y;
	tElecBeamDesc.matWorld._43 = vNewPos.z;

	if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_ElecBeam"), TEXT("Prototype_GameObject_ElecBeam"), &tElecBeamDesc)))
		return E_FAIL;
		
	return S_OK;
}

HRESULT CElecChain::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_ElecChain"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_ELEC])))
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

HRESULT CElecChain::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_ELEC]->Bind_ShaderResources(m_pShaderCom, "g_ElecChain")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_ElecNoise")))
		return E_FAIL;

	return S_OK;
}

CElecChain* CElecChain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElecChain* pInstance = new CElecChain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CElecChain"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CElecChain::Clone(void* pArg)
{
	CElecChain* pInstance = new CElecChain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CElecChain"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CElecChain::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pShaderCom);
}
