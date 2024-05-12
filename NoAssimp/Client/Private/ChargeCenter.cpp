#include "stdafx.h"
#include "ChargeCenter.h"

CChargeCenter::CChargeCenter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CChargeCenter::CChargeCenter(const CChargeCenter& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CChargeCenter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CChargeCenter::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	return S_OK;
}

void CChargeCenter::Tick(_float fTimeDelta)
{
	m_pTransformCom->Look_At(m_pGameInstance->Get_CamPosition_Vector());

	m_fTime += fTimeDelta;
	if (m_fTime > m_fEndTime)
		Set_Dead();

	m_fPlayRatio = m_fTime / m_fEndTime;
	if (m_fPlayRatio > 1.f)
		m_fPlayRatio = 1.f;

	m_fScale += exp(m_fTime * fTimeDelta) * fTimeDelta * 1.5f;
	if (m_fScale > 25)
		m_fScale = 25;

	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);
}

void CChargeCenter::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CChargeCenter::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(13))) // ChargeCenter
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CChargeCenter::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_NewTex"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_DEFAULT])))
		return E_FAIL;

	/* For.Com_Texture_Mask */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Particle"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TEX_MASK])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CChargeCenter::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_DEFAULT]->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_MASK]->Bind_ShaderResources(m_pShaderCom, "g_NoiseTexture")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CChargeCenter* CChargeCenter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CChargeCenter* pInstance = new CChargeCenter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CChargeCenter"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CChargeCenter::Clone(void* pArg)
{
	CChargeCenter* pInstance = new CChargeCenter(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CChargeCenter"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CChargeCenter::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	for(_int i = 0; i< TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pVIBufferCom);
}
