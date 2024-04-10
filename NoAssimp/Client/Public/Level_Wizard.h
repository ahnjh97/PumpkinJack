#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Wizard final : public CLevel
{
private:
	CLevel_Wizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Wizard() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_LandObject();
	HRESULT Ready_ParsedObjects();
	HRESULT Ready_UI(const wstring& strLayerTag);

private:
	ifstream m_InputFile;

public:
	static CLevel_Wizard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END

