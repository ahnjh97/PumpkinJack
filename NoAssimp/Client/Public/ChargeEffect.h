#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CChargeEffect final : public CGameObject
{
private:
	CChargeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CChargeEffect(const CChargeEffect& rhs);
	virtual ~CChargeEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_uint m_iSpriteIndex = {};

	_float m_fTime = {};
	_float m_fDuration = { 9.f };

	_float3 m_vPivot = {};

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CChargeEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

