#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CTexture;
class CShader;
END

BEGIN(Client)

class CHoleWall final : public CGameObject
{
private:
	CHoleWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CHoleWall(const CHoleWall& rhs);
	virtual ~CHoleWall() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Check_PlayerCol();

private:
	_float m_fTime = {};
	_float m_fDamageTime = { 3.f };
	_float m_fFallSpeed = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CCollider* m_pColliderCom[2] = { nullptr };
	CGameObject* m_pPlayer = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CHoleWall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
