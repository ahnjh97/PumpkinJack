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
	const _float4x4* Get_PlayerWorldMatrixPtr() { if (m_pTransformCom == nullptr) { return nullptr; }
		else { return m_pTransformCom->Get_WorldFloat4x4_Ptr(); }}
	_bool Get_InSlimeBossRoom() { return m_bInSlimeBossRoom; }
	_uint Get_SlimeBossRoomEscapeCount() { return m_iSlimeBossRoomEscapeCount; }

	void Reduce_HP(_float fDamage) { ; }

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
	_bool Check_TerrainCol();
	_bool Check_GroundColliders();
	_bool Check_WallColliders(list<CGameObject*> objList, CCollider::TYPE eColliderType, const wstring& wstrColliderTag = TEXT("Com_Collider_Wall"));
	_bool Check_WallColliders_LevelSlimeBoss();
	_bool Check_WallColliders_LevelGamePlay();
	_bool Check_SlimeBossPortal(_float fTimeDelta);
	_bool Check_Kart();
	void Check_GoBackUp();

private:
	STATE m_eState = { PLAYER_END };
	WEAPON m_eWeapon = {};
	_uint m_iLastAttack = {};
	vector<string> m_vecWeaponName = { "Spear", "Rifle", "Scythe", "WPN_END" };

	_bool m_bIsJumping = { false };
	_bool m_bIsAttacking = { false };
	_bool m_bIsRolling = { false };
	_bool m_bInSlimeBossRoom = { false };
	_bool m_bIsPushingKart = { false };
	_bool m_bKartCol = { false };

	_uint m_iJumpCount = { 2 };
	_uint m_iSlimeBossRoomEscapeCount = {};

	_float m_fRollEndTime = {};
	_float m_fHighFallTime = {};
	_float m_fGravityTime = {};

	_float3	m_vDirection = {};
	_float3 m_vKartLook = {};
	_float4 m_vPushPos = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Add_PartObjects(LEVEL eLevel);

private:
	map<const wstring, CPart*> m_Parts;
	CCamera_Jack* m_pCamera = { nullptr };
	CCollider* m_pColliderCom = { nullptr };
	CNavigation* m_pNavigationCom = { nullptr };

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

