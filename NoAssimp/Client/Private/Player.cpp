#include "stdafx.h"
#include "Player.h"
#include "Weapon.h"
#include "Body.h"
#include "Collider.h"
#include "Kart.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer& rhs)
	: CGameObject{ rhs }
{

}

CPart* CPlayer::Get_Part(const wstring& wstrPart)
{
	auto iter = m_Parts.find(wstrPart);
	if (iter != m_Parts.end())
		return iter->second;

	return nullptr;
}

_float4 CPlayer::Get_WeaponPos(WEAPON eWeapon)
{
	wstring wstrWeapon = L"Part_";
	if (eWeapon == WPN_SPEAR)
		wstrWeapon += L"Spear";
	if (eWeapon == WPN_RIFLE)
		wstrWeapon += L"Rifle";
	if (eWeapon == WPN_SCYTHE)
		wstrWeapon += L"Scythe";
		
	auto iter = m_Parts.find(wstrWeapon);
	if (iter != m_Parts.end())
		return dynamic_cast<CWeapon*>(iter->second)->Get_Pos();

	return _float4();
}

CModel* CPlayer::Get_BodyModel()
{
	auto iter = m_Parts.find(TEXT("Part_Body"));
	if (iter == m_Parts.end())
		return nullptr;

	CModel* pBodyModel = dynamic_cast<CModel*>(iter->second->Get_Component(TEXT("Com_Model")));
	if (pBodyModel == nullptr)
		return nullptr;

	return pBodyModel;
}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 43.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	if (FAILED(Add_PartObjects((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	XMStoreFloat3(&m_vDirection, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));

	if(m_pNavigationCom[DEFAULT] != nullptr)
		m_pNavigationCom[DEFAULT]->Find_CurrentIndex(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));

	m_pCurLevel = m_pGameInstance->Get_CurrentLevelID();
	m_iHp = 80;
	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (m_pGameInstance->KeyDown(DIK_1))
		Deduct_HP(20);
	if(m_pGameInstance->KeyDown(DIK_2))
		Deduct_HP(-20);

	if (m_iHp <= 0)
		m_iHp = 0;
	if (m_iHp >= 100)
		m_iHp = 100;

	if (m_pCamera == nullptr)
	{
		m_pCamera = dynamic_cast<CCamera_Jack*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Jack")));
		if(m_pCamera != nullptr)
			Safe_AddRef(m_pCamera);
	}

	_bool bPortalCol = Check_SlimeBossPortalCol(fTimeDelta);
	if(bPortalCol)
		m_eState = HIGHFALL;

	_bool bWallCol = false;
	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_GAMEPLAY)
	{
		bWallCol = Check_WallColliders_LevelGamePlay();
		m_bKartCol = Check_Kart();
		if (false == m_bKartCol)
			m_bIsPushingKart = false;
	}
		
	else if(*m_pCurLevel == LEVEL_SLIMEBOSS)
		bWallCol = Check_WallColliders_LevelSlimeBoss();

	m_pTransformCom->Set_WallCol(bWallCol);

	m_bGroundCol = Check_TerrainCol();

	if (m_bGroundCol)
	{
		m_bIsJumping = false;
		m_pTransformCom->Reset_Jump();
		m_iJumpCount = 2;
	}

	if (!m_bGroundCol && !m_bIsJumping && !bPortalCol && !m_bIsPushingKart)
	{
		m_fGravityTime += fTimeDelta;
		m_pTransformCom->Fall(m_fGravityTime * 0.05f); // 임시 자유낙하
	}
	else
		m_fGravityTime = 0;

	Check_GoToSlimeBossRoom();
	Check_GoBackUp();

	if (!m_bIsRolling)
		m_fRollEndTime += fTimeDelta;

	Link_Animation();
	OnKeyInput(fTimeDelta);

	if (m_bPreIsPushingKart && !m_bIsPushingKart && m_pNavigationCom[DEFAULT] != nullptr)
		m_pNavigationCom[DEFAULT]->Find_CurrentIndex(m_pTransformCom->Get_Pos_Vector());

	if (m_bIsJumping)
	{
		if (m_eState != AIR_ATTACK)
			m_pTransformCom->Jump(fTimeDelta);
		else
		{
			if (m_eWeapon == WPN_SCYTHE || m_eWeapon == WPN_SPEAR)
				m_pTransformCom->Jump(fTimeDelta * 0.4f);
		}
	}
		
	if (m_bGroundCol)
		m_pTransformCom->Set_Height(m_fPlaneHeight);

	if (m_bIsAttacking)
		TargetNearestEnemy();

	Update_PositionToCam();
	Turn_Body(fTimeDelta);
	SetUp_Damage();

	if (m_bIsAttacking)	
		Activate_Weapon();

	if (WPN_RIFLE == m_eWeapon)
		RifleRecoil(fTimeDelta);

	if (WPN_SPEAR == m_eWeapon && ROLL_ATTACK == m_eState)
		SpearRollAttackDash(fTimeDelta);
	else {
		m_bIsDashDirDetermined = false;
		m_fDashTime = 0.f;
	}


	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CPlayer::Late_Tick(_float fTimeDelta)
{
	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	for (_int i = 0; i < NAVIGATION_END; i++)
	{
		if (m_pNavigationCom[i] != nullptr)
			m_pNavigationCom[i]->Late_Tick();
	}

	CModel* pBodyModel = Get_BodyModel();
	if (pBodyModel == nullptr)
		return;

	if (m_bIsAttacking && pBodyModel->isFinished())
		Reset_WeaponCollider(m_eWeapon);

	m_bPreInSlimeBossRoom = m_bInSlimeBossRoom;
	m_bPreIsPushingKart = m_bIsPushingKart;
}

HRESULT CPlayer::Render()
{
#ifdef _DEBUG
	m_pColliderCom->Render();

	if (m_pNavigationCom[m_eNavigation] != nullptr && m_pGameInstance->Get_NavigationMode() == true)
		m_pNavigationCom[m_eNavigation]->Render();
#endif

	return S_OK;
}

void CPlayer::OnKeyInput(_float fTimeDelta)
{
	if (m_pCamera == nullptr)
		return;
	if (m_eState == HIGHFALL) {
		m_bIsJumping = false;
		return;
	}
	
	CTransform* pCamTransform = dynamic_cast<CTransform*>(m_pCamera->Get_Component(g_strTransformTag));
	if (pCamTransform == nullptr)
		return;
	_vector vCamLook = pCamTransform->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vCamRight = pCamTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	CModel* pBodyModel = Get_BodyModel();

	if (m_bInSlimeBossRoom)
		m_eNavigation = SLIMEBOSSROOM;
	else
		m_eNavigation = DEFAULT;

	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && m_bKartCol && !m_bIsRolling)	{
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, m_vPushPos);
		m_bIsPushingKart = true;
		m_eState = PUSHING;
		m_bGroundCol = true;
	}
	else
		m_bIsPushingKart = false;

	if (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		m_bIsPushingKart = false;

	Update_KartPush(m_bIsPushingKart);

	if (!(m_pGameInstance->Get_DIKeyState(DIK_LSHIFT) & 0x80) && m_pGameInstance->KeyDown(DIK_TAB) && m_eState == IDLE)	{
		m_eWeapon = (WEAPON)((_uint)(m_eWeapon)+1);
		m_iLastAttack = 0;
		if (m_eWeapon == WPN_END)
			m_eWeapon = (WEAPON)0;
		Reset_WeaponCollider(m_eWeapon);
	}

	if (m_pGameInstance->KeyDown(DIK_LCONTROL) && !m_bIsAttacking && !m_bIsJumping && !m_bIsPushingKart) {
		m_eState = ROLL;
		m_bIsRolling = true;
	}

	if(m_eState == ROLL)
		m_pTransformCom->Move(fTimeDelta * 1.5f, ::XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);

	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) || (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		|| (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80) || (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
	{
		if (!m_bIsAttacking && !m_bIsRolling && !m_bIsPushingKart)
		{
			if(m_bIsJumping)
				m_pTransformCom->Move(fTimeDelta * 0.85f, ::XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);
			else
				m_pTransformCom->Move(fTimeDelta, ::XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);
		}
			
		if(!m_bIsJumping && !m_bIsAttacking && !m_bIsRolling && !m_bIsPushingKart)
			m_eState = RUN;
	}

	if (m_pGameInstance->KeyDown(DIK_SPACE) && !m_bIsAttacking && !m_bIsRolling && !m_bIsPushingKart && (m_iJumpCount > 0) )	{
		m_pTransformCom->Reset_Jump();
		m_eState = JUMP;
		m_bIsJumping = true;
		m_iJumpCount--;
		pBodyModel->Reset_Animation(m_vecWeaponName[m_eWeapon] + "_Jump");
	}

	if (m_pGameInstance->MouseDown(DIMKS_LBUTTON) && !m_bIsAttacking && !m_bIsRolling && !m_bIsPushingKart)
	{
		if (m_fRollEndTime < 0.5f && !m_bIsJumping)
			m_eState = ROLL_ATTACK;
		else if (m_bIsJumping == true)
		{
			m_eState = AIR_ATTACK;
		}
		else if (m_iLastAttack == 0 || m_iLastAttack == 3)
		{
			m_eState = ATTACK1;
			m_iLastAttack = 1;
		}
		else if (m_iLastAttack == 1)
		{
			m_eState = ATTACK2;
			m_iLastAttack = 2;
		}
		else if (m_iLastAttack == 2)
		{
			m_eState = ATTACK3;
			m_iLastAttack = 3;
		}
		m_bIsAttacking = true;
	}

	if (!(m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && !(m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80) &&
		!(m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && !(m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80) && 
		 !m_bIsJumping && !m_bIsAttacking && !m_bIsRolling && !pBodyModel->Get_IsLerping())
	{
		m_eState = IDLE;
	}

	Update_Direction(vCamLook, vCamRight);
	if (m_bIsPushingKart)
		m_vDirection = m_vKartLook;
}

void CPlayer::Update_Direction(_fvector vCamLook, _fvector vCamRight)
{
	if (m_bIsAttacking || m_bIsRolling || m_fDashTime != 0.f)
		return;

	_vector vLook = vCamLook;
	vLook = ::XMVectorSetY(vLook, 0.f);
	_vector vRight = vCamRight;
	vRight = ::XMVectorSetY(vRight, 0.f);

	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80))
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(vLook - vRight));
	else if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(vLook + vRight));
	else if ((m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80))
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(-vLook - vRight));
	else if ((m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(-vLook + vRight));
	else if (m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80)
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(vLook));
	else if (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80)
		::XMStoreFloat3(&m_vDirection, XMVector3Normalize(-vRight));
	else if (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80)
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(vRight));
	else if (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		::XMStoreFloat3(&m_vDirection, ::XMVector3Normalize(-vLook));
}

