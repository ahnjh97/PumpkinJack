#include "stdafx.h"
#include "HitEffect.h"

CHitEffect::CHitEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CHitEffect::CHitEffect(const CHitEffect& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CHitEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHitEffect::Initialize(void* pArg)
{
	HITEFFECT_DESC		GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(HITEFFECT_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_fNewScale = GameObjectDesc.fNewScale;

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(GameObjectDesc.matWorld);

	return S_OK;
}

void CHitEffect::Tick(_float fTimeDelta)
{
	m_pTransformCom->Look_At(m_pGameInstance->Get_CamPosition_Vector());

	m_fTime += fTimeDelta;

	m_fScale += exp(m_fTime * fTimeDelta * 600.f) * fTimeDelta * 5.f;

	if(m_fScale < 4.5f)
		m_pTransformCom->Set_Scaled(m_fScale * 3.f * m_fNewScale, m_fScale * 3.f * m_fNewScale, m_fScale * 3.f * m_fNewScale);

	if (m_fTime > 0.32f)
		Set_Dead();
}

void CHitEffect::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CHitEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(7))) // HitEffect
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CHitEffect::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Hit"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHitEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	return S_OK;
}

CHitEffect* CHitEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHitEffect* pInstance = new CHitEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CHitEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHitEffect::Clone(void* pArg)
{
	CHitEffect* pInstance = new CHitEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CHitEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHitEffect::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
