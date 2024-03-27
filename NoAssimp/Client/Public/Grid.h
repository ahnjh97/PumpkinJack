#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Terrain;
class CNavigation;
END

BEGIN(Client)

class CGrid final : public CGameObject
{
public:
	enum TEXTURETYPE { TYPE_DIFFUSE, TYPE_MASK, TYPE_BRUSH, TYPE_END };

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		wstring wstrFileName = L"";
	}GRID_DESC;

	void Set_PassIndex(_int iPassIndex) { m_iPassIndex = iPassIndex; }

private:
	CGrid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGrid(const CGrid& rhs);
	virtual ~CGrid() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_uint m_iPassIndex = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom[TYPE_END] = { nullptr };
	CVIBuffer_Terrain* m_pVIBufferCom = { nullptr };
	CNavigation* m_pNavigationCom = { nullptr };

private:
	HRESULT Add_Components(const wstring& wstrFileName);
	HRESULT Bind_ShaderResources();

public:
	static CGrid* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END