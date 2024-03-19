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
	_bool isFinished() const { return m_isFinished; }
	void Output() {	SetWindowText(g_hWnd, m_strLoadingText.c_str()); }
	

private:
	ID3D11Device*				m_pDevice = { nullptr };
	ID3D11DeviceContext*		m_pContext = { nullptr };
	HANDLE						m_hThread;
	CRITICAL_SECTION			m_Critical_Section;
	LEVEL						m_eNextLevelID = { LEVEL_END };
	wstring						m_strLoadingText;
	_bool						m_isFinished = { false };

	CGameInstance*				m_pGameInstance = { nullptr };

	map<string, MODEL_CONFIG>	m_ModelConfigs;

private:
	HRESULT Loading_For_Logo();
	HRESULT Loading_For_GamePlay(LEVEL eLevel);
	HRESULT Loading_For_MapTool(LEVEL eLevel);
	HRESULT Loading_For_SlimeBoss(LEVEL eLevel);

	HRESULT Add_Model(LEVEL eLevel, TYPE eModelType, string strModelName);
	HRESULT Add_Models_GamePlay();
	HRESULT Add_Shaders(LEVEL eLevel);
	HRESULT Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures = 1);

	void Insert_ScaleRotationData();

	void TraverseAnimModels(LEVEL eLevel, const wstring& rootFolderPath);
	void TraverseNonAnimModels(LEVEL eLevel, const wstring& rootFolderPath);
	void TraverseDirectory(const wstring& rootFolderPath, list<wstring>& fileList);

public:
	static CLoader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID);
	virtual void Free() override;
};

END
