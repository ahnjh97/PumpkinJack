#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CLifeBar final : public CGameObject
{
	enum TEXTURETYPE { TYPE_DIFFUSE, TYPE_MASK, TYPE_END };
private:
	CLifeBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLifeBar(const CLifeBar& rhs);
	virtual ~CLifeBar() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Lerp_Hp(_float fTimeDelta);

private:
	_float					m_fX, m_fY, m_fZ, m_fSizeX, m_fSizeY;
	_float4x4				m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	_uint					m_iTextureIndex = {};
	_float					m_fTime = {};
	_float					m_fOffsetTime = {};
	_float					m_fHpRatio = { 1 };
	_float					m_fPreHpRatio = { 1 };
	_float					m_fRenderedHpRatio = { 1 };
	_float					m_fHpDifference = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom[TYPE_END] = {nullptr};
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	class CPlayer* m_pPlayer = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CLifeBar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END