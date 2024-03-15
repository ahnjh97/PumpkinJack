#include "stdafx.h"
#include "Player.h"
#include "Weapon.h"
#include "Body.h"

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

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 25.f;
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
	if (m_bIsJumping)
		m_fTempJumpTime += fTimeDelta;
	if (m_fTempJumpTime > 1.2f)
	{
		m_bIsJumping = false;
		m_fTempJumpTime = 0.f;
	}
		
	if (m_pCamera == nullptr)
	{
		m_pCamera = dynamic_cast<CCamera_Jack*>(m_pGameInstance->Get_GameObject(m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Jack")));
		Safe_AddRef(m_pCamera);
	}

	Link_Animation();
	OnKeyInput(fTimeDelta);
	Update_PositionToCam();
	Turn_Body(fTimeDelta);

	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
}

void CPlayer::Late_Tick(_float fTimeDelta)
{
	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
}

HRESULT CPlayer::Render()
{
	return S_OK;
}

void CPlayer::OnKeyInput(_float fTimeDelta)
{
	if (m_pCamera == nullptr)
		return;

	CTransform* pCamTransform = dynamic_cast<CTransform*>(m_pCamera->Get_Component(g_strTransformTag));
	if (pCamTransform == nullptr)
		return;
	_vector vCamLook = pCamTransform->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vCamRight = pCamTransform->Get_State_Vector(CTransform::STATE_RIGHT);

	Update_Direction(vCamLook, vCamRight);

	if (m_pGameInstance->KeyDown(DIK_TAB))
	{
		m_eWeapon = (WEAPON)((_uint)(m_eWeapon)+1);
		m_iLastAttack = 0;
		if (m_eWeapon == WPN_END)
			m_eWeapon = (WEAPON)0;
	} 
		
	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) || (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
		|| (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80) || (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
	{
		if(!m_bIsAttacking)
			m_pTransformCom->Move(fTimeDelta, XMLoadFloat3(&m_vTargetDir));
		if(!m_bIsJumping && !m_bIsAttacking)
			m_eState = RUN;
	}

	if (m_pGameInstance->KeyDown(DIK_SPACE) && !m_bIsAttacking)
	{
		m_pTransformCom->Go_Right(fTimeDelta, 1, vCamRight);
		m_eState = JUMP;
		m_bIsJumping = true;
	}

	if (m_pGameInstance->MouseDown(DIMKS_LBUTTON) && !m_bIsAttacking)
	{
		m_bIsAttacking = true;
		if (m_bIsJumping == true)
		{
		/*	if (m_eWeapon == WPN_SPEAR || m_eWeapon == WPN_SPEAR)*/
				m_eState = AIR_ATTACK;
			/*else
				m_eState = AIR_ATTACK_END;*/
			//m_iLastAttack = 0;
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
	}

	if (!(m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && !(m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80) &&
		!(m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && !(m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80) && 
		!(m_pGameInstance->Get_DIKeyState(DIK_SPACE) & 0x80) && !(m_pGameInstance->MouseDown(DIMKS_LBUTTON)) && 
		!(m_pGameInstance->MouseDown(DIMKS_RBUTTON)) && !m_bIsJumping && !m_bIsAttacking)
	{
		m_eState = IDLE;
	}
}

void CPlayer::Update_Direction(_fvector vCamLook, _fvector vCamRight)
{
	_vector vLook = vCamLook;
	vLook = XMVectorSetY(vLook, 0.f);
	_vector vRight = vCamRight;
	vRight = XMVectorSetY(vRight, 0.f);

	if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80))
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(vLook - vRight));
	}
	else if ((m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(vLook + vRight));
	}
	else if ((m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80))
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(-vLook - vRight));
	}
	else if ((m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80))
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(-vLook + vRight));
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80)
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(vLook));
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80)
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(-vRight));
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80)
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(vRight));
	}
	else if (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
	{
		XMStoreFloat3(&m_vTargetDir, XMVector3Normalize(-vLook));
	}
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
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, XMLoadFloat3(&m_vTargetDir)));
	_float fY = XMVectorGetY(::XMVector3Cross(vLook, XMLoadFloat3(&m_vTargetDir)));
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

	if (m_eState == AIR_ATTACK && !m_bIsJumping)
	{
		m_eState = AIR_ATTACK_END;
	}
	else if ((m_eState == AIR_ATTACK_END || m_eState == ATTACK1 || m_eState == ATTACK2 || m_eState == ATTACK3) && pBody->Get_IsFinished())
	{
		m_bIsAttacking = false;
	}
}

HRESULT CPlayer::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	return S_OK;
}

HRESULT CPlayer::Add_PartObjects(LEVEL eLevel)
{
	/* For.Part_Body */
	CPart* pBodyObject = { nullptr };
	CBody::BODY_DESC	BodyDesc{};

	BodyDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	BodyDesc.pState = &m_eState;
	BodyDesc.pWeapon = &m_eWeapon;
	BodyDesc.pLastAttack = &m_iLastAttack;
	BodyDesc.iLevel = eLevel;
	BodyDesc.wstrModelTag = TEXT("Prototype_Component_Model_Jack");
	pBodyObject = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Body"), &BodyDesc));
	if (nullptr == pBodyObject)
		return E_FAIL;

	m_Parts.emplace(TEXT("Part_Body"), pBodyObject);

	/* For.Part_Rifle */
	CPart* pRifle = { nullptr };
	CWeapon::WEAPON_DESC	RifleDesc{};

	CModel* pModel = (CModel*)pBodyObject->Get_Component(TEXT("Com_Model"));
	RifleDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	RifleDesc.pSocket = pModel->Get_BonePtr("RightHandIndex1");
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
	ScytheDesc.pSocket = pModel->Get_BonePtr("RightHandIndex1");
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
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CPlayer::Clone(void* pArg)
{
	CPlayer* pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();
	Safe_Release(m_pCamera);
	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();
}
