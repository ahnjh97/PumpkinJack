#include "stdafx.h"
#include "Level_Loading.h"
#include "Loader.h"
#include "GameObject.h"
#include "GameInstance.h"

#include "Level_Logo.h"
#include "Level_Wizard.h"
#include "Level_MapTool.h"
#include "Level_GamePlay.h"
#include "Level_SlimeBoss.h"

#define fZ_LoadingScreen 0.9f
#define fZ_Holder 0.8f
#define fZ_Jack 0.7f
#define fZ_GameLogo 0.6f
#define fZ_Spinner 0.5f

CLevel_Loading::CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_Loading::Initialize(LEVEL eNextLevelID)
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	m_eNextLevelID = eNextLevelID;

	/* 스레드를 생성하여 자원로드를 맡긴다. */
	m_pLoader = CLoader::Create(m_pDevice, m_pContext, eNextLevelID);
	if (nullptr == m_pLoader)
		return E_FAIL;		
	Ready_Layer_BackGround();
	return S_OK;
}

void CLevel_Loading::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (true == m_pLoader->isFinished())
	{
		if (GetKeyState(VK_SPACE) & 0x8000)
		{
			CLevel*		pLevel = { nullptr };

			switch (m_eNextLevelID)
			{
			case LEVEL_LOGO:
				pLevel = CLevel_Logo::Create(m_pDevice, m_pContext);
				break;
			case LEVEL_MAPTOOL:
				pLevel = CLevel_MapTool::Create(m_pDevice, m_pContext);
				break;
			case LEVEL_GAMEPLAY:
				pLevel = CLevel_GamePlay::Create(m_pDevice, m_pContext);
				break;
			case LEVEL_SLIMEBOSS:
				pLevel = CLevel_SlimeBoss::Create(m_pDevice, m_pContext);
				break;
			case LEVEL_WIZARD:
				pLevel = CLevel_Wizard::Create(m_pDevice, m_pContext);
				break;
			}

			if (nullptr == pLevel)
				return;

			if (FAILED(m_pGameInstance->Open_Level(m_eNextLevelID, pLevel)))
				return;			
		}		
	}
}

HRESULT CLevel_Loading::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	m_pLoader->Output();

	return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_BackGround()
{
	CGameObject::GAMEOBJECT_DESC tDesc = {};
	tDesc.matWorld._43 = fZ_LoadingScreen;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_LOADING, TEXT("Layer_LoadingScreen"), TEXT("Prototype_GameObject_LoadingScreen"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_Holder;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_LOADING, TEXT("Layer_LoadingScreen"), TEXT("Prototype_GameObject_LoadingScreen"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_Jack;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_LOADING, TEXT("Layer_LoadingScreen"), TEXT("Prototype_GameObject_LoadingScreen"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_GameLogo;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_LOADING, TEXT("Layer_LoadingScreen"), TEXT("Prototype_GameObject_LoadingScreen"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_Spinner;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_LOADING, TEXT("Layer_LoadingScreen"), TEXT("Prototype_GameObject_LoadingScreen"), &tDesc)))
		return E_FAIL;
	return S_OK;
}

HRESULT CLevel_Loading::Ready_Layer_UI()
{
	return S_OK;
}

CLevel_Loading * CLevel_Loading::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLevel_Loading*		pInstance = new CLevel_Loading(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_Loading"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Loading::Free()
{
	__super::Free();

	Safe_Release(m_pLoader);
}
