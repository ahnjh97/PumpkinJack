#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CShockWave final : public CGameObject
{
	enum TEX_SHOCKWAVE { TEX_DEFAULT, TEX_MASK, TEX_END };

public:
	typedef struct : public GAMEOBJECT_DESC
	{
		_int iRotation;
		_int iShockWaveIndex;
	}SHOCKWAVE_DESC;
private:
	CShockWave(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CShockWave(const CShockWave& rhs);
	virtual ~CShockWave() = default;

public:
	_int Get_ShockWaveIndex() { return m_iShockWaveIndex; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_int m_iShockWaveIndex = {};

	_float m_fTime = {};
	_float m_fDuration = {};

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr, nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CShockWave* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


