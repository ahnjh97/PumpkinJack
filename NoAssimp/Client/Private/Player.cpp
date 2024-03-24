#include "stdafx.h"
#include "Player.h"
#include "Weapon.h"
#include "Body.h"
#include "Collider.h"

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

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 38.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	if (FAILED(Add_PartObjects((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (m_pCamera == nullptr)
	{
		m_pCamera = dynamic_cast<CCamera_Jack*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Jack")));
		Safe_AddRef(m_pCamera);
	}


	_bool bPortalCol = Check_SlimeBossPortal(fTimeDelta);
	if(bPortalCol)
		m_eState = HIGHFALL;
	_bool bGroundCol = Check_GroundCol();
	m_pTransformCom->Set_GroundCol(bGroundCol);

	if (!bGroundCol && !m_bIsJumping && !bPortalCol)
	{
		m_fGravityTime += fTimeDelta;
		m_pTransformCom->Fall(m_fGravityTime * 0.05f); // 임시 자유낙하
	}
	else
		m_fGravityTime = 0;
		
	if (m_fHighFallTime >= 2.f)
	{
		/*_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		_vector vSlimeBossRoom = XMVectorSetY(vPos, -150.f);*/
		_vector vSlimeBossRoom = XMVectorSet(205, -150, 150, 1);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, vSlimeBossRoom);
		m_fHighFallTime = 0.f;
		m_eState = IDLE;
		m_bInSlimeBossRoom = true;
	}

	if (m_pTransformCom->Get_GroundCol())
	{
		m_bIsJumping = false;
		m_pTransformCom->Reset_Jump();
		m_iJumpCount = 2;
	}
		
	Check_GoBackUp();

	if (!m_bIsRolling)
		m_fRollEndTime += fTimeDelta;

	Link_Animation();
	OnKeyInput(fTimeDelta);
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
		
	Update_PositionToCam();
	Turn_Body(fTimeDelta);

	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CPlayer::Late_Tick(_float fTimeDelta)
{
	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CPlayer::Render()
{
#ifdef _DEBUG
	m_pColliderCom->Render();
#endif
	return S_OK;
}

void CPlayer::OnKeyInput(_float fTimeDelta)
{
	if (m_pCamera == nullptr)
		return;
	if (m_eState == HIGHFALL)
	{
		m_bIsJumping = false;
		return;
	}
	
	CTransform* pCamTransform = dynamic_cast<CTransform*>(m_pCamera->Get_Component(g_strTransformTag));
	if (pCamTransform == nullptr)
		return;
	_vector vCamLook = pCamTransform->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vCamRight = pCamTransform->Get_State_Vector(CTransform::STATE_RIGHT);
	auto iter = m_Parts.find(TEXT("Part_Body"));
	CModel* pBodyModel = dynamic_cast<CModel*>(iter->second->Get_Component(TEXT("Com_Model")));

	if (m_pGameInstance->KeyDown(DIK_TAB) && m_eState == IDLE)
	{
		m_eWeapon = (WEAPON)((_uint)(m_eWeapon)+1);
		m_iLastAttack = 0;
		if (m_eWeapon == WPN_END)
			m_eWeapon = (WEAPON)0;
	}

	if (m_pGameInstance->KeyDown(DIK_LCONTROL) && !m_bIsAttacking && !m_bIsJumping)
	{
		m_eState = ROLL;
		m_bIsRolling = true;
	}

	if(m_eState == ROLL)
		m_pTransformCom->Move(fTimeDelta * 1.5f, XMLoadFloat3(&m_vDirection));

	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) || (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		|| (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80) || (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
	{
		if (!m_bIsAttacking && !m_bIsRolling)
		{
			if(m_bIsJumping)
				m_pTransformCom->Move(fTimeDelta * 0.85f, XMLoadFloat3(&m_vDirection));
			else
				m_pTransformCom->Move(fTimeDelta, XMLoadFloat3(&m_vDirection));
		}
			
		if(!m_bIsJumping && !m_bIsAttacking && !m_bIsRolling)
			m_eState = RUN;
	}

	if (m_pGameInstance->KeyDown(DIK_SPACE) && !m_bIsAttacking && !m_bIsRolling && (m_iJumpCount > 0))
	{
		m_pTransformCom->Reset_Jump();
		m_eState = JUMP;
		m_bIsJumping = true;
		m_iJumpCount--;
		pBodyModel->Reset_Animation(m_vecWeaponName[m_eWeapon] + "_Jump");
	}

	if (m_pGameInstance->MouseDown(DIMKS_LBUTTON) && !m_bIsAttacking && !m_bIsRolling)
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
}

void CPlayer::Update_Direction(_fvector vCamLook, _fvector vCamRight)
{
	if (m_bIsAttacking || m_bIsRolling)
		return;

	_vector vLook = vCamLook;
	vLook = XMVectorSetY(vLook, 0.f);
	_vector vRight = vCamRight;
	vRight = XMVectorSetY(vRight, 0.f);

	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80))
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(vLook - vRight));
	else if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(vLook + vRight));
	else if ((m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80))
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(-vLook - vRight));
	else if ((m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(-vLook + vRight));
	else if (m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80)
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(vLook));
	else if (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80)
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(-vRight));
	else if (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80)
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(vRight));
	else if (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		XMStoreFloat3(&m_vDirection, XMVector3Normalize(-vLook));
}

