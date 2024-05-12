#include "stdafx.h"
#include "ChargeEffect.h"
#include "Player.h"

CChargeEffect::CChargeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CChargeEffect::CChargeEffect(const CChargeEffect& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CChargeEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CChargeEffect::Initialize(void* pArg)
{
	GAMEOBJECT_DESC tElecPartsDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		tElecPartsDesc = *(GAMEOBJECT_DESC*)pArg;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(tElecPartsDesc.iLevel))))
		return E_FAIL;

	m_vPivot = m_pVIBufferCom->Get_Pivot();

	return S_OK;
}

void CChargeEffect::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	if (m_fDuration != 0 && m_fTime > m_fDuration)
		Set_Dead();

	m_pVIBufferCom->MoveToPivot(fTimeDelta);
}

void CChargeEffect::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CChargeEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(1))) // ChargeEffect
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CChargeEffect::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point_Bullet"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Bullet"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_VIBuffer_Instance_Point_ChargeEffect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CChargeEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vPivot", &m_vPivot, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_AttackEffect")))
		return E_FAIL;

	return S_OK;
}

CChargeEffect* CChargeEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CChargeEffect* pInstance = new CChargeEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CChargeEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CChargeEffect::Clone(void* pArg)
{
	CChargeEffect* pInstance = new CChargeEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CChargeEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChargeEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
}
