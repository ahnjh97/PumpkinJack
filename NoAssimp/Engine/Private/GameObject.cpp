#include "GameObject.h"
#include "GameInstance.h"
#include <locale>
#include <codecvt>

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject & rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
	, m_pGameInstance(rhs.m_pGameInstance)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

PARSE_DATA CGameObject::Get_ParseData()
{
	PARSE_DATA parseData = {};
	parseData.iLevel = m_iLevel;
	parseData.wstrPrototypeTag = wstrPrototypeTag;
	CModel* pModel = dynamic_cast<CModel*>(Get_Component(TEXT("Com_Model")));
	if (pModel != nullptr)
	{
		string strModelName = pModel->Get_ModelName();
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		parseData.wstrModelTag = L"Prototype_Component_Model_" + converter.from_bytes(strModelName);
	}
		
	if (m_pTransformCom != nullptr)
		parseData.matWorld = m_pTransformCom->Get_WorldFloat4x4();

	return parseData;
}

CComponent * CGameObject::Get_Component(const wstring & strComTag)
{
	auto	iter = m_Components.find(strComTag);

	if (iter == m_Components.end())
		return nullptr;

	if (iter->second == nullptr)
		return nullptr;

	return iter->second;	
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

/* 실제 게임내엣 사용되는 객체가 호출하는 함수다. */
HRESULT CGameObject::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {};
	if(nullptr != pArg)
		GAMEOBJECT_DESC		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;		

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext);
	if (nullptr == m_pTransformCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Initialize(pArg)))
		return E_FAIL;

	m_Components.emplace(g_strTransformTag, m_pTransformCom);
	Safe_AddRef(m_pTransformCom);	
	
	m_iLevel = GameObjectDesc.iLevel;

	return S_OK;
}

void CGameObject::Tick(_float fTimeDelta)
{

}

void CGameObject::Late_Tick(_float fTimeDelta)
{
}

HRESULT CGameObject::Render()
{
	return S_OK;
}


HRESULT CGameObject::Add_Component(_uint iLevelIndex, const wstring & strPrototypeTag, const wstring & strComponentTag, CComponent** ppOut, void * pArg)
{
	CComponent*		pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
	if (nullptr == pComponent)
		return E_FAIL;

	auto	iter = m_Components.find(strComponentTag);
	if (iter != m_Components.end())
		return E_FAIL;

	m_Components.emplace(strComponentTag, pComponent);
	
	*ppOut = pComponent;


	Safe_AddRef(pComponent);

	return S_OK;
}

void CGameObject::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);

	for (auto& Pair : m_Components)
		Safe_Release(Pair.second);

	m_Components.clear();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);	
}
