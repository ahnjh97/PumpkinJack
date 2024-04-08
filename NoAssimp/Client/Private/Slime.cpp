#include "stdafx.h"
#include "Slime.h"
#include "Player.h"
#include "Part.h"
#include "Slime_Slime.h"

CSlime::CSlime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSlime::CSlime(const CSlime& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSlime::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlime::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	if (FAILED(Add_PartObjects(LEVEL_SLIMEBOSS)))
		return E_FAIL;

	m_pModelCom->Set_LerpTime(0.2f);
	m_iHp = 10;
	m_pModelCom->Set_AnimSpeed(SLIME_ATTACK, 2.5f);
	return S_OK;
}

void CSlime::Tick(_float fTimeDelta)
{
	if (m_pPlayer == nullptr || m_pPlayerWorldMat == nullptr || m_pPlayerCollider == nullptr)
	{
		m_pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
		if (m_pPlayer != nullptr)
		{
			Safe_AddRef(m_pPlayer);
			m_pPlayerWorldMat = m_pPlayer->Get_PlayerWorldMatrixPtr();
			m_pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));
			if(m_pPlayerCollider != nullptr)
				Safe_AddRef(m_pPlayerCollider);
		}
	}

	if (m_pPlayer == nullptr)
		return;

	if (m_eState != SLIME_ATTACK && m_iHp <= 0)
		m_eState = SLIME_DEATH;

	if (SLIME_DEATH == m_eState)
	{
		DeathEffect();
		Set_Dead();
	}

	m_fTime += fTimeDelta;

	if (m_eState == SLIME_ATTACK && m_pModelCom->isFinished()) // 자폭공격
	{
		AttackEffect();
		Set_Dead();
	}
	
	if (m_pPlayer->Get_InSlimeBossRoom())
		Set_Dead();

	if (m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y <= 0)
		m_pTransformCom->Fall(-fTimeDelta * 1.1f);

	Check_SlimeCol(fTimeDelta);
	Check_PillarCol(fTimeDelta);
	Check_SlimeBossCol(fTimeDelta);

	_vector vPlayerPos = XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1);
	m_pTransformCom->Look_At_ForLandObject(vPlayerPos);
	if(m_eState != SLIME_ATTACK)
		m_pTransformCom->Move(fTimeDelta * 2.f, XMVector3Normalize(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK)));

	if(m_pColliderCom[DEFAULT]->Intersect(m_pPlayerCollider))
		m_bAttackTrigger = true;

	if (m_bAttackTrigger == true)
		m_eState = SLIME_ATTACK;

	StateToAnimation();

	if (Check_DeathTrigger())
		Set_Dead();

	m_pColliderCom[DEFAULT]->Tick(m_pTransformCom->Get_WorldMatrix());
	if(m_pColliderCom[DAMAGE_RANGE] != nullptr)
		m_pColliderCom[DAMAGE_RANGE]->Tick(m_pTransformCom->Get_WorldMatrix());

	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
}

void CSlime::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
}

HRESULT CSlime::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pColliderCom[DEFAULT]->Render();
	if(m_pColliderCom[DAMAGE_RANGE] != nullptr)
		m_pColliderCom[DAMAGE_RANGE]->Render();
#endif
	return S_OK;
}

void CSlime::StateToAnimation()
{
	if (SLIME_DEATH == m_eState)
		return;

	_bool bLoop = true;
	_uint iIndex = m_eState;
	if (m_eState == SLIME_ATTACK)
		bLoop = false;
	m_pModelCom->Change_Animation_ByIndex(iIndex, bLoop);
}

_bool CSlime::Check_DeathTrigger()
{
	_bool bDeathTrigger = { false };

	// 슬라임보스 칼 체크
	if (m_pGameInstance->Check_Collision(LEVEL_SLIMEBOSS, TEXT("Layer_Knife"), m_pColliderCom[DEFAULT]))
		bDeathTrigger = true;

	return bDeathTrigger;
}

void CSlime::Check_SlimeCol(_float fTimeDelta)
{
	if (SLIME_ATTACK == m_eState)
		return;

	list<CGameObject*> slimeList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Slime"));

	if (slimeList.empty())
		return;

	_float3 vSrcPosFloat = m_pColliderCom[DEFAULT]->Get_Center(CCollider::TYPE_SPHERE);
	_vector vSrsPos = XMLoadFloat3(&vSrcPosFloat);

	for (auto& slime : slimeList)
	{
		if (slime == nullptr || slime == this)
			continue;

		CCollider* pSlimeCollider = dynamic_cast<CCollider*>(slime->Get_Component(TEXT("Com_Collider")));

		if (pSlimeCollider == nullptr)
			continue;

		if (m_pColliderCom[DEFAULT]->Intersect(pSlimeCollider))
		{
			_float3 vDstPosFloat = pSlimeCollider->Get_Center(CCollider::TYPE_SPHERE);
			_vector vDstPos = XMLoadFloat3(&vDstPosFloat);

			_vector vDir = XMVector3Normalize(vSrsPos - vDstPos);

			m_pTransformCom->Move(fTimeDelta, vDir);
		}
	}
}

