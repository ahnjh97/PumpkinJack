#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CSoldier final : public CGameObject
{
	enum SOLDIER_COLLIDER { DEFAULT, RANGE, COLLIDER_END };

private:
	CSoldier(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSoldier(const CSoldier& rhs);
	virtual ~CSoldier() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Get_PlayerWorldMatPtr();
	void LinkState();
	void StateToAnimation();
	void Soldier_AnimSpeed();
	void DeathEffect();

	_bool IsPlayerInRange();

private:
	SOLDIER m_eState = {};

	_bool m_bSpinAttack = { false };

	_int m_iHp = { 100 };
	
	_float m_fIdleTime = {};
	_float m_fTurnTime = {100.f};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom[COLLIDER_END] = {nullptr};
	CGameObject* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSoldier* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

