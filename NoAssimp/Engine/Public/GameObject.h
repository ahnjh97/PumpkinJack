#pragma once

#include "Transform.h"

BEGIN(Engine)

class ENGINE_DLL CGameObject abstract : public CBase
{
public:
	typedef struct : public CTransform::TRANSFORM_DESC
	{	
		wstring wstrModelTag = L"";
		_uint iLevel = { 4 }; // �ʱⰪ: LEVEL_GAMPLAY
	}GAMEOBJECT_DESC;


protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& rhs);
	virtual ~CGameObject() = default;

public:
	void Set_PrototypeTag(const wstring& _wstrPrototypeTag) { wstrPrototypeTag = _wstrPrototypeTag; }
	PARSE_DATA Get_ParseData();

	_bool Get_Dead() { return m_bDead; }
	void Set_Dead() { m_bDead = true; }
	_uint Get_Level() { return m_iLevel; }

public:
	class CComponent* Get_Component(const wstring& strComTag);

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };

	class CGameInstance*				m_pGameInstance = { nullptr };
	CTransform*							m_pTransformCom = { nullptr };

protected:
	map<const wstring, class CComponent*>		m_Components;

protected:
	HRESULT Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, class CComponent** ppOut, void* pArg = nullptr);
	
	HRESULT Deactivate_BackFaceCulling();
protected:
	_bool m_bDead = false;
	wstring wstrPrototypeTag = {};
	_uint m_iLevel = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END