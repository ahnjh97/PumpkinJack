#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Rect;
class CTexture;
class CShader;
END

BEGIN(Client)

class CElecChain final : public CGameObject
{
	enum TEX_ELEC { TEX_ELEC, TEX_NOISE, TEX_END };

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		_uint iElecBeamIndex = {};
	}ELEC_BEAM_DESC;

private:
	CElecChain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CElecChain(const CElecChain& rhs);
	virtual ~CElecChain() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	
private:
	HRESULT Add_ElecChain(LEVEL eLevel, _uint iBeamIndex);

private:
	_uint m_iElecBeamIndex = {};

	_float m_fTime = {};

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = {};
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CElecChain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

