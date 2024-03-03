#pragma once
#include "Client_Defines.h"
#include "MyImGui.h"

BEGIN(Client)

class CImGui_Client : public CMyImGui
{
private:
	CImGui_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CImGui_Client() = default;

public:
	HRESULT						Initialize();
	virtual void				Tick(_float fTimeDelta)		override;
	virtual HRESULT				Render()					override;

	_bool						Check_Pick();

private:
	_bool						bCallNewFrame = { false };
	_float3						m_vPickPos = {};

public:
	static CImGui_Client* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free();
};

END