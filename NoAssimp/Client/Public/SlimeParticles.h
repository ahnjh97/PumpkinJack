#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CSlimeParticles final : public CGameObject
{
	enum TEX_SLIMEPARTICLES { TEX_SLIMETEX, TEX_MASK, TEX_END };
private:
	CSlimeParticles(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlimeParticles(const CSlimeParticles& rhs);
	virtual ~CSlimeParticles() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fTime = {};
	_float m_fDuration = {};

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr, nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CSlimeParticles* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
