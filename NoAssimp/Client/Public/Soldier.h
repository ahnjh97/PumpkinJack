#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPart;
class CNavigation;
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
	SOLDIER Get_SoldierState() { return m_eState; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Get_PlayerWorldMatPtr();
	void LinkState(_float fTimeDelta);
	void StateToAnimation();
	void Soldier_AnimSpeed();
	void DeathEffect();
	void ActivateSoldierWeapon();

	_bool IsPlayerInRange();
	_bool Check_KartCol(_float fTimeDelta);

	_float CalculateDistanceFromPlayer();

private:
	SOLDIER m_eState = {};

	_bool m_bSpinAttack = { false };
	_bool m_bResetSpinAttack = { true };
	_bool m_bDeathEffect = { true };
	_bool m_bResetSoldierSlash = { false };
	_bool m_bPlayLandSound = { true };
	_bool m_bPlayAttackSound = { true };

	_uint m_iDamage = { };
	
	_float m_fIdleTime = {};
	_float m_fTurnTime = { 100.f };
	_float m_fDeathTime = {};
	_float m_fAlpha = { 1.f };
	_float m_fTime = { 100.f };
	_float m_fEndTime = { 0.7f };
	_float m_fPlayRatio = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom[COLLIDER_END] = {nullptr};
	CNavigation* m_pNavigationCom = { nullptr };
	CGameObject* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };
	CPart* m_pPart_Weapon = { nullptr };
	class CSoldierSlash* m_pPart_SoldierSlash = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Add_PartObjects(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CSoldier* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

