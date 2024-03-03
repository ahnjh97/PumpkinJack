#pragma once
#include "Base.h"

BEGIN(Engine)

class CPicking : public CBase
{
private:
	CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CPicking() = default;

public:
	HRESULT Initialize(HWND hWnd, _uint iWinSizeX, _uint iWinSizeY);
	void Update();
	void Transform_PickingToLocalSpace(const class CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float3* pRayPos);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	HWND					m_hWnd;
	_uint					m_iWinSizeX, m_iWinSizeY;

	_float3					m_vRayDir, m_vRayPos;

public:
	static CPicking* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, _uint iWinSizeX, _uint iWinSizeY);
	virtual void Free() override;
};

END