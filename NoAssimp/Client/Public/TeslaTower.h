#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CTeslaTower final : public CGameObject
{
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		_vector vReactorPos;
	}TESLATOWER_DESC;

private:
	CTeslaTower(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTeslaTower(const CTeslaTower& rhs);
	virtual ~CTeslaTower() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CGameObject* Add_Sphere(LEVEL eLevel, _float4x4 _matWorld);
	CGameObject* Add_ElecParts(LEVEL eLevel, _float4x4 _matWorld);
	CGameObject* Add_ElecChain(LEVEL eLevel, _float4x4 _matWorld, _fvector vReactorPos, _uint iChainIndex);

	void Add_DestroyEffect();
	void Add_RockParticles();

private:
	_uint m_iNumElecChains = 6;

	_float m_fTime = { 100.f };
	_float m_fEndTime = { 0.35f };
	_float m_fPlayRatio = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };
	CGameObject* m_pSphere = { nullptr };
	CGameObject* m_pElecParts = { nullptr };
	CGameObject* m_pElecChain[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CTeslaTower* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END