void CSlime::Check_PillarCol(_float fTimeDelta)
{
	list<CGameObject*> pillarList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar"));

	if (pillarList.empty())
		return;

	_float3 vSrcPosFloat = m_pColliderCom[DEFAULT]->Get_Center(CCollider::TYPE_SPHERE);
	_vector vSrsPos = XMLoadFloat3(&vSrcPosFloat);

	for (auto& pillar : pillarList)
	{
		if (pillar == nullptr)
			continue;

		CCollider* pPillarWallCollider = dynamic_cast<CCollider*>(pillar->Get_Component(TEXT("Com_Collider_Wall")));

		if (pPillarWallCollider == nullptr)
			continue;

		if (m_pColliderCom[DEFAULT]->Intersect(pPillarWallCollider))
		{
			_float3 vDstPosFloat = pPillarWallCollider->Get_Center(CCollider::TYPE_OBB);
			_vector vDstPos = XMLoadFloat3(&vDstPosFloat);
			
			_vector vTemp = vSrsPos - vDstPos;
			vTemp = XMVectorSetY(vTemp, 0.f);
			_vector vDir = XMVector3Normalize(vTemp);
			
			m_pTransformCom->Move(fTimeDelta, vDir);
		}
	}
}

void CSlime::Check_SlimeBossCol(_float fTimeDelta)
{
	if (m_pSlimeBossCollider == nullptr)
	{
		CGameObject* pSlimeBoss = m_pGameInstance->Get_GameObject(LEVEL_SLIMEBOSS, TEXT("Layer_SlimeBoss"), TEXT("Prototype_GameObject_SlimeBoss"));
		if (pSlimeBoss == nullptr)
			return;
		m_pSlimeBossCollider = dynamic_cast<CCollider*>(pSlimeBoss->Get_Component(TEXT("Com_Collider")));
		if (m_pSlimeBossCollider != nullptr)
			Safe_AddRef(m_pSlimeBossCollider);
	}

	if (m_pSlimeBossCollider == nullptr)
		return;

	if (m_pColliderCom[DEFAULT]->Intersect(m_pSlimeBossCollider))
	{
		_float3 vSrcPosFloat = m_pColliderCom[DEFAULT]->Get_Center(CCollider::TYPE_SPHERE);
		_vector vSrsPos = XMLoadFloat3(&vSrcPosFloat);

		_float3 vDstPosFloat = m_pSlimeBossCollider->Get_Center(CCollider::TYPE_SPHERE);
		_vector vDstPos = XMLoadFloat3(&vDstPosFloat);

		_vector vDir = XMVector3Normalize(vSrsPos - vDstPos);

		m_pTransformCom->Move(fTimeDelta * 2.f, vDir);
	}
}

void CSlime::DeathEffect()
{
}

void CSlime::AttackEffect()
{
}

HRESULT CSlime::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.fRadius = 5.2f;
	ColliderDesc.vCenter = _float3(0.f, 0, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom[DEFAULT], &ColliderDesc)))
		return E_FAIL;

	ColliderDesc.fRadius = 15.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider_Range"), (CComponent**)&m_pColliderCom[DAMAGE_RANGE], &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlime::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlime::Add_PartObjects(LEVEL eLevel)
{
	/* For.Part_Slime */
	CPart* pSlime_Slime = { nullptr };
	CSlime_Slime::SLIME_SLIME_DESC	SlimeDesc{};

	SlimeDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SlimeDesc.pState = &m_eState;
	SlimeDesc.iLevel = eLevel;
	SlimeDesc.wstrModelTag = TEXT("Prototype_Component_Model_PartSlime");
	pSlime_Slime = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_Slime_Slime"), &SlimeDesc));
	if (nullptr == pSlime_Slime)
		return E_FAIL;
	m_Parts.emplace(TEXT("Part_Slime_Slime"), pSlime_Slime);

	return S_OK;
}

CSlime* CSlime::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlime* pInstance = new CSlime(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlime"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CSlime::Clone(void* pArg)
{
	CSlime* pInstance = new CSlime(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlime"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSlime::Free()
{
	__super::Free();

	Safe_Release(m_pSlimeBossCollider);
	Safe_Release(m_pPlayerCollider);
	Safe_Release(m_pPlayer);
	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();

	for(_int i = 0; i< COLLIDER_END; i++)
		Safe_Release(m_pColliderCom[i]);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
