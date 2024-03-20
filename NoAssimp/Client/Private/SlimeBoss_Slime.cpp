#include "stdafx.h"
#include "SlimeBoss_Slime.h"

CSlimeBoss_Slime::CSlimeBoss_Slime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPart{ pDevice, pContext }
{
}

CSlimeBoss_Slime::CSlimeBoss_Slime(const CSlimeBoss_Slime& rhs)
	: CPart{ rhs }
{

}

HRESULT CSlimeBoss_Slime::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlimeBoss_Slime::Initialize(void* pArg)
{
	SLIMEBOSS_SLIME_DESC* pSlimeBossDesc = (SLIMEBOSS_SLIME_DESC*)pArg;

	m_pState = pSlimeBossDesc->pState;
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(pSlimeBossDesc->iLevel), pSlimeBossDesc->wstrModelTag)))
		return E_FAIL;

	m_pModelCom->Set_Animation(SLIMEBOSS_SPAWN, false);
	m_pModelCom->Set_LerpTime(0.2f);
	return S_OK;
}

void CSlimeBoss_Slime::Tick(_float fTimeDelta)
{
	if (*m_pState == SLIMEBOSS_DEATH)
		m_pModelCom->Shrink();
	if (m_pModelCom->Get_Shrinked())
		Set_Dead();
	StateToAnimation();
}

void CSlimeBoss_Slime::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CSlimeBoss_Slime::Render()
{
	_float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pContext->OMSetBlendState(m_pGameInstance->Get_BlendState(), blendFactor, 0xffffffff);

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if(FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;
		if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_EmitTexture")))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CSlimeBoss_Slime::StateToAnimation()
{
	_bool bLoop = true;
	_uint iIndex = *m_pState;
	if (*m_pState == SLIMEBOSS_SPAWN || *m_pState == SLIMEBOSS_INVOKE_BEGIN || *m_pState == SLIMEBOSS_DEATH || *m_pState == SLIMEBOSS_FORK
		|| *m_pState == SLIMEBOSS_PILLAR_SPAWN || *m_pState == SLIMEBOSS_MEGASPAWN)
		bLoop = false;
	m_pModelCom->Change_Animation_ByIndex(iIndex, bLoop);
}

HRESULT CSlimeBoss_Slime::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_SlimeBoss_Emit"),
		TEXT("Com_Texture_Emit"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel_Slime");
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeBoss_Slime::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
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

CSlimeBoss_Slime* CSlimeBoss_Slime::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlimeBoss_Slime* pInstance = new CSlimeBoss_Slime(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBoss_Slime"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CSlimeBoss_Slime::Clone(void* pArg)
{
	CSlimeBoss_Slime* pInstance = new CSlimeBoss_Slime(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBoss_Slime"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSlimeBoss_Slime::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
}
