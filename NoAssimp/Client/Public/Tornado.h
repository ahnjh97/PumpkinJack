#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CTexture;
class CShader;
END

BEGIN(Client)

class CTornado final : public CGameObject
{
private:
	CTornado(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTornado(const CTornado& rhs);
	virtual ~CTornado() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Get_PlayerPtr();
	void Check_PlayerCol();
	void Check_TornadoCol(_float fTimeDelta);
	void EnlargeTornado(_float fTimeDelta);
	void MoveTowardsPlayer(_float fTimeDelta);
	void Check_HoleWallCol();
	void Check_Fall();

private:
	_bool m_bMove = { false };
	_bool m_bHoleWallCol = { false };
	_bool m_bFall = { false };

	_int m_iTurnDir = { 1 };

	_float m_fTime = {};
	_float m_fTornadoSize = {};
	_float m_fDamageTime = {};
	_float m_fHoleWallColTime = {};

	_vector m_vMoveDir = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CCollider* m_pColliderCom[2] = {nullptr};
	CGameObject* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMatrix = {nullptr};

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CTornado* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
