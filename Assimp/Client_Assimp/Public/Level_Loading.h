#pragma once

#include "Client_Defines.h"
#include "Level.h"

/* 1. 레벨에 필요한 객체들을 생성한다. */
/* 2. 레벨을 반복적으로 갱신하여 화면에 보여준다. */

/* 3. 다음레벨에대한 자원을 로드하기 위한 로더객체를 생성해준다.  */

BEGIN(Client)

class CLevel_Loading final : public CLevel
{
private:
	CLevel_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Loading() = default;

public:
	virtual HRESULT Initialize(LEVEL eNextLevelID);
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	class CLoader*				m_pLoader = { nullptr };
	LEVEL						m_eNextLevelID = { LEVEL_END };

private:
	/* 로딩레벨에 필요한 객체를 만드는 함수. */
	HRESULT Ready_Layer_BackGround();
	HRESULT Ready_Layer_UI();


public:
	static CLevel_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END