void CPlayer::Update_PositionToCam()
{
	if (m_pTransformCom != nullptr && m_pCamera != nullptr)
	{
		_vector vPlayerPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		m_pCamera->Copy_PlayerPos(vPlayerPos);
	}
}

void CPlayer::Turn_Body(_float fTimeDelta)
{
	_vector	vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, XMLoadFloat3(&m_vDirection)));
	_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, XMLoadFloat3(&m_vDirection)));
	if (fY < 0)
		fAngle = -fAngle;
	if (abs(fAngle) >= XMConvertToRadians(2.f))
		m_pTransformCom->Turn(::XMVectorSet(0, 1, 0, 0), fTimeDelta * fAngle * 7.f);
}

void CPlayer::Link_Animation()
{
	auto iter = m_Parts.find(TEXT("Part_Body"));
	if (iter == m_Parts.end())
		return;
	CBody* pBody = dynamic_cast<CBody*>(iter->second);

	if (m_eState == ROLL && (pBody->Get_IsFinished()))
	{
		m_bIsRolling = false;
		m_fRollEndTime = 0;
	}
	else if (m_eState == AIR_ATTACK && pBody->Get_IsFinished())
	{
		if (m_eWeapon == WPN_SCYTHE || m_eWeapon == WPN_SPEAR)
			m_eState = AIR_ATTACK_END;
		else
		{
			m_bIsAttacking = false;
			m_eState = IDLE;
		}
			
		m_pTransformCom->AirAttack_End();
	}
		
	else if ((m_eState == AIR_ATTACK_END || m_eState == ATTACK1 || m_eState == ATTACK2 || m_eState == ATTACK3 || m_eState == ROLL_ATTACK) && pBody->Get_IsFinished())
		m_bIsAttacking = false;
}

