#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CIndicator final : public CGameObject
{
public:
	typedef struct : public GAMEOBJECT_DESC
	{
		WEAPON eWeapon = {};
	}INVEN_DESC;

private:
	CIndicator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CIndicator(const CIndicator& rhs);
	virtual ~CIndicator() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	class CPlayer* m_pPlayer = { nullptr };

private:
	WEAPON					m_eWeapon = { WPN_END };
	_float					m_fX, m_fY, m_fZ, m_fSizeX, m_fSizeY;
	_float4x4				m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	_uint					m_iTextureIndex = {};
	_float					m_fTime = {};
	_float					m_fDegree = {};
	_float					m_fAlpha = { };

	_vector					m_vInvenUp[WPN_END];
	

private:
	HRESULT Add_Components(WEAPON eWeapon);
	HRESULT Bind_ShaderResources();

public:
	static CIndicator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END