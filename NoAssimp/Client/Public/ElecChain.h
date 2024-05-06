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
		_uint iElecChainIndex = {};
		_vector vReactorPos;
	}ELEC_CHAIN_DESC;

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
	void Randomize_Chain();

private:
	_uint m_iElecChainIndex = {};

	_float m_fTime = {};
	_float m_fChangeTime = {};

private:
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = {};
	CShader* m_pShaderCom = { nullptr };

	random_device	m_RandomDevice;
	mt19937_64		m_RandomNumber;

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CElecChain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

