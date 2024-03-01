#pragma once

#include "Base.h"

/* 1. 실제 게임내에 보여줘야할 레벨을 보유하고 있다. */
/* 2. 보유하고 있는 레벨의 Tick, Render를 무한히 반복하여 호출한다. */
/* 3. 새로운 레벨로 할당할 때, 기존레벨을 삭제하는 기능을 수행한다. */
/* 4. 기존 레벨이 파괴될 때, 기존 레벨용으로 로드해놓은 자원들을 삭제한다. */

BEGIN(Engine)

class CLevel_Manager final : public CBase
{
private:
	CLevel_Manager();
	virtual ~CLevel_Manager() = default;

public:
	HRESULT Initialize();
	HRESULT Open_Level(_uint iNewLevelID, class CLevel* pNewLevel);
	void Tick(_float fTimeDelta);
	HRESULT Render();


private:
	_uint					m_iCurrentLevelID = { 0 };
	class CLevel*			m_pCurrentLevel = { nullptr };

public:
	static CLevel_Manager* Create();
	virtual void Free() override;
};

END