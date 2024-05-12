#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CRockParticles final : public CGameObject
{
	enum TEX_ROCKPARTICLES { TEX_DEFAULT, TEX_MASK, TEX_END };

public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		wstring wstrTextureName;
	}ROCKPARTICLES_DESC;
private:
	CRockParticles(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRockParticles(const CRockParticles& rhs);
	virtual ~CRockParticles() = default;

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
	HRESULT Add_Components(LEVEL eLevel, wstring _wstrTextureName);
	HRESULT Bind_ShaderResources();

public:
	static CRockParticles* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