void CPlayer::Update_PositionToCam()
{
	if (m_pTransformCom != nullptr)
	{
		_vector vPlayerPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		m_pCamera->Copy_PlayerPos(vPlayerPos);
	}
}

void CPlayer::Turn_Body(_float fTimeDelta)
{
	_vector	vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, XMLoadFloat3(&m_vDirection)));
	_float fY = XMVectorGetY(::XMVector3Cross(vLook, XMLoadFloat3(&m_vDirection)));
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

_bool CPlayer::Check_GroundCol()
{
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	if (m_bInSlimeBossRoom == false)
	{
		if (vPos.y <= -0.f)
			return true;
		else if (Check_GroundMeshes())
			return true;
	}
	else
	{
		if (vPos.y <= -150.f)
			return true;
	}

	return false;
}

_bool CPlayer::Check_GroundMeshes()
{
	_float fY = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y;
	CCollider* pPillarCollider = { nullptr };
	if (*m_pGameInstance->Get_CurrentLevelID() != LEVEL_SLIMEBOSS)
		return false;
	list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar"));
	if (objList.empty())
		return false;

	for (auto& pillar : objList)
	{
		if (pillar == nullptr)
			continue;

		pPillarCollider = dynamic_cast<CCollider*>(pillar->Get_Component(TEXT("Com_Collider")));
		if (pPillarCollider == nullptr)
			continue;

		if (m_pColliderCom->Intersect(pPillarCollider) && m_pColliderCom->Get_HightestPoint(CCollider::TYPE_OBB) > fY)
			return true;
	}
	
	return false;
}

_bool CPlayer::Check_SlimeBossPortal(_float fTimeDelta)
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
			XMVectorSetW(vPortalCenter, 1);
			m_pTransformCom->Move_toTarget(vPortalCenter, fTimeDelta * 30.f, 2.f);
			m_fHighFallTime += fTimeDelta;
			return true;
		}
	}
	
	return false;
}

void CPlayer::Check_GoBackUp()
{
	if (m_bInSlimeBossRoom)
	{
		list<CGameObject*> objList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar2"));
		_bool bNullPointers = { true };
		if (objList.empty())
		{
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(205, 0, 150, 1));
			m_bInSlimeBossRoom = false;
			m_iSlimeBossRoomEscapeCount++;
		}
		else
		{
			for (auto& iter : objList)
			{
				if (iter != nullptr)
				{
					bNullPointers = false;
					break;
				}
			}

			if (bNullPointers == true)
			{
				m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(205, 0, 150, 1));
				m_bInSlimeBossRoom = false;
				m_iSlimeBossRoomEscapeCount++;
			}
		}
	}
}

HRESULT CPlayer::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(4, 8, 4);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f + 1, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

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
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pCamera);
	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();
}
