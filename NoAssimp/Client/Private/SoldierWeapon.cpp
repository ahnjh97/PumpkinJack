#include "stdafx.h"
#include "SoldierWeapon.h"
#include "Bone.h"

CSoldierWeapon::CSoldierWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPart{ pDevice, pContext }
{
}

CSoldierWeapon::CSoldierWeapon(const CSoldierWeapon& rhs)
	: CPart{ rhs }
{
}

void CSoldierWeapon::Reset_Damagable()
{
	if (m_pColliderCom != nullptr)
		m_pColliderCom->Clear_CollidedColliders();
}

HRESULT CSoldierWeapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSoldierWeapon::Initialize(void* pArg)
{
	SOLDIER_WEAPON_DESC* pSoldierWeaponDesc = (SOLDIER_WEAPON_DESC*)pArg;

	m_pSocket = pSoldierWeaponDesc->pSocket;
	m_pState = pSoldierWeaponDesc->pState;
	m_pDamage = pSoldierWeaponDesc->pDamage;
	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pCurLevel = m_pGameInstance->Get_CurrentLevelID();

	return S_OK;
}

void CSoldierWeapon::Tick(_float fTimeDelta)
{
	if (m_bActivated)
		Check_PlayerCol();
}

void CSoldierWeapon::Late_Tick(_float fTimeDelta)
{
	_matrix matCombined = XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix());
	_vector vScale, vRot, vTrans;
	XMMatrixDecompose(&vScale, &vRot, &vTrans, matCombined);

	_vector vNewScale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	_matrix matNew = XMMatrixAffineTransformation(vNewScale, XMVectorZero(), vRot, vTrans);
	_matrix matRot = XMMatrixRotationY(0.f);

	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * matRot * matNew * XMLoadFloat4x4(m_pParentMatrix));

	m_pColliderCom->Tick(XMLoadFloat4x4(&m_WorldMatrix));

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSoldierWeapon::Render()
{
#ifdef _DEBUG
	if (m_bActivated)
		m_pColliderCom->Render();
#endif // _DEBUG

	return S_OK;
}

void CSoldierWeapon::Check_PlayerCol()
{
	if (*m_pCurLevel != LEVEL_GAMEPLAY)
		return;

	CGameObject* pPlayer = m_pGameInstance->Get_Player(LEVEL_GAMEPLAY);
	if (nullptr == pPlayer)
		return;

	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
	if (nullptr == pPlayerCollider)
		return;

	if (m_pColliderCom->Check_AlreadyCollided(pPlayerCollider))
		return;

	if(m_pColliderCom->Intersect(pPlayerCollider))
		pPlayer->Deduct_HP(*m_pDamage);
}

HRESULT CSoldierWeapon::Add_Components()
{
	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC ColliderDesc{};
	ColliderDesc.vSize = _float3(1.5f, 8.7f, 1.5f);
	ColliderDesc.vCenter = _float3(0.f, 4.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

CSoldierWeapon* CSoldierWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSoldierWeapon* pInstance = new CSoldierWeapon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSoldierWeapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSoldierWeapon::Clone(void* pArg)
{
	CSoldierWeapon* pInstance = new CSoldierWeapon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSoldierWeapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSoldierWeapon::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pSocket);
}