_bool CPlayer::Check_TerrainCol()
{
	_float fJumpTime = m_pTransformCom->Get_JumpTime();

	if (m_bIsJumping && fJumpTime <= 0.3f && fJumpTime > 0.f)
		return false;

	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	if (m_bInSlimeBossRoom == false)
	{
		if (m_pNavigationCom[DEFAULT] == nullptr)
			return true;

		_float fPlaneHeight = m_pNavigationCom[DEFAULT]->Get_PlaneHeight(vPos);
		if (vPos.y <= fPlaneHeight)
		{
			m_fPlaneHeight = fPlaneHeight;
			return true;
		}
			
		else if (Check_GroundColliders())
			return true;
	}
	else
	{
		_float fSBR_PlaneHeight = m_pNavigationCom[SLIMEBOSSROOM]->Get_PlaneHeight(vPos);
		if (vPos.y <= fSBR_PlaneHeight)
		{
			m_fPlaneHeight = fSBR_PlaneHeight;
			return true;
		}
	}

	return false;
}

_bool CPlayer::Check_GroundColliders()
{
	_float fY = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y;
	CCollider* pPillarGroundCollider = { nullptr };
	if (*m_pGameInstance->Get_CurrentLevelID() != LEVEL_SLIMEBOSS)
		return false;
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar"));
	if (objList.empty())
		return false;

	for (auto& pillar : objList)
	{
		if (pillar == nullptr)
			continue;

		pPillarGroundCollider = dynamic_cast<CCollider*>(pillar->Get_Component(TEXT("Com_Collider_Ground")));
		if (pPillarGroundCollider == nullptr)
			continue;

		if (m_pColliderCom->Intersect(pPillarGroundCollider))
		{
			_float fPillarGroundColliderHeight = pPillarGroundCollider->Get_HightestPoint(CCollider::TYPE_OBB);
			if (fPillarGroundColliderHeight <= fY + 2.0f && m_pTransformCom->Get_JumpPower() <= 0.f)
			{
				m_fPlaneHeight = fPillarGroundColliderHeight - 1.5f;
				return true;
			}
		}
	}
	
	return false;
}

