#pragma once

#include "Base.h"

/* 오브젝트 매니져가 개발자의 의도에 따라 객체들을 묶어서 보관한다. */
/* 한 묶음. */

BEGIN(Engine)

class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	const class CComponent* Get_Component(const wstring& strComTag, _uint iIndex);

public:
	HRESULT Initialize();
	HRESULT	Add_GameObject(class CGameObject* pGameObject);
	void Tick(_float fTimeDelta);
	void Late_Tick(_float fTimeDelta);

private:
	list<class CGameObject*>		m_GameObjects;

public:
	static CLayer* Create();
	virtual void Free() override;
};

END