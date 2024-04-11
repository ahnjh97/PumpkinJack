#include "stdafx.h"
#include "..\Public\BackGround.h"
#define fZ_LoadingScreen 0.9f
#define fZ_Holder 0.8f
#define fZ_Jack 0.7f
#define fZ_GameLogo 0.6f
#define fZ_Spinner 0.5f

CBackGround::CBackGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _bool bLoadingScreen)
	: CGameObject{ pDevice, pContext }, m_bLoadingScreen{ bLoadingScreen }
{
}

CBackGround::CBackGround(const CBackGround & rhs)
	: CGameObject{ rhs }, m_bLoadingScreen{ rhs.m_bLoadingScreen }, m_fZ{ rhs.m_fZ }
{

}

HRESULT CBackGround::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBackGround::Initialize(void * pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if(pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	m_fZ = GameObjectDesc.matWorld._43;
	if (FAILED(Add_Components()))
		return E_FAIL;

	m_fSizeX = g_iWinSizeX;
	m_fSizeY = g_iWinSizeY;
	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;
	if (m_fZ == fZ_Spinner)
	{
		m_fSizeX = m_fSizeY = 70;
		m_fX += 570;
		m_fY += 308;
	}
		
	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX - g_iWinSizeX * 0.5f,
		-m_fY + g_iWinSizeY * 0.5f,
		m_fZ,
		1.f
	));

	if (m_fZ == fZ_LoadingScreen)
		m_iTextureIndex = m_pGameInstance->RandomNumber(0,5);

	::XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());

	::XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	return S_OK;
}

void CBackGround::Tick(_float fTimeDelta)
{
}

void CBackGround::Late_Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
	float currentSpeed = 0.5f / (log(m_fTime + 1) * 1.f + 1) / 30;
	if (m_fZ == fZ_Jack)
		m_pTransformCom->Go_Right(currentSpeed);
	else if (m_fZ == fZ_GameLogo)
		m_pTransformCom->Go_Left(currentSpeed);
	else if (m_fZ == fZ_Spinner)
		m_pTransformCom->Turn(XMVectorSet(0, 0, 1, 0), -fTimeDelta * 1.1f);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_PRIORITY, this);
}

HRESULT CBackGround::Render()
{
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

HRESULT CBackGround::Add_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"), 
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	if (m_bLoadingScreen == false)
	{
		/* For.Com_Texture */
		if (FAILED(__super::Add_Component(LEVEL_LOGO, TEXT("Prototype_Component_Texture_Logo"),
			TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
			return E_FAIL;
	}
	else
	{
		if (m_fZ == fZ_LoadingScreen)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		else if (m_fZ == fZ_Holder)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens_Holder"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		else if (m_fZ == fZ_Jack)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens_Jack"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		else if (m_fZ == fZ_GameLogo)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens_GameLogo"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		else if (m_fZ == fZ_Spinner)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Spinner"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
	}

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBackGround::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
	//	return E_FAIL;
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_iTextureIndex))
		return E_FAIL;

	return S_OK;
}

CBackGround* CBackGround::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _bool bLoadingScreen)
{
	CBackGround* pInstance = new CBackGround(pDevice, pContext, bLoadingScreen);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBackGround"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBackGround::Clone(void * pArg)
{
	CBackGround*		pInstance = new CBackGround(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CBackGround"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBackGround::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
