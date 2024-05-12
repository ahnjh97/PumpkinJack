#include "stdafx.h"
#include "FadeInOut.h"

CFadeInOut::CFadeInOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CFadeInOut::CFadeInOut(const CFadeInOut& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CFadeInOut::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CFadeInOut::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_fZ = 0.1f;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX - g_iWinSizeX * 0.5f,
		-m_fY + g_iWinSizeY * 0.5f,
		m_fZ,
		1.f
	));

	::XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	::XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	m_fEndTime = 2.2f;

	return S_OK;
}

void CFadeInOut::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	m_fPlayRatio = m_fTime / m_fEndTime;
	if (m_fPlayRatio > 1.f) {
		m_fPlayRatio = 1.f;
		Set_Dead();
	}
}

void CFadeInOut::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CFadeInOut::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */

	if (FAILED(m_pShaderCom->Begin(12)))  // FadeInOut
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CFadeInOut::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CFadeInOut::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CFadeInOut* CFadeInOut::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFadeInOut* pInstance = new CFadeInOut(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CFadeInOut"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CFadeInOut::Clone(void* pArg)
{
	CFadeInOut* pInstance = new CFadeInOut(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CFadeInOut"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFadeInOut::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);
}
