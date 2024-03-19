#pragma once
#include "Client_Defines.h"
#include "Level.h"
BEGIN(Client)

class CLevel_SlimeBoss final : public CLevel
{
private:
	CLevel_SlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_SlimeBoss() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Lights();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_LandObject();
	HRESULT Ready_Layer_Monster(const wstring& strLayerTag);
	HRESULT Ready_Layer_Effect(const wstring& strLayerTag);
	HRESULT Ready_UI(const wstring& strLayerTag);

private:
	ifstream m_InputFile;

public:
	static CLevel_SlimeBoss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
