#pragma once

#include "Base.h"

/* 원형객체들을 보관한다 ..*/
/* 원형객체를 검색 후, 복제하여 사본객체를 생성한다. */
/* 실제 게임내에 사용하고자하는 사본객체들을 내 기준에 따라 그룹(CLayer)지어 보관한다 .*/
/* 사본객체들의 틱함수를 반복적으로 호출해 준다 .*/

BEGIN(Engine)

class CObject_Manager final : public CBase
{
private:
	CObject_Manager();
	virtual ~CObject_Manager() = default;

public:
	const class CComponent* Get_Component(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strComTag, _uint iIndex);

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_Prototype(const wstring& strPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_Clone(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strPrototypeTag, void* pArg);
	void Tick(_float fTimeDelta);
	void Late_Tick(_float fTimeDelta);
	void Clear(_uint iLevelIndex);


private:	
	map<const wstring, class CGameObject*>				m_Prototypes;

	_uint												m_iNumLevels = { 0 };
	map<const wstring, class CLayer*>*					m_pLayers = { nullptr };

private:
	class CGameObject* Find_Prototype(const wstring& strPrototypeTag);
	class CLayer* Find_Layer(_uint iLevelIndex, const wstring& strLayerTag);

public:
	static CObject_Manager* Create(_uint iNumLevels);
	virtual void Free() override;
};

END