#include "stdafx.h"
#include "Indicator.h"
#include "Player.h"

CIndicator::CIndicator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CIndicator::CIndicator(const CIndicator& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CIndicator::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CIndicator::Initialize(void* pArg)
{
	INVEN_DESC		IndicatorDesc{};
	if (pArg)
		IndicatorDesc = *(INVEN_DESC*)pArg;

	IndicatorDesc.fSpeedPerSec = 10.f;
	IndicatorDesc.fRotationPerSec = XMConvertToRadians(120.0f);
	m_eWeapon = IndicatorDesc.eWeapon;

	if (FAILED(__super::Initialize(&IndicatorDesc)))
		return E_FAIL;

	m_fZ = IndicatorDesc.matWorld._43;
	if (FAILED(Add_Components(IndicatorDesc.eWeapon)))
		return E_FAIL;


	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;

	m_fSizeX = m_fSizeY = 180;
	m_fX += 540;
	m_fY += 260;

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, m_fZ, 1.f));

	::XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	::XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	_matrix matRotation = XMMatrixRotationAxis(XMVectorSet(0, 0, 1, 0), XMConvertToRadians(-120.f));
	m_vInvenUp[WPN_SPEAR] = XMVectorSet(0, 1, 0, 0);
	m_vInvenUp[WPN_RIFLE] = XMVector3TransformNormal(m_vInvenUp[WPN_SPEAR], matRotation);
	m_vInvenUp[WPN_SCYTHE] = XMVector3TransformNormal(m_vInvenUp[WPN_RIFLE], matRotation);

	return S_OK;
}

void CIndicator::Tick(_float fTimeDelta)
{
	if (m_pPlayer == nullptr)
	{
		CGameObject* pObject = m_pGameInstance->Get_Player(*m_pGameInstance->Get_CurrentLevelID());
		if (pObject != nullptr)
		{
			m_pPlayer = dynamic_cast<CPlayer*>(pObject);
			Safe_AddRef(m_pPlayer);
		}
	}

	_vector vUp = m_pTransformCom->Get_State_Vector(CTransform::STATE_UP);
	_float m_fDegree = XMConvertToDegrees(XMVectorGetY(XMVector3AngleBetweenVectors(vUp, m_vInvenUp[m_pPlayer->Get_Weapon()])));
	if (m_fDegree > 4.f)
	{
		m_pTransformCom->Turn(XMVectorSet(0, 0, 1, 0), -fTimeDelta * m_fDegree * 0.055f);
		m_bTurning = true;
	}
		
	else
		m_bTurning = false;
}

void CIndicator::Late_Tick(_float fTimeDelta)
{
	if(m_pPlayer->Get_Weapon() == m_eWeapon && m_bTurning == false)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CIndicator::Render()
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

HRESULT CIndicator::Add_Components(WEAPON eWeapon)
{
	/* For.Com_Texture */
	wstring wstrTextureTag = TEXT("Prototype_Component_Texture_Indicator");
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrTextureTag, TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_Inven"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CIndicator::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", (2 - m_eWeapon))))
		return E_FAIL;

	return S_OK;
}

CIndicator* CIndicator::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CIndicator* pInstance = new CIndicator(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CIndicator"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CIndicator::Clone(void* pArg)
{
	CIndicator* pInstance = new CIndicator(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CIndicator"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CIndicator::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
