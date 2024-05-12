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
	enum NAVIGATION { DEFAULT, SLIMEBOSSROOM, NAVIGATION_END };
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
	_bool Get_GroundCol() { return m_bGroundCol; }
	_float Get_PlayerHpRatio() { return m_iHp * 0.01f; }
	CModel* Get_BodyModel();
	vector<_float> Get_QuadTreeCellHeights() { return m_vecQuadTreeCells; }
	class CWeapon* Get_WeaponPtr();

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
	_bool Check_WallColliders(list<CGameObject*> objList, const wstring& wstrColliderTag = TEXT("Com_Collider_Wall"));
	_bool Check_WallColliders_LevelSlimeBoss();
	_bool Check_WallColliders_LevelGamePlay();
	_bool Check_SlimeBossPortalCol(_float fTimeDelta);
	_bool Check_Kart();
	_bool Check_GroundCells(_Out_ _float& _fMinHeight, _Out_ _float& _fMaxHeight);
	_bool Check_Cauldron();

	void Check_GoToSlimeBossRoom();
	void Check_GoBackUp();
	void Update_KartPush(_bool bPush);
	void Reset_WeaponCollider(WEAPON eWeapon);
	void TargetNearestEnemy();
	void Check_EnemyDistance(LEVEL eLevel, const wstring& wstrLayerTag, const wstring& wstrColliderTag = TEXT("Com_Collider"));
	void SetUp_Damage();
	void Activate_Weapon();
	void SpearRollAttackDash(_float fTimeDelta);
	void RifleRecoil(_float fTimeDelta);

	void DamageEffect();
	void Add_JumpEffect();
	void Add_Slash(_fvector vDir);
	void Add_Bullets(_fvector vDir);
	void Check_ShockWaveColliders();
	void Add_FadeInOut();

private:
	STATE m_eState = { PLAYER_END };
	WEAPON m_eWeapon = {};
	_uint m_iLastAttack = {};
	vector<string> m_vecWeaponName = { "Spear", "Rifle", "Scythe", "WPN_END" };
	NAVIGATION m_eNavigation = {};
	CCollider* m_pNearestEnemyCollider = { nullptr };
	CCollider::TYPE m_eEnemyColliderType = {};
	vector<_float> m_vecQuadTreeCells;

	_bool m_bGroundCol = { false };
	_bool m_bIsJumping = { false };
	_bool m_bIsAttacking = { false };
	_bool m_bIsRolling = { false };
	_bool m_bInSlimeBossRoom = { false };
	_bool m_bPreInSlimeBossRoom = { false };
	_bool m_bIsPushingKart = { false };
	_bool m_bPreIsPushingKart = { false };
	_bool m_bKartCol = { false };
	_bool m_bScytheRifleRollAttack = { true };
	_bool m_bIsDashDirDetermined = { false };
	_bool m_bAddSlash = { false };
	_bool m_bAddBullets = { false };
	_bool m_bAddFadeInOut = { true };
	_bool m_bChangeLevel = { false };

	_uint m_iJumpCount = { 2 };
	_uint m_iSlimeBossRoomEscapeCount = {};
	_uint m_iDamage = {};

	_float m_fRollEndTime = { 1.f };
	_float m_fHighFallTime = {};
	_float m_fGravityTime = {};
	_float m_fDashTime = {};
	_float m_fPlaneHeight = {};
	_float m_fEnemyDistance = {};

	_float3	m_vDirection = {};
	_float3 m_vKartLook = {};
	_float4 m_vPushPos = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Add_PartObjects(LEVEL eLevel);

private:
	map<const wstring, CPart*> m_Parts;
	CCamera_Jack* m_pCamera = { nullptr };
	CCollider* m_pColliderCom = {nullptr};
	CNavigation* m_pNavigationCom[NAVIGATION_END] = { nullptr, nullptr };
	
public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

