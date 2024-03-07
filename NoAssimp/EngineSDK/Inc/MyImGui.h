#pragma once
#include "Base.h"
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CMyImGui abstract : public CBase
{
protected:
	CMyImGui(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMyImGui() = default;

public:
	void Set_Vectors(vector<string> vecAnim, vector<string> vecNonAnim);
	_float3 Get_PickPos() { return m_vPickPos; }
	_bool IsPicking() { return m_bIsPicking; }

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
	vector<string>		vecAnimModels;
	vector<string>		vecNonAnimModels;
	vector<string>		vecAnimNames;

	_float3						m_vPickPos = {};
	_bool						m_bIsPicking = { false };
	CGameObject*				m_pPickedObject = { nullptr };

public:
	virtual void				Free();

};

END
