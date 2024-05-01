#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CLightningEffects final : public CGameObject
{
private:
	CLightningEffects(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLightningEffects(const CLightningEffects& rhs);
	virtual ~CLightningEffects() = default;

public:
	_float Get_LightningTime() { return m_fTime; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Get_PlayerColliderPtr();
	void Check_PlayerCol();
	void Add_Collider();
	void Add_ElecParts();
	void Add_Lightning(_uint iLightningIndex);

private:
	_bool m_bAddElecParts = { true };
	_bool m_bAddLightning = { true };

	_float m_fTime = {};

private:
	/*CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };*/
	CCollider* m_pColliderCom = { nullptr };

	CGameObject* m_pPlayer = { nullptr };
	CCollider* m_pPlayerCollider = { nullptr };
	CGameObject* m_pElecParts[2] = { nullptr, nullptr };
	CGameObject* m_pLightning = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CLightningEffects* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
