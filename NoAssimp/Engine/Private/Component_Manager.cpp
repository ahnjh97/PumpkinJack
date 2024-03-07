#include "Component_Manager.h"
#include "Component.h"
#include <typeinfo>

CComponent_Manager::CComponent_Manager()
{
}

HRESULT CComponent_Manager::Initialize(_uint iNumLevels)
{
	m_iNumLevels = iNumLevels;

	m_pPrototypes = new PROTOTYPES[iNumLevels];

	return S_OK;
}

HRESULT CComponent_Manager::Add_Prototype(_uint iLevelIndex, const wstring & strPrototypeTag, CComponent * pPrototype)
{
	if (nullptr != Find_Prototype(iLevelIndex, strPrototypeTag))
		return E_FAIL;

	m_pPrototypes[iLevelIndex].emplace(strPrototypeTag, pPrototype);

	return S_OK;
}

CComponent * CComponent_Manager::Clone_Component(_uint iLevelIndex, const wstring & strPrototypeTag, void * pArg)
{
	CComponent*		pPrototype = Find_Prototype(iLevelIndex, strPrototypeTag);
	if (nullptr == pPrototype)
		return nullptr;

	CComponent*		pComponent = pPrototype->Clone(pArg);
	if (nullptr == pComponent)
		return nullptr;

	return pComponent;
}

void CComponent_Manager::Clear(_uint iLevelIndex)
{
	for (auto& Pair : m_pPrototypes[iLevelIndex])
	{
		Safe_Release(Pair.second);
	}
	m_pPrototypes[iLevelIndex].clear();
}

void CComponent_Manager::PushBackPrototypes(vector<string>& vecAnim, vector<string>& vecNonAnim)
{
	for (auto mapIter : m_pPrototypes[Level_MapTool])
	{
		if (mapIter.first.find(L"_Model") == wstring::npos) // 문자열 찾기실패
			continue;
		CModel* pModel = dynamic_cast<CModel*>(mapIter.second);
		if (pModel == nullptr)
			continue;
		TYPE eType = pModel->Get_ModelType();
		string strModelName = pModel->Get_ModelName();
		const char* charModelName = strModelName.c_str();
		if (eType == TYPE_ANIM)
			vecAnim.push_back(charModelName);
		if (eType == TYPE_NONANIM)
			vecNonAnim.push_back(charModelName);
	}
	return;
}

CComponent * CComponent_Manager::Find_Prototype(_uint iLevelIndex, const wstring & strPrototypeTag)
{
	if (iLevelIndex >= m_iNumLevels)
		return nullptr;

	auto	iter = m_pPrototypes[iLevelIndex].find(strPrototypeTag);

	if (iter == m_pPrototypes[iLevelIndex].end())
		return nullptr;

	return iter->second;	
}

CComponent_Manager * CComponent_Manager::Create(_uint iNumLevels)
{
	CComponent_Manager*		pInstance = new CComponent_Manager();

	if (FAILED(pInstance->Initialize(iNumLevels)))
	{
		MSG_BOX(TEXT("Failed To Created : CComponent_Manager"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CComponent_Manager::Free()
{
	for (size_t i = 0; i < m_iNumLevels; i++)
	{
		for (auto& Pair : m_pPrototypes[i])
			Safe_Release(Pair.second);
		m_pPrototypes[i].clear();
	}
	Safe_Delete_Array(m_pPrototypes);
}
