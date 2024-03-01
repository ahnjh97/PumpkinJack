#include "..\Public\Level_Manager.h"

#include "GameInstance.h"
#include "Level.h"

CLevel_Manager::CLevel_Manager()
{
}

HRESULT CLevel_Manager::Initialize()
{
	return S_OK;
}

HRESULT CLevel_Manager::Open_Level(_uint iNewLevelID, CLevel * pNewLevel)
{
	/* 최초레벨할당시에는 수행하지 않는다. */
	if (nullptr != m_pCurrentLevel)
	{
		CGameInstance*		pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		/* 기존 레벨용 자원을 파괴한다 .*/
		pGameInstance->Clear(m_iCurrentLevelID);

		Safe_Release(pGameInstance);
	}	

	/* 기존 레벨을 파괴한다. */
	Safe_Release(m_pCurrentLevel);

	/* 새로운 레벨로 교체한다. */
	m_pCurrentLevel = pNewLevel;

	m_iCurrentLevelID = iNewLevelID;

	return S_OK;
}

void CLevel_Manager::Tick(_float fTimeDelta)
{
	if (nullptr == m_pCurrentLevel)
		return;

	m_pCurrentLevel->Tick(fTimeDelta);
}

HRESULT CLevel_Manager::Render()
{
	if (nullptr == m_pCurrentLevel)
		return E_FAIL;

	return m_pCurrentLevel->Render();
}

CLevel_Manager * CLevel_Manager::Create()
{
	CLevel_Manager*		pInstance = new CLevel_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CLevel_Manager"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_Manager::Free()
{
	Safe_Release(m_pCurrentLevel);
}
