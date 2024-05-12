#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CChargeCenter final : public CGameObject
{
	enum TEX_CHARGE { TEX_DEFAULT, TEX_MASK, TEX_END };
private:
	CChargeCenter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CChargeCenter(const CChargeCenter& rhs);
	virtual ~CChargeCenter() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fTime = {};
	_float m_fScale = { 1.2f };
	_float m_fEndTime = { 9.f };
	_float m_fPlayRatio = {};
	_float m_fAccSpeed = {};
	_float m_fAccScale = { 1.f };

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr, nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CChargeCenter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END



