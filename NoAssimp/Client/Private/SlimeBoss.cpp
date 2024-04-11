#include "stdafx.h"
#include "SlimeBoss.h"
#include "SlimeBoss_Slime.h"
#include "Player.h"
#include "Portal.h"

CSlimeBoss::CSlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSlimeBoss::CSlimeBoss(const CSlimeBoss& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSlimeBoss::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlimeBoss::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_iLevel = GameObjectDesc.iLevel = LEVEL_SLIMEBOSS;
	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	if (FAILED(Add_PartObjects((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	m_pModelCom->Set_Animation(SLIMEBOSS_SPAWN, false);
	m_pModelCom->Set_LerpTime(0.2f);
	m_pModelCom->Set_AnimSpeed(SLIMEBOSS_MEGASPAWN, 0.7f);
	m_iHp = 100;
	return S_OK;
}

void CSlimeBoss::Tick(_float fTimeDelta)
{
	if (m_pPlayer == nullptr)
	{
		m_pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
		if (m_pPlayer != nullptr)
		{
			Safe_AddRef(m_pPlayer);
			m_pPlayerWorldMatrix = m_pPlayer->Get_PlayerWorldMatrixPtr();
		}
	}

	if (m_pPlayer != nullptr)
	{
		if (m_pPlayer->Get_InSlimeBossRoom())
		{
			m_bPillarSpawn = false;
			m_bPortalSpawn = false;
			m_bSlimeSpawn = false;
			m_bSlimeDamageZoneSpawn = false;
			m_eState = SLIMEBOSS_HIT;
			m_iCurPattern = 0;
			return;
		}
			
		if (m_pPlayer->Get_SlimeBossRoomEscapeCount() >= 2)
			m_eState = SLIMEBOSS_DEATH;
	}

	TurnTowards_Player(fTimeDelta);
	Link_Animation(fTimeDelta);
	StateToAnimation();

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());

	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
	if (m_pModelCom->Get_Shrinked())
	{
		DeathEffect();
		Set_Dead();
	}

}

void CSlimeBoss::Late_Tick(_float fTimeDelta)
{
	if (m_pPlayer != nullptr)
	{
		if (m_pPlayer->Get_InSlimeBossRoom())
			return;
	}
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
}

HRESULT CSlimeBoss::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}
#ifdef _DEBUG
	//m_pColliderCom->Render();
#endif
	return S_OK;
}

void CSlimeBoss::Link_Animation(_float fTimeDelta)
{
	/*if (m_iHp <= 0)
		m_eState = SLIMEBOSS_DEATH;*/

	if (m_eState == SLIMEBOSS_DEATH)
	{
		if (m_pModelCom->isFinished())
		{
			_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
			m_pModelCom->Shrink();
			m_pTransformCom->Move_toTarget(XMVectorSet(vPos.x, 50.f, vPos.z, 1.f), fTimeDelta * 1.3f, 2.f);
		}
	}

	if (m_eState == SLIMEBOSS_HIT && m_pModelCom->isFinished())
		m_eState = m_vecSlimeBossPattern[0];
		
	if (m_eState == SLIMEBOSS_SPAWN)
	{
		if (m_pModelCom->isFinished())
		{
			KnifeSpawn();
			SlimeSpawn();
			PillarSpawn();
			PortalSpawn();
			m_eState = SLIMEBOSS_IDLE;
		}
	}	
	else if (m_eState == SLIMEBOSS_IDLE && m_iCurPattern == -1)
	{
		if (m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_IDLE) >= 1)
		{
			m_iCurPattern = 1;
			m_eState = m_vecSlimeBossPattern[m_iCurPattern];
			m_pModelCom->Reset_AnimPlayCount(SLIMEBOSS_IDLE);
		}
	}
	else
	{
		if (m_eState == SLIMEBOSS_FORK || m_eState == SLIMEBOSS_INVOKE_BEGIN
			|| m_eState == SLIMEBOSS_PILLAR_SPAWN || m_eState == SLIMEBOSS_MEGASPAWN)
		{
			if (m_eState == SLIMEBOSS_INVOKE_BEGIN && m_pModelCom->Get_AnimPlayRatio() > 0.32f)
			{
				if (m_pPlayer->Get_SlimeBossRoomEscapeCount() >= 1 && m_bSlimeDamageZoneSpawn == false)
				{
					SlimeDamageZoneSpawn(10);
					m_bSlimeDamageZoneSpawn = true;
				}
			}

			if (m_eState == SLIMEBOSS_PILLAR_SPAWN && m_bPillarSpawn == false)
			{
				PillarSpawn();
				m_bPillarSpawn = true;
			}

			if (m_eState == SLIMEBOSS_MEGASPAWN && m_bPortalSpawn == false)
			{
				PortalSpawn();
				m_bPortalSpawn = true;
			}

			if (m_pModelCom->isFinished() && m_ePreState != SLIMEBOSS_HIT) {
				++m_iCurPattern;
				if (m_iCurPattern >= m_vecSlimeBossPattern.size())
					m_iCurPattern = 0;
				m_eState = m_vecSlimeBossPattern[m_iCurPattern];
			}
		}
		else if (m_eState == SLIMEBOSS_INVOKE)
		{
			if (m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_INVOKE) >= 3)
			{
				m_eState = m_vecSlimeBossPattern[++m_iCurPattern];
				m_pModelCom->Reset_AnimPlayCount(SLIMEBOSS_INVOKE);
			}
				
		}
		else if (m_eState == SLIMEBOSS_LIGHTNING)
		{
			_float fPlayRatio = m_pModelCom->Get_AnimPlayRatio();
			if (m_iLightningCount == m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_LIGHTNING) && fPlayRatio > 0.6f)
				KnifeSpawn();

			if (m_bSlimeSpawn == false)	{
				SlimeSpawn();
				m_bSlimeSpawn = true;
			}

			if (m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_LIGHTNING) >= 5)
			{
				m_eState = m_vecSlimeBossPattern[++m_iCurPattern];
				m_pModelCom->Reset_AnimPlayCount(SLIMEBOSS_LIGHTNING);
				m_iLightningCount = 0;
			}
		}
	}
	m_ePreState = m_eState;
}

