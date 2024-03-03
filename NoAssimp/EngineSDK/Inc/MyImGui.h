#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CMyImGui abstract : public CBase
{
protected:
	CMyImGui(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMyImGui() = default;

public:
	virtual HRESULT		Initialize(HWND hWnd);
	virtual void		Tick(_float fTimeDelta);
	virtual HRESULT		Render();

protected:
	HRESULT				Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, class CComponent** ppOut, void* pArg = 0);
	void				EditGuizmo(_float4x4& worldMatrix);

protected:
	class CGameInstance* m_pGameInstance = { nullptr };
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	map<const wstring, class CComponent*>		m_Components;

public:
	virtual void								Free();

};

END
