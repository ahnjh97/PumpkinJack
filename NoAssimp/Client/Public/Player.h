#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "Camera_Jack.h"

BEGIN(Engine)
class CPart;
END

BEGIN(Client)

class CPlayer final : public CGameObject
{
private:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& rhs);
	virtual ~CPlayer() = default;

public:
	STATE Get_State() { return m_eState; }
	WEAPON Get_Weapon() { return m_eWeapon; }
	_uint Get_Attack() { return m_iLastAttack; }
	_bool Get_IsJumping() { return m_bIsJumping; }
	_float3 Get_TargetDir() { return m_vDirection; }
	CPart* Get_Part(const wstring& wstrPart);
	_float4 Get_WeaponPos(WEAPON eWeapon);
	_vector Get_PlayerPos() { if (m_pTransformCom == nullptr) { return _vector(); }	
	else { return m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION); }}

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void OnKeyInput(_float fTimeDelta);
	void Update_Direction(_fvector vCamLook, _fvector vCamRight);

	void Update_PositionToCam();
	void Turn_Body(_float fTimeDelta);
	void Link_Animation();
	void Check_GroundCol();

private:
	map<const wstring, CPart*> m_Parts;
	CCamera_Jack* m_pCamera = { nullptr };

	STATE m_eState = { PLAYER_END };
	WEAPON m_eWeapon = { };
	_uint m_iLastAttack = { 0 };
	vector<string> m_vecWeaponName = { "Spear", "Rifle", "Scythe", "WPN_END" };

	_bool m_bIsJumping = { false };
	_bool m_bIsAttacking = { false };
	_bool m_bIsRolling = { false };
	
	_uint m_iJumpCount = { 2 };

	_float m_fRollEndTime = { 0.f };

	_float3	m_vDirection = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Add_PartObjects(LEVEL eLevel);

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