void CSlimeBoss::StateToAnimation()
{
	_bool bLoop = true;
	_uint iIndex = m_eState;
	if (m_eState == SLIMEBOSS_SPAWN || m_eState == SLIMEBOSS_INVOKE_BEGIN|| m_eState == SLIMEBOSS_DEATH || m_eState == SLIMEBOSS_FORK 
		|| m_eState == SLIMEBOSS_PILLAR_SPAWN || m_eState == SLIMEBOSS_MEGASPAWN || m_eState == SLIMEBOSS_HIT)
		bLoop = false;
	m_pModelCom->Change_Animation_ByIndex(iIndex, bLoop);
}

void CSlimeBoss::TurnTowards_Player(_float fTimeDelta)
{
	if (m_bTurnToPlayer == true)
		return;

	_vector vPlayerPos = XMVectorSet(m_pPlayerWorldMatrix->_41, m_pPlayerWorldMatrix->_42, m_pPlayerWorldMatrix->_43, 1);
	m_pTransformCom->Look_At_ForLandObject(vPlayerPos);
	if (m_eState == SLIMEBOSS_DEATH)
		m_bTurnToPlayer = true;
}

void CSlimeBoss::KnifeSpawn()
{
	XMStoreFloat4(&m_vLightningPos, XMVectorSet(m_pPlayerWorldMatrix->_41, m_pPlayerWorldMatrix->_42, m_pPlayerWorldMatrix->_43, 1));

	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.matWorld._41 = m_vLightningPos.x;
	tDesc.matWorld._42 = 85.f;
	tDesc.matWorld._43 = m_vLightningPos.z;
	tDesc.iLevel = LEVEL_SLIMEBOSS;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Knife");
	m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_Knife"), TEXT("Prototype_GameObject_Knife"), &tDesc);
	m_iLightningCount++;
}

void CSlimeBoss::PillarSpawn()
{
	_float4 vPillarPos = {};
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));
	_float4 vRight = {};
	XMStoreFloat4(&vRight, XMVector3Normalize(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT)));
	XMStoreFloat4(&vPillarPos, vPos + vLook * 40.f);

	wstring wstrLayer = TEXT("Layer_Pillar");
	wstring wstrPrototype = TEXT("Prototype_GameObject_cemeteryPillar");
	CGameObject::GAMEOBJECT_DESC tDesc{};
 	tDesc.iLevel = LEVEL_SLIMEBOSS;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_cemeteryPillar");
	
	tDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	tDesc.matWorld._41 = vPillarPos.x + vRight.x * 13.f;
	tDesc.matWorld._42 = -42;
	tDesc.matWorld._43 = vPillarPos.z + vRight.z * 13.f;
	if(FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, wstrLayer, wstrPrototype, &tDesc)))
		return;
	
	tDesc.matWorld._41 = vPillarPos.x - vRight.x * 13.f;
	tDesc.matWorld._43 = vPillarPos.z - vRight.z * 13.f;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, wstrLayer, wstrPrototype, &tDesc)))
		return;
}

