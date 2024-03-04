#include "..\Public\Object_Manager.h"
#include "GameObject.h"
#include "Layer.h"
#include <fstream>

CObject_Manager::CObject_Manager()
{
}

const CComponent * CObject_Manager::Get_Component(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strComTag, _uint iIndex)
{
	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);

	if (nullptr == pLayer)
		return nullptr;

	return pLayer->Get_Component(strComTag, iIndex);	
}

HRESULT CObject_Manager::Initialize(_uint iNumLevels)
{
	m_iNumLevels = iNumLevels;

	m_pLayers = new map<const wstring, class CLayer*>[iNumLevels];

	return S_OK;
}

HRESULT CObject_Manager::Add_Prototype(const wstring & strPrototypeTag, CGameObject * pPrototype)
{
	if (nullptr != Find_Prototype(strPrototypeTag))
		return E_FAIL;

	m_Prototypes.emplace(strPrototypeTag, pPrototype);

	return S_OK;
}

HRESULT CObject_Manager::Add_Clone(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strPrototypeTag, void* pArg)
{
	/* 복제해야할 원형객체를 검색한다. */
	CGameObject*	pPrototype = Find_Prototype(strPrototypeTag);
	if (nullptr == pPrototype)
		return E_FAIL;

	CGameObject*	pGameObject = pPrototype->Clone(pArg);
	if (nullptr == pGameObject)
		return E_FAIL;

	pGameObject->Set_PrototypeTag(strPrototypeTag);

	/* 복제한 사본객체를 추가해야할 레이어를 찾는다.*/
	CLayer*		pLayer = Find_Layer(iLevelIndex, strLayerTag);

	/* 레이어가 없었다면 만들어서 객체를 추가하고 만든 레이어를 다시 맵에 추가해준다. */
	if (nullptr == pLayer)
	{
		pLayer = CLayer::Create();
		if (nullptr == pLayer)
			return E_FAIL;
		pLayer->Add_GameObject(pGameObject);

		m_pLayers[iLevelIndex].emplace(strLayerTag, pLayer);
	}
	/* 추가하려고하느 ㄴ레이어가 이미 있었다.*/
	else	
		pLayer->Add_GameObject(pGameObject);

	return S_OK;
}

void CObject_Manager::Tick(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			/* 필요한 위치의 갱신작어블 수행한다. */
			Pair.second->Tick(fTimeDelta);
		}		
	}
}

void CObject_Manager::Late_Tick(_float fTimeDelta)
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
		{
			/* 갱신된 위치를 활용하여 추가적인 기능을 수행하낟. */
			Pair.second->Late_Tick(fTimeDelta);
		}
	}
}

void CObject_Manager::Clear(_uint iLevelIndex)
{
	for (auto& Pair : m_pLayers[iLevelIndex])
	{
		Safe_Release(Pair.second);
	}
	m_pLayers[iLevelIndex].clear();
}

CGameObject * CObject_Manager::Find_Prototype(const wstring & strPrototypeTag)
{
	auto		iter = m_Prototypes.find(strPrototypeTag);

	if (iter == m_Prototypes.end())
		return nullptr;

	return iter->second;	
}

CLayer * CObject_Manager::Find_Layer(_uint iLevelIndex, const wstring & strLayerTag)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	auto		iter = m_pLayers[iLevelIndex].find(strLayerTag);

	if (iter == m_pLayers[iLevelIndex].end())
		return nullptr;

	return iter->second;
}

_bool CObject_Manager::Binarize_Objects(string strLevel)
{
	string tempFileName = "temp_" + strLevel + ".txt";

	ofstream outputFile(tempFileName, ios::out | ios::binary);
	if (!outputFile.is_open()) // 임시파일 열렸는지 확인
	{
		string tempstr = "Failed to Open: " + tempFileName;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return false;
	}

	for (_uint i = 0; i < m_iNumLevels; i++)
	{
		for (auto& mapIter : m_pLayers[i])
		{
			if (mapIter.second == nullptr)
				continue;

			size_t found = mapIter.first.find(TEXT("Layer_Parse")); // 일단 임시로 monster
			if (found == wstring::npos) // 찾지못한경우
				continue;

			list<CGameObject*>& objectList = mapIter.second->Get_ObjectList(); // 파일입출력
			if (objectList.empty())
				continue;

			for (auto listIter : objectList)
			{
				pair<wstring, _float4x4> parsePair; // 프로토타입 이름, 월드행렬
				parsePair = listIter->Get_ParsingData();

				wchar_t wcharBuf[MAX_PATH];
				wcscpy_s(wcharBuf, parsePair.first.c_str());
				outputFile.write(reinterpret_cast<const char*>(&wcharBuf), sizeof(wcharBuf));
				outputFile.write(reinterpret_cast<const char*>(&parsePair.second), sizeof(parsePair.second));
			}
		}
	}

	outputFile.close();

	if (!outputFile)
	{
		string errorStr = "Failed to write data to " + tempFileName;
		MessageBoxA(nullptr, errorStr.c_str(), "error", MB_OK);
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	string fileName = "../../../objects_txt/" + strLevel + ".txt";
	remove(fileName.c_str());

	if (rename(tempFileName.c_str(), fileName.c_str()) != 0) // 임시파일 이름을 level 이름으로 변경
	{
		string errorStr2 = "Failed to rename " + tempFileName;
		MessageBoxA(nullptr, errorStr2.c_str(), "error", MB_OK);
		remove(tempFileName.c_str()); // 임시파일 삭제
		return false;
	}

	MSG_BOX(TEXT("Saved."));
	return true;
}


CObject_Manager * CObject_Manager::Create(_uint iNumLevels)
{
	CObject_Manager*		pInstance = new CObject_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX(TEXT("Failed To Created : CObject_Manager"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CObject_Manager::Free()
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pLayers[i])
			Safe_Release(Pair.second);
		m_pLayers[i].clear();		
	}

	Safe_Delete_Array(m_pLayers);

	for (auto& Pair : m_Prototypes)
		Safe_Release(Pair.second);

	m_Prototypes.clear();
}
