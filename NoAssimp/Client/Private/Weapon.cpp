#include "stdafx.h"
#include "Weapon.h"
#include "Bone.h"

CWeapon::CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPart{ pDevice, pContext }
{
}

CWeapon::CWeapon(const CWeapon& rhs)
	: CPart{ rhs }
{
}

void CWeapon::Reset_Damagable()
{
	if (m_pColliderCom != nullptr)
		m_pColliderCom->Clear_CollidedColliders();
}

HRESULT CWeapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon::Initialize(void* pArg)
{
	WEAPON_DESC* pWeaponDesc = (WEAPON_DESC*)pArg;

	m_pSocket = pWeaponDesc->pSocket;
	m_pState = pWeaponDesc->pState;
	m_pWeapon = pWeaponDesc->pWeapon;
	m_pDamage = pWeaponDesc->pDamage;
	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)pWeaponDesc->iLevel, pWeaponDesc->wstrModelTag)))
		return E_FAIL;

	
	if (m_pModelCom->Get_ModelName() == "Spear")
	{
		m_pTransformCom->Rotation(XMVectorSet(0, 0, 1, 0), -XM_PIDIV2 + 0.1f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0, 0, 0, 1));
	}
		
	if (m_pModelCom->Get_ModelName() == "Rifle")
	{
		m_pTransformCom->Rotation(XMVectorSet(0, 0, 1, 0), -XM_PIDIV2);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0, 0, -1, 1));
	}
		
	if (m_pModelCom->Get_ModelName() == "Scythe")
	{
		m_pTransformCom->Rotation(XMVectorSet(0, 0, 1, 0), XM_PIDIV2);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0, -0.25f, 0.3f, 1));
	}
		
	m_pCurLevel = m_pGameInstance->Get_CurrentLevelID();

	return S_OK;
}

void CWeapon::Tick(_float fTimeDelta)
{
	m_bCorrectWeapon = Check_CorrectWeapon();
	if (!m_bCorrectWeapon)
		return;

	if (m_bActivated)
	{
		Check_SlimeCol();
		Check_SlimeBossRoomPillarCol();
		Check_SoldierCol();
	}
}

void CWeapon::Late_Tick(_float fTimeDelta)
{
	_matrix matCombined = XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix());
	_vector vScale, vRot, vTrans;
	XMMatrixDecompose(&vScale, &vRot, &vTrans, matCombined);
	
	_vector vNewScale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	_matrix matNew = XMMatrixAffineTransformation(vNewScale, XMVectorZero(), vRot, vTrans);

	_matrix matRot = XMMatrixRotationY(0.f);
	if (m_pModelCom->Get_ModelName() == "Spear")
	{
		if(*m_pState == IDLE || *m_pState == JUMP || *m_pState == ATTACK3)
			matRot = XMMatrixRotationZ(XMConvertToRadians(-3.f)) * XMMatrixRotationY(XMConvertToRadians(-10.f));
	}
	if (m_pModelCom->Get_ModelName() == "Scythe")
		matRot = XMMatrixRotationX(XMConvertToRadians(155.f));
		
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * matRot * matNew * XMLoadFloat4x4(m_pParentMatrix));

	if (m_bCorrectWeapon)
	{
		m_pColliderCom->Tick(XMLoadFloat4x4(&m_WorldMatrix));
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	}
}

HRESULT CWeapon::Render()
{
	if (!m_bCorrectWeapon)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	if (m_bActivated)
		m_pColliderCom->Render();

	return S_OK;
}

_bool CWeapon::Check_CorrectWeapon()
{
	if (m_pModelCom != nullptr)
	{
		if (*m_pWeapon == WPN_SPEAR && m_pModelCom->Get_ModelName() == "Spear")
			return true;
		if (*m_pWeapon == WPN_SCYTHE && m_pModelCom->Get_ModelName() == "Scythe")
			return true;
		if (*m_pWeapon == WPN_RIFLE && m_pModelCom->Get_ModelName() == "Rifle")
			return true;
	}

	return _bool();
}

void CWeapon::Check_SlimeCol()
{
	if (*m_pCurLevel != LEVEL_SLIMEBOSS)
		return;

	list<CGameObject*> slimeList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Slime"));
	if (slimeList.empty())
		return;

	for (auto& slime : slimeList)
	{
		if (slime == nullptr)
			continue;

		CCollider* pSlimeCollider = dynamic_cast<CCollider*>(slime->Get_Component(TEXT("Com_Collider")));
		if (pSlimeCollider == nullptr)
			continue;

		if (m_pColliderCom->Check_AlreadyCollided(pSlimeCollider))
			continue;

		if (m_pColliderCom->Intersect(pSlimeCollider))
			slime->Deduct_HP(*m_pDamage);
	}
}

void CWeapon::Check_SlimeBossRoomPillarCol()
{
	if (*m_pCurLevel != LEVEL_SLIMEBOSS)
		return;

	list<CGameObject*> SBR_PillarList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar2"));
	if (SBR_PillarList.empty())
		return;

	for (auto& SBR_Pillar : SBR_PillarList)
	{
		if (SBR_Pillar == nullptr)
			continue;

		CCollider* pSBR_Pillar_Collider = dynamic_cast<CCollider*>(SBR_Pillar->Get_Component(TEXT("Com_Collider")));
		if (pSBR_Pillar_Collider == nullptr)
			continue;

		if (m_pColliderCom->Check_AlreadyCollided(pSBR_Pillar_Collider))
			continue;

		if (m_pColliderCom->Intersect(pSBR_Pillar_Collider))
			SBR_Pillar->Deduct_HP(*m_pDamage);
	}
}

void CWeapon::Check_SoldierCol()
{
	if (*m_pCurLevel != LEVEL_GAMEPLAY)
		return;

	list<CGameObject*> soldierList = m_pGameInstance->Get_ObjectList(LEVEL_GAMEPLAY, TEXT("Layer_Soldier"));
	if (soldierList.empty())
		return;

	for (auto& soldier : soldierList)
	{
		if (soldier == nullptr)
			continue;

		CCollider* pSoliderCollider = dynamic_cast<CCollider*>(soldier->Get_Component(TEXT("Com_Collider")));
		if (pSoliderCollider == nullptr)
			continue;

		if (m_pColliderCom->Check_AlreadyCollided(pSoliderCollider))
			continue;

		if (m_pColliderCom->Intersect(pSoliderCollider))
			soldier->Deduct_HP(*m_pDamage);
	}
}

HRESULT CWeapon::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxModel_Lighted"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC	ColliderDesc{};
	ColliderDesc.fRadius = 5.f;
	ColliderDesc.vCenter = _float3(0.f, 0, 0.f);
	if (m_pModelCom->Get_ModelName() == "Spear")
		ColliderDesc.vCenter = _float3(0.f, 10, 0.f);
	else if (m_pModelCom->Get_ModelName() == "Rifle")
	{
		ColliderDesc.fRadius = 9.2f;
		ColliderDesc.vCenter = _float3(0.f, 0, 14.5f);
	}

	else if (m_pModelCom->Get_ModelName() == "Scythe")
		ColliderDesc.vCenter = _float3(0.f, 10, 0.f);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CWeapon::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	return S_OK;
}

CWeapon* CWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWeapon* pInstance = new CWeapon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CWeapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWeapon::Clone(void* pArg)
{
	CWeapon* pInstance = new CWeapon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CWeapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWeapon::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pSocket);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
