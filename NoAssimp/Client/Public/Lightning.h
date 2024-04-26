#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CLightning final : public CGameObject
{
	enum TEX_ELEC { TEX_LIGHTNING, TEX_NOISE, TEX_END };

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		_uint iLightingIndex = {};
		const _float4x4* pTargetWorldMat = { nullptr };
	}LIGHTNING_DESC;

private:
	CLightning(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLightning(const CLightning& rhs);
	virtual ~CLightning() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_uint m_iSpriteIndex = {};
	_uint m_iLightningIndex = {};
	_uint m_iSpriteEnd = {};

	_float m_fTime = {};

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = {};
	CShader* m_pShaderCom = { nullptr };

	const _float4x4* m_pTargetWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, _uint _iLightingIndex);
	HRESULT Bind_ShaderResources();

public:
	static CLightning* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

