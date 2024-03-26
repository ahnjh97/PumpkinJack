#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
class CPart;
END

BEGIN(Client)
class CInven final : public CGameObject
{
	enum TEXTURETYPE { TYPE_DIFFUSE, TYPE_BRUSH, TYPE_END };
public:
	typedef struct : public GAMEOBJECT_DESC
	{
		WEAPON eWeapon = {};
	}INVEN_DESC;

private:
	CInven(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInven(const CInven& rhs);
	virtual ~CInven() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = {nullptr};
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	class CPlayer* m_pPlayer = { nullptr };
	map<const wstring, CPart*> m_Parts;

private:
	WEAPON					m_eWeapon = { WPN_END };
	_float					m_fX, m_fY, m_fZ, m_fSizeX, m_fSizeY;
	_float4x4				m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	_uint					m_iTextureIndex = {};
	_float					m_fTime = {};

	_vector					m_vInvenUp[WPN_END];

private:
	HRESULT Add_Components(WEAPON eWeapon);
	HRESULT Bind_ShaderResources();
	HRESULT Add_PartObjects(LEVEL eLevel);

public:
	static CInven* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END