#include "Light_Manager.h"
#include "Light.h"

CLight_Manager::CLight_Manager()
{

}

const LIGHT_DESC* CLight_Manager::Get_LightDesc(_uint iIndex)
{
	if (iIndex >= m_Lights.size())
		return nullptr;

	auto	iter = m_Lights.begin();

	for (_uint i = 0; i < iIndex; ++i)
		++iter;

	return (*iter)->Get_LightDesc();
}

/* いじし ぞしし さぁ*/
HRESULT CLight_Manager::Initialize()
{
	return S_OK;
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
	CLight* pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.emplace_back(pLight);

	return S_OK;
}

CLight_Manager* CLight_Manager::Create()
{
	CLight_Manager* pInstance = new CLight_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLight_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLight_Manager::Free()
{
	for (auto& pLight : m_Lights)
		Safe_Release(pLight);

	m_Lights.clear();
}
