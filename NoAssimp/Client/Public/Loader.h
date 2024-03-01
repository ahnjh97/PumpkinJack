#pragma once

#include "Client_Defines.h"
#include "Base.h"

/* 다음레벨에 대한 자원을 로드한다. */

BEGIN(Engine)
class CGameInstance;
END

BEGIN(Client)

class CLoader final : public CBase
{
private:
	CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLoader() = default;

public:
	HRESULT Initialize(LEVEL eNextLevelID);
	HRESULT Start();
	_bool isFinished() const {
		return m_isFinished;
	}
	void Output() {
		SetWindowText(g_hWnd, m_strLoadingText.c_str());
	}
	

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	HANDLE						m_hThread;
	CRITICAL_SECTION			m_Critical_Section;
	LEVEL						m_eNextLevelID = { LEVEL_END };
	wstring						m_strLoadingText;
	_bool						m_isFinished = { false };

	CGameInstance*				m_pGameInstance = { nullptr };


private:
	HRESULT Loading_For_Logo();
	HRESULT Loading_For_GamePlay();

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END