_bool CPlayer::Check_WallColliders(list<CGameObject*> objList, CCollider::TYPE eColliderType, const wstring& wstrColliderTag)
{
	CCollider* pWallCollider = { nullptr };
	_vector vPlayerPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_bool bMovingTowards = false;

	for (auto& obj : objList)
	{
		if (obj == nullptr)
			continue;

		pWallCollider = dynamic_cast<CCollider*>(obj->Get_Component(wstrColliderTag));
		if (pWallCollider == nullptr)
			continue;

		_float3 vCenterPosFloat = pWallCollider->Get_Center(eColliderType);
		_vector vCenterPos = XMLoadFloat3(&vCenterPosFloat);
		_vector vDirectionToWallCenter = vCenterPos - vPlayerPos;
		vDirectionToWallCenter = ::XMVectorSetY(vDirectionToWallCenter, 0.f);
		_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenNormals(::XMVector3Normalize(XMLoadFloat3(&m_vDirection)), ::XMVector3Normalize(vDirectionToWallCenter)));
		if (fAngle <= XM_PIDIV2 - 0.15f)
			bMovingTowards = true;

		if (m_pColliderCom->Intersect(pWallCollider) && bMovingTowards == true)
			return true;
	}

	return _bool();
}

_bool CPlayer::Check_WallColliders_LevelSlimeBoss()
{
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar"));
	if (objList.empty())
		return false;

	return Check_WallColliders(objList, CCollider::TYPE_OBB);
}

_bool CPlayer::Check_WallColliders_LevelGamePlay()
{
	list<CGameObject*> kartList = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, TEXT("Layer_Kart"));
	if (kartList.empty())
		return false;

	_bool bWallCol = Check_WallColliders(kartList, CCollider::TYPE_OBB);
	if (bWallCol)
		return bWallCol;
	
	list<CGameObject*> crateBoxList = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, TEXT("Layer_CrateBox"));
	if (crateBoxList.empty())
		return false;

	return Check_WallColliders(crateBoxList, CCollider::TYPE_OBB);
}

_bool CPlayer::Check_SlimeBossPortalCol(_float fTimeDelta)
{
	CCollider* pPortalCollider = { nullptr };
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Portal"));
	if (objList.empty())
		return false;

	for (auto& portal : objList)
	{
		if (portal == nullptr)
			continue;

		pPortalCollider = dynamic_cast<CCollider*>(portal->Get_Component(TEXT("Com_Collider")));
		if (pPortalCollider == nullptr)
			continue;

		if (m_pColliderCom->Intersect(pPortalCollider))
		{
			_vector vPortalCenter = XMLoadFloat3(&pPortalCollider->Get_Center(CCollider::TYPE_OBB));
			::XMVectorSetW(vPortalCenter, 1);
			m_pTransformCom->Move_toTarget(vPortalCenter, fTimeDelta * 0.25f, 1.5f);
			m_fHighFallTime += fTimeDelta;
			m_pTransformCom->Set_PortalCol(true);
			return true;
		}
		else
			m_pTransformCom->Set_PortalCol(false);
	}
	
	return false;
}

