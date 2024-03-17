#include "stdafx.h"
#include "..\Public\BackGround.h"

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
	if (m_fZ == 0.1f)
	{
		m_fSizeX = m_fSizeY = 90;
		m_fX += 520;
		m_fY += 308;
	}
		
	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX - g_iWinSizeX * 0.5f,
		-m_fY + g_iWinSizeY * 0.5f,
		m_fZ,
		1.f
	));

	if (m_fZ == 0.5f)
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
	if (m_fZ == 0.1f)
	{
		_int iSpeed = m_pGameInstance->RandomNumber(1, 10);
		if (iSpeed > 4)
			iSpeed = 0;
		m_pTransformCom->Turn(XMVectorSet(0, 0, 1, 0), -fTimeDelta);
	}
		
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_PRIORITY, this);
}

HRESULT CBackGround::Render()
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE; // 또는 TRUE, 필요에 따라 설정

	// 렌더 타겟에 대한 블렌드 설정 구조체 생성 및 초기화
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = TRUE; // 블렌딩 활성화
	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA; // 소스 블렌드 팩터
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 대상 블렌드 팩터
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD; // 블렌드 연산
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE; // 알파 소스 블렌드 팩터
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO; // 알파 대상 블렌드 팩터
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD; // 알파 블렌드 연산
	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 색상 쓰기 마스크

	// 렌더 타겟에 블렌드 설정 적용
	blendDesc.RenderTarget[0] = rtBlendDesc; // 여러 렌더 타겟에 대한 설정이 필요한 경우 배열에 추가

	ID3D11BlendState* pBlendState = {};
	m_pDevice->CreateBlendState(&blendDesc, &pBlendState);
	_float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pContext->OMSetBlendState(pBlendState, blendFactor, 0xffffffff);

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
		if (m_fZ == 0.5f)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		if (m_fZ == 0.4f)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens_Holder"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		if (m_fZ == 0.3f)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens_Jack"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		if (m_fZ == 0.2f)
		{
			if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_LoadingScreens_GameLogo"),
				TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
				return E_FAIL;
		}
		if (m_fZ == 0.1f)
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

	if (m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureIndex))
		return E_FAIL;

	return S_OK;
}

CBackGround* CBackGround::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _bool bLoadingScreen)
{
	CBackGround* pInstance = new CBackGround(pDevice, pContext, bLoadingScreen);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBackGround"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CBackGround::Clone(void * pArg)
{
	CBackGround*		pInstance = new CBackGround(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CBackGround"));

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
