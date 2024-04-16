#pragma once
#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_EffectTool : public CLevel
{
private:
	CLevel_EffectTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_EffectTool() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT		Render() override;

private:
	HRESULT				Ready_Lights();
	HRESULT				Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT				Ready_Layer_Grid(const wstring& strLayerTag);
	HRESULT				Ready_Layer_Effect(const wstring& strLayerTag);

public:
	static CLevel_EffectTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void		Free() override;
};

END