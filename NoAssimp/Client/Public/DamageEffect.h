#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CDamageEffect final : public CGameObject
{
private:
	CDamageEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDamageEffect(const CDamageEffect& rhs);
	virtual ~CDamageEffect() = default;

public:
	void Reset_Time() { if (m_fTime >= 0.5f) { m_fTime = 0.f; } }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float		m_fX, m_fY, m_fZ, m_fSizeX, m_fSizeY;
	_float4x4	m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	_float		m_fTime = { 100.f };
	_float		m_fEndTime = { 1.f };
	_float		m_fPlayRatio = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CDamageEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
