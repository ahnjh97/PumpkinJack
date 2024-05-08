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
	ELEC_CHAIN_DESC tElecChainDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		tElecChainDesc = *(ELEC_CHAIN_DESC*)pArg;

	m_iLevel = tElecChainDesc.iLevel;
	tElecChainDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&tElecChainDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(tElecChainDesc.iLevel))))
		return E_FAIL;

	m_iElecChainIndex = tElecChainDesc.iElecChainIndex;

	_float fScale = 28.f;
		m_pTransformCom->Set_Scaled(fScale, fScale * 1.2f, fScale);

	m_pTransformCom->Set_Pos(_float4(tElecChainDesc.matWorld._41, tElecChainDesc.matWorld._42, tElecChainDesc.matWorld._43, 1));
	m_pTransformCom->Look_At(tElecChainDesc.vReactorPos);
	m_pTransformCom->Turn(XMVector4Normalize(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP)), 1);

	m_RandomNumber = mt19937_64(m_RandomDevice());

	Randomize_Chain();

	return S_OK;
}

void CElecChain::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	if (m_fChangeTime < m_fTime)
		Randomize_Chain();

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

	if (FAILED(m_pShaderCom->Begin(6))) // ElecChain
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CElecChain::Randomize_Chain()
{
	m_iElecChainIndex = m_pGameInstance->RandomNumber(0, 7);
	m_fTime = 0;
	uniform_real_distribution<_float> ChangeTime(0.04f, 0.08f);
	m_fChangeTime = ChangeTime(m_RandomNumber);
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iElecChainIndex", &m_iElecChainIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_ELEC]->Bind_ShaderResources(m_pShaderCom, "g_ElecChain")))
		return E_FAIL;
	/*if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_ElecNoise")))
		return E_FAIL;*/

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
