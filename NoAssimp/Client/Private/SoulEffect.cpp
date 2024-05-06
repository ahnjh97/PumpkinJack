#include "stdafx.h"
#include "SoulEffect.h"

CSoulEffect::CSoulEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSoulEffect::CSoulEffect(const CSoulEffect& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSoulEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSoulEffect::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(GameObjectDesc.matWorld);

	/*_float fScale = 20.f;
	m_pTransformCom->Set_Scaled(fScale, fScale, fScale);*/

	return S_OK;
}

void CSoulEffect::Tick(_float fTimeDelta)
{
	m_pTransformCom->Look_At(m_pGameInstance->Get_CamPosition_Vector());

	m_fTime += fTimeDelta;
	if (m_fTime > m_fEndTime)
		Set_Dead();

	m_fPlayRatio = m_fTime / m_fEndTime;
	if (m_fPlayRatio > 1.f)
		m_fPlayRatio = 1.f;

	m_fAccSpeed += exp(m_fTime * fTimeDelta) * fTimeDelta;
	m_pTransformCom->Fall(-m_fAccSpeed * 0.03f);

	if(m_fPlayRatio > 0.3f)
		m_fAccScale += exp(m_fTime * fTimeDelta * 180.f) * fTimeDelta;

	m_fScale += exp(m_fTime * fTimeDelta * 600.f) * fTimeDelta * 5.f;
	if (m_fScale > 20)
		m_fScale = 20;

	m_pTransformCom->Set_Scaled(m_fScale, m_fScale + m_fAccScale, m_fScale);
}

void CSoulEffect::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CSoulEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(8))) // SoulEffect
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSoulEffect::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Soul"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSoulEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_SoulTexture")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CSoulEffect* CSoulEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSoulEffect* pInstance = new CSoulEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSoulEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSoulEffect::Clone(void* pArg)
{
	CSoulEffect* pInstance = new CSoulEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSoulEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSoulEffect::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