_bool CPlayer::Check_Kart()
{
	CKart* pKart = dynamic_cast<CKart*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Kart"), TEXT("Prototype_GameObject_Kart")));
	if (pKart == nullptr)
		return false;

	CCollider* pPushCollider = dynamic_cast<CCollider*>(pKart->Get_Component(TEXT("Com_Collider_Push")));
	if (pPushCollider == nullptr)
		return false;

	_vector vPlayerPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	if (m_pColliderCom->Intersect(pPushCollider))
	{
		_float3 vCenterPosFloat = pPushCollider->Get_Center(CCollider::TYPE_OBB);
		_vector vCenterPos = XMLoadFloat3(&vCenterPosFloat);
		_vector vDirectionToWallCenter = vCenterPos - vPlayerPos;
		vDirectionToWallCenter = ::XMVectorSetY(vDirectionToWallCenter, 0.f);
		_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenNormals(::XMVector3Normalize(XMLoadFloat3(&m_vDirection)), ::XMVector3Normalize(vDirectionToWallCenter)));
		if (fAngle <= XM_PIDIV2 - 0.6f)
		{
			CTransform* pKartTransform = dynamic_cast<CTransform*>(pKart->Get_Component(g_strTransformTag));
			if (pKartTransform == nullptr)
				return false;
			XMStoreFloat3(&m_vKartLook, pKartTransform->Get_State_Vector(CTransform::STATE_LOOK));
			_vector vKartPos = pKartTransform->Get_State_Vector(CTransform::STATE_POSITION);
			_vector vKartLook = pKartTransform->Get_State_Vector(CTransform::STATE_LOOK);
			XMStoreFloat4(&m_vPushPos, vKartPos - vKartLook * 6.f - ::XMVectorSet(0, 1.15f, 0, 0));
			return true;
		}
	}

	return _bool();
}

void CPlayer::Check_GoToSlimeBossRoom()
{
	if (m_fHighFallTime >= 1.7f)
	{
		_vector vSlimeBossRoom = ::XMVectorSet(205, -145, 150, 1);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vSlimeBossRoom);
		m_fHighFallTime = 0.f;
		m_eState = IDLE;
		m_bInSlimeBossRoom = true;
		m_eNavigation = SLIMEBOSSROOM;
		// curCell 다시검사
		if (m_pNavigationCom[m_eNavigation] != nullptr) {
			m_pNavigationCom[m_eNavigation]->Find_CurrentIndex(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
			m_pNavigationCom[m_eNavigation]->Get_PlaneHeight(m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION));
		}
	}
}

void CPlayer::Check_GoBackUp()
{
	if (m_bInSlimeBossRoom)
	{
		list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar2"));
		_bool bNullPointers = { true };
		_vector vTeleportPos = ::XMVectorSet(180, 0, 65, 1);

		if (!objList.empty())
		{
			for (auto& iter : objList)
			{
				if (iter != nullptr)
				{
					bNullPointers = false;
					break;
				}
			}
		}

		if (bNullPointers == true)
		{
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, vTeleportPos);
			m_bInSlimeBossRoom = false;
			m_iSlimeBossRoomEscapeCount++;
			m_eNavigation = DEFAULT;
			// curCell 다시검사
			if(m_pNavigationCom[m_eNavigation] != nullptr)
				m_pNavigationCom[m_eNavigation]->Find_CurrentIndex(m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION));
		}
	}
}

void CPlayer::Update_KartPush(_bool bPush)
{
	CKart* pKart = dynamic_cast<CKart*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Kart"), TEXT("Prototype_GameObject_Kart")));
	if (pKart == nullptr)
		return;

	pKart->Set_Push(bPush);
}

void CPlayer::Reset_WeaponCollider(WEAPON eWeapon)
{
	wstring wstrWeapon = L"Part_";
	if (eWeapon == WPN_SPEAR)
		wstrWeapon += L"Spear";
	if (eWeapon == WPN_RIFLE)
		wstrWeapon += L"Rifle";
	if (eWeapon == WPN_SCYTHE)
		wstrWeapon += L"Scythe";

	auto iter = m_Parts.find(wstrWeapon);
	if (iter != m_Parts.end())
	{
		CWeapon* pWeapon = dynamic_cast<CWeapon*>(iter->second);
		if(pWeapon != nullptr)
			pWeapon->Reset_Damagable();
	}
}