void CSlimeBoss::PortalSpawn()
{
	_float4 vPortalPos = {};
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	//_vector vLook = -XMVector3Normalize(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));
	XMStoreFloat4(&vPortalPos, vPos);

	wstring wstrLayer = TEXT("Layer_Portal");
	wstring wstrPrototype = TEXT("Prototype_GameObject_Portal");

	CPortal::PORTAL_DESC portalDesc{};
	portalDesc.pPortalBone = m_pModelCom->Get_BonePtr("tongue1");
	portalDesc.iLevel = LEVEL_SLIMEBOSS;
	portalDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	portalDesc.matWorld._43 = 0.3f;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, wstrLayer, wstrPrototype, &portalDesc)))
		return;
}

void CSlimeBoss::SlimeSpawn()
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);

	wstring wstrLayer = TEXT("Layer_Slime");
	wstring wstrPrototype = TEXT("Prototype_GameObject_Slime");

	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_SLIMEBOSS;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Slime");
	_int iSlimeNum = 12;
	_float fDis = 38.f;
	for (_int i = 0; i < iSlimeNum; i++)
	{
		tDesc.matWorld._41 = XMVectorGetX(vPos) + cosf(i * XM_2PI / iSlimeNum) * fDis;
		tDesc.matWorld._42 = -10;
		tDesc.matWorld._43 = XMVectorGetZ(vPos) + sinf(i * XM_2PI / iSlimeNum) * fDis;
		if (m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, wstrLayer, wstrPrototype, &tDesc))
			return;
	}
}

void CSlimeBoss::SlimeDamageZoneSpawn(_uint iNum)
{
	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	wstring wstrLayer = TEXT("Layer_SlimeDamageZone");
	wstring wstrPrototype = TEXT("Prototype_GameObject_SlimeDamageZone");

	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_SLIMEBOSS;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SlimeDamageZone");
	_float fDis = 50.f;
	for (_uint i = 0; i < iNum; i++)
	{
		if (i >= iNum / 2)
			fDis = 105.f;

		_float fAngle = XMConvertToRadians(360.f/iNum * 2 * i + m_pGameInstance->RandomNumber(-30, 30));
		tDesc.matWorld._41 = XMVectorGetX(vPos) + cosf(fAngle) * fDis;
		tDesc.matWorld._42 = -5;
		tDesc.matWorld._43 = XMVectorGetZ(vPos) + sinf(fAngle) * fDis;;
		if (m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, wstrLayer, wstrPrototype, &tDesc))
			return;
	}
}

void CSlimeBoss::DeathEffect()
{
}

HRESULT CSlimeBoss::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC	ColliderDesc{};
	ColliderDesc.fRadius = 24.f;
	ColliderDesc.vCenter = _float3(0.f, 0, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeBoss::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_LightDesc(0);
	if (nullptr == pLightDesc)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CSlimeBoss::Add_PartObjects(LEVEL eLevel)
{
	/* For.Part_SlimeBoss */
	CPart* pSlimeBoss_Slime = { nullptr };
	CSlimeBoss_Slime::SLIMEBOSS_SLIME_DESC	SlimeBossDesc{};

	SlimeBossDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SlimeBossDesc.pState = &m_eState;
	SlimeBossDesc.iLevel = eLevel;
	SlimeBossDesc.wstrModelTag = TEXT("Prototype_Component_Model_PartSlimeBoss");
	pSlimeBoss_Slime = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_SlimeBoss_Slime"), &SlimeBossDesc));
	if (nullptr == pSlimeBoss_Slime)
		return E_FAIL;
	m_Parts.emplace(TEXT("Part_SlimeBoss_Slime"), pSlimeBoss_Slime);
	return S_OK;
}

CSlimeBoss* CSlimeBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlimeBoss* pInstance = new CSlimeBoss(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBoss"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CSlimeBoss::Clone(void* pArg)
{
	CSlimeBoss* pInstance = new CSlimeBoss(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBoss"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CSlimeBoss::Free()
{
	__super::Free();
	Safe_Release(m_pPlayer);
	Safe_Release(m_pColliderCom);
	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