void CPlayer::TargetNearestEnemy()
{
	if (m_bIsDashDirDetermined)
		return;
	
	m_pNearestEnemyCollider = nullptr;
	m_fEnemyDistance = 50.f;
	
	if (LEVEL_GAMEPLAY == *m_pCurLevel)	{
		Check_EnemyDistance(LEVEL_GAMEPLAY, TEXT("Layer_Soldier"), CCollider::TYPE_OBB);
	}
	else if (LEVEL_SLIMEBOSS == *m_pCurLevel) {
		if (!m_bInSlimeBossRoom)
			Check_EnemyDistance(LEVEL_SLIMEBOSS, TEXT("Layer_Slime"), CCollider::TYPE_SPHERE);
		else
			Check_EnemyDistance(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar2"), CCollider::TYPE_OBB);
	}
	else if(LEVEL_WIZARD == *m_pCurLevel) { 
	}

	if (nullptr == m_pNearestEnemyCollider)
		return;

	_float3 vCenterPos = m_pNearestEnemyCollider->Get_Center(m_eEnemyColliderType);

	_float4 vRightFloat, vLookFloat;
	m_pTransformCom->Get_Look_LandObject(XMVectorSet(vCenterPos.x, vCenterPos.y, vCenterPos.z, 1), vRightFloat, vLookFloat);

	_vector vLook = XMVector3Normalize(XMLoadFloat4(&vLookFloat));
	::XMStoreFloat3(&m_vDirection, vLook);
	if (WPN_SPEAR == m_eWeapon && ROLL_ATTACK == m_eState)
		m_bIsDashDirDetermined = true;
}

void CPlayer::Check_EnemyDistance(LEVEL eLevel, const wstring& wstrLayerTag, CCollider::TYPE eColliderType, const wstring& wstrColliderTag)
{
	_vector vPlayerPos = m_pTransformCom->Get_Pos_Vector();

	list<CGameObject*> enemyList = m_pGameInstance->Get_ObjectList(eLevel, wstrLayerTag);
	if (enemyList.empty())
		return;

	for (auto& enemy : enemyList)
	{
		if (nullptr == enemy)
			continue;

		CCollider* pEnemyCollider = dynamic_cast<CCollider*>(enemy->Get_Component(wstrColliderTag));
		if (nullptr == pEnemyCollider)
			continue;

		_float3 vCenter = pEnemyCollider->Get_Center(eColliderType);

		_vector vResult = vPlayerPos - XMLoadFloat3(&vCenter);
		_float fDis = ::XMVectorGetX(::XMVector3Length(vResult));

		if (fDis < m_fEnemyDistance) 
		{
			m_pNearestEnemyCollider = pEnemyCollider;
			m_eEnemyColliderType = eColliderType;
		}
	}
}

void CPlayer::SetUp_Damage()
{
	if (WPN_SPEAR == m_eWeapon)
	{
		if (m_eState == AIR_ATTACK)
			m_iDamage = 15;
		else
			m_iDamage = 10;
	}
	else if (WPN_RIFLE == m_eWeapon)
	{
		if (m_eState == AIR_ATTACK)
			m_iDamage = 15;
		else if (m_eState == ROLL_ATTACK)
			m_iDamage = 5;
		else
			m_iDamage = 10;
	}
	else if (WPN_SCYTHE == m_eWeapon)
	{
		if (m_eState == AIR_ATTACK_END)
			m_iDamage = 20;
		else if (m_eState == ROLL_ATTACK)
			m_iDamage = 10;
		else
			m_iDamage = 15;
	}
}

void CPlayer::Activate_Weapon()
{
	CWeapon* pWeapon = { nullptr };
	CModel* pBodyModel = Get_BodyModel();
	if (pBodyModel == nullptr)
		return;

	_float fAnimPlayRatio = pBodyModel->Get_AnimPlayRatio();
	if (WPN_SPEAR == m_eWeapon)
	{
		pWeapon = dynamic_cast<CWeapon*>(Get_Part(TEXT("Part_Spear")));
		pWeapon->DeActivate_WeaponCollider();

		if (ATTACK1 == m_eState) {
			if (fAnimPlayRatio > 0.2f && fAnimPlayRatio < 0.35f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ATTACK2 == m_eState) {
			if (fAnimPlayRatio > 0.24f && fAnimPlayRatio < 0.42f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ATTACK3 == m_eState || ROLL_ATTACK == m_eState) {
			if (fAnimPlayRatio > 0.3f && fAnimPlayRatio < 0.55f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (AIR_ATTACK_END == m_eState) {
			if (fAnimPlayRatio < 0.2f)
				pWeapon->Activate_WeaponCollider();
		}
	}

	else if (WPN_RIFLE == m_eWeapon)
	{
		pWeapon = dynamic_cast<CWeapon*>(Get_Part(TEXT("Part_Rifle")));
		pWeapon->DeActivate_WeaponCollider();

		if (ATTACK1 == m_eState) {
			if (fAnimPlayRatio > 0.22f && fAnimPlayRatio < 0.27f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ATTACK2 == m_eState) {
			if (fAnimPlayRatio > 0.15f && fAnimPlayRatio < 0.35f)
				pWeapon->Activate_WeaponCollider();
		}
		else if(ATTACK3 == m_eState) {
			if (fAnimPlayRatio > 0.22f && fAnimPlayRatio < 0.35f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (AIR_ATTACK == m_eState) {
			if (fAnimPlayRatio > 0.3f && fAnimPlayRatio < 0.4f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ROLL_ATTACK == m_eState) {
			if ((fAnimPlayRatio > 0.07f && fAnimPlayRatio < 0.115f) || (fAnimPlayRatio > 0.27f && fAnimPlayRatio < 0.31f)
				|| (fAnimPlayRatio > 0.48f && fAnimPlayRatio < 0.52f) || (fAnimPlayRatio > 0.68f && fAnimPlayRatio < 0.72f))
				pWeapon->Activate_WeaponCollider();

			if((fAnimPlayRatio > 0.25f && fAnimPlayRatio < 0.27f) || (fAnimPlayRatio > 0.46f && fAnimPlayRatio < 0.48f) || (fAnimPlayRatio > 0.66f && fAnimPlayRatio < 0.68f))
				pWeapon->Reset_Damagable();
		}
	}

	else if (WPN_SCYTHE == m_eWeapon)
	{
		pWeapon = dynamic_cast<CWeapon*>(Get_Part(TEXT("Part_Scythe")));
		pWeapon->DeActivate_WeaponCollider();

		if (ATTACK1 == m_eState) {
			if (fAnimPlayRatio > 0.3f && fAnimPlayRatio < 0.5f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ATTACK2 == m_eState) {
			if (fAnimPlayRatio > 0.3f && fAnimPlayRatio < 0.4f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ATTACK3 == m_eState) {
			if (fAnimPlayRatio > 0.3f && fAnimPlayRatio < 0.45f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (AIR_ATTACK_END == m_eState) {
			if (fAnimPlayRatio < 0.2f)
				pWeapon->Activate_WeaponCollider();
		}
		else if (ROLL_ATTACK == m_eState) {
			if ((fAnimPlayRatio > 0.15f && fAnimPlayRatio < 0.7f))
				pWeapon->Activate_WeaponCollider();

			if ((fAnimPlayRatio > 0.25f && fAnimPlayRatio < 0.3f) || (fAnimPlayRatio > 0.5f && fAnimPlayRatio < 0.55f))	{
				if (m_bScytheRifleRollAttack) {
					pWeapon->Reset_Damagable();
					m_bScytheRifleRollAttack = false;
				}
			}

			else if (fAnimPlayRatio > 0.4f && fAnimPlayRatio < 0.45f) {
				if (!m_bScytheRifleRollAttack) {
					pWeapon->Reset_Damagable();
					m_bScytheRifleRollAttack = true;
				}
			}
			else if (fAnimPlayRatio > 0.7f)
				m_bScytheRifleRollAttack = true;
		}
	}
}

void CPlayer::SpearRollAttackDash(_float fTimeDelta)
{
	CModel* pBodyModel = Get_BodyModel();
	if (pBodyModel == nullptr)
		return;

	_float fAnimPlayRatio = pBodyModel->Get_AnimPlayRatio();

	if (fAnimPlayRatio > 0.2f && fAnimPlayRatio < 0.4f) {
		m_fDashTime += fTimeDelta;
		m_pTransformCom->Move(cosf(m_fDashTime) * 0.075f, XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);		
	}
		
	if (pBodyModel->isFinished())
	{
		m_fDashTime = 0.f;
		m_bIsDashDirDetermined = false;
	}
}

void CPlayer::RifleRecoil(_float fTimeDelta)
{
	CModel* pBodyModel = Get_BodyModel();
	if (pBodyModel == nullptr)
		return;

	_float fAnimPlayRatio = pBodyModel->Get_AnimPlayRatio();

	if (ATTACK1 == m_eState) {
		if (fAnimPlayRatio > 0.28f && fAnimPlayRatio < 0.305f){
			m_fDashTime += fTimeDelta * 4.f;
			m_pTransformCom->Move(-cosf(m_fDashTime) * 0.065f, XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);
		}
	}
	else if (ATTACK2 == m_eState) {
		if (fAnimPlayRatio > 0.3f && fAnimPlayRatio < 0.325f) {
			m_fDashTime += fTimeDelta * 4.f;
			m_pTransformCom->Move(-cosf(m_fDashTime) * 0.065f, XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);
		}
	}
	else if (ATTACK3 == m_eState) {
		if (fAnimPlayRatio > 0.34f && fAnimPlayRatio < 0.365f) {
			m_fDashTime += fTimeDelta * 4.f;
			m_pTransformCom->Move(-cosf(m_fDashTime) * 0.065f, XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);
		}
	}
	else if (AIR_ATTACK == m_eState) {
		if (fAnimPlayRatio > 0.37f && fAnimPlayRatio < 0.395f) {
			m_fDashTime += fTimeDelta * 4.f;
			m_pTransformCom->Move(-cosf(m_fDashTime) * 0.035f, XMLoadFloat3(&m_vDirection), m_pNavigationCom[m_eNavigation]);
		}
	}

	if (pBodyModel->isFinished())
		m_fDashTime = 0.f;
}

HRESULT CPlayer::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(4, 8, 4);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f + 1, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	if (LEVEL_WIZARD == eLevel)
		return S_OK;

	/* For.Com_Navigation */
	CNavigation::NAVIGATION_DESC			NavigationDesc{};
	NavigationDesc.iCurrentIndex = 0;
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Navigation") + to_wstring(eLevel),
		TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom[DEFAULT], &NavigationDesc)))
		return E_FAIL;

	if (eLevel == LEVEL_SLIMEBOSS) {
		if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Navigation_SlimeBossRoom"),
			TEXT("Com_Navigation_SlimeBossRoom"), (CComponent**)&m_pNavigationCom[SLIMEBOSSROOM], &NavigationDesc)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CPlayer::Add_PartObjects(LEVEL eLevel)
{
	/* For.Part_Body */
	CPart* pBody = { nullptr };
	CBody::BODY_DESC	BodyDesc{};

	BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	BodyDesc.pState = &m_eState;
	BodyDesc.pWeapon = &m_eWeapon;
	BodyDesc.pLastAttack = &m_iLastAttack;
	BodyDesc.pIsJumping = &m_bIsJumping;
	BodyDesc.iLevel = eLevel;
	BodyDesc.wstrModelTag = TEXT("Prototype_Component_Model_Jack");
	pBody = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Body"), &BodyDesc));
	if (nullptr == pBody)
		return E_FAIL;

	m_Parts.emplace(TEXT("Part_Body"), pBody);

	CModel* pModel = dynamic_cast<CModel*>(pBody->Get_Component(TEXT("Com_Model")));
	CTransform* pBodyTransform = dynamic_cast<CTransform*>(pBody->Get_Component(g_strTransformTag));
	CBody* pBodyBody = dynamic_cast<CBody*>(Get_Part(TEXT("Part_Body")));
	/* For.Part_Rifle */
	CPart* pRifle = { nullptr };
	CWeapon::WEAPON_DESC	RifleDesc{};
	RifleDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	RifleDesc.pSocket = pModel->Get_BonePtr("RightHandIndex1");
	RifleDesc.pState = &m_eState;
	RifleDesc.pWeapon = &m_eWeapon;
	RifleDesc.pDamage = &m_iDamage;
	RifleDesc.iLevel = eLevel;
	RifleDesc.wstrModelTag = TEXT("Prototype_Component_Model_Rifle");
	pRifle = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Rifle"), &RifleDesc));
	if (nullptr == pRifle)
		return E_FAIL;
	m_Parts.emplace(TEXT("Part_Rifle"), pRifle);

	/* For.Part_Spear */
	CPart* pSpear = { nullptr };
	CWeapon::WEAPON_DESC	SpearDesc{};
	SpearDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SpearDesc.pSocket = pModel->Get_BonePtr("RightHandIndex1");
	SpearDesc.pState = &m_eState;
	SpearDesc.pWeapon = &m_eWeapon;
	SpearDesc.pDamage = &m_iDamage;
	SpearDesc.iLevel = eLevel;
	SpearDesc.wstrModelTag = TEXT("Prototype_Component_Model_Spear");
	pSpear = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Spear"), &SpearDesc));
	if (nullptr == pSpear)
		return E_FAIL;
	m_Parts.emplace(TEXT("Part_Spear"), pSpear);

	/* For.Part_Scythe */
	CPart* pScythe = { nullptr };
	CWeapon::WEAPON_DESC	ScytheDesc{};
	ScytheDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	ScytheDesc.pSocket = pModel->Get_BonePtr("LeftHandIndex1");
	ScytheDesc.pState = &m_eState;
	ScytheDesc.pWeapon = &m_eWeapon;
	ScytheDesc.pDamage = &m_iDamage;
	ScytheDesc.iLevel = eLevel;
	ScytheDesc.wstrModelTag = TEXT("Prototype_Component_Model_Scythe");
	pScythe = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Scythe"), &ScytheDesc));
	if (nullptr == pScythe)
		return E_FAIL;
	m_Parts.emplace(TEXT("Part_Scythe"), pScythe);
	return S_OK;
}

CPlayer* CPlayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPlayer* pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CPlayer::Clone(void* pArg)
{
	CPlayer* pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();
	m_pNearestEnemyCollider = nullptr;

	for(_int i = 0; i < NAVIGATION_END; i++)
		Safe_Release(m_pNavigationCom[i]);

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pCamera);

	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();
}
