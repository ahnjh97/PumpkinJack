#include "stdafx.h"
#include "SlimeBoss.h"
#include "SlimeBoss_Slime.h"
#include "Player.h"

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

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	if (FAILED(Add_PartObjects((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	m_pModelCom->Set_Animation(SLIMEBOSS_SPAWN, false);
	m_pModelCom->Set_LerpTime(0.2f);
	return S_OK;
}

void CSlimeBoss::Tick(_float fTimeDelta)
{
	if (m_pPlayer == nullptr)
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_GameObject(LEVEL_SLIMEBOSS, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player")));
		if (pPlayer != nullptr)
		{
			m_pPlayer = pPlayer;
			Safe_AddRef(m_pPlayer);
		}
	}



	TurnTowards_Player(fTimeDelta);
	Link_Animation(fTimeDelta);
	StateToAnimation();

	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
	if (m_pModelCom->Get_Shrinked())
		Set_Dead();
}

void CSlimeBoss::Late_Tick(_float fTimeDelta)
{
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

	return S_OK;
}

void CSlimeBoss::Link_Animation(_float fTimeDelta)
{
	if (m_iHp <= 0)
		m_eState = SLIMEBOSS_DEATH;

	if (m_eState == SLIMEBOSS_DEATH)
		m_pModelCom->Shrink();

	if (m_eState == SLIMEBOSS_SPAWN)
	{
		if(m_pModelCom->isFinished())
			m_eState = SLIMEBOSS_IDLE;
	}	
	else if (m_eState == SLIMEBOSS_IDLE && m_iCurPattern == -1)
	{
		if (m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_IDLE) >= 1)
		{
			m_eState = m_vecSlimeBossPattern[0];
			m_pModelCom->Reset_AnimPlayCount(SLIMEBOSS_IDLE);
		}
	}
	else
	{
		if (m_eState == SLIMEBOSS_FORK || m_eState == SLIMEBOSS_INVOKE_BEGIN
			|| m_eState == SLIMEBOSS_PILLAR_SPAWN || m_eState == SLIMEBOSS_MEGASPAWN)
		{
			if (m_pModelCom->isFinished())
			{
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
			XMStoreFloat4(&m_vLightningPos, m_pPlayer->Get_PlayerPos());

			_float fPlayRatio = m_pModelCom->Get_AnimPlayRatio(SLIMEBOSS_LIGHTNING);
			if (m_iLightningCount == m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_LIGHTNING) && fPlayRatio > 0.7f)
			{
				_float fHeight = 45.f;
				CGameObject::GAMEOBJECT_DESC tDesc{};
				tDesc.matWorld._41 = m_vLightningPos.x;
				tDesc.matWorld._42 = m_vLightningPos.y + fHeight;
				tDesc.matWorld._43 = m_vLightningPos.z;
				tDesc.iLevel = LEVEL_SLIMEBOSS;
				tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Knife");
				m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_Knife"), TEXT("Prototype_GameObject_Knife"), &tDesc);
				m_iLightningCount++;
			}

			if (m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_LIGHTNING) >= 5)
			{
				m_eState = m_vecSlimeBossPattern[++m_iCurPattern];
				m_pModelCom->Reset_AnimPlayCount(SLIMEBOSS_LIGHTNING);
			}
		}
		else if (m_eState == SLIMEBOSS_IDLE)
		{
			if (m_pModelCom->Get_AnimPlayCount(SLIMEBOSS_IDLE) >= 1)
			{
				m_eState = m_vecSlimeBossPattern[++m_iCurPattern];
				m_pModelCom->Reset_AnimPlayCount(SLIMEBOSS_IDLE);
				m_iLightningCount = 0;
			}
		}
	}
}

void CSlimeBoss::StateToAnimation()
{
	_bool bLoop = true;
	_uint iIndex = m_eState;
	if (m_eState == SLIMEBOSS_SPAWN || m_eState == SLIMEBOSS_INVOKE_BEGIN|| m_eState == SLIMEBOSS_DEATH || m_eState == SLIMEBOSS_FORK 
		|| m_eState == SLIMEBOSS_PILLAR_SPAWN || m_eState == SLIMEBOSS_MEGASPAWN)
		bLoop = false;
	m_pModelCom->Change_Animation_ByIndex(iIndex, bLoop);
}

void CSlimeBoss::TurnTowards_Player(_float fTimeDelta)
{
	if (m_eState == SLIMEBOSS_DEATH)
		return;

	_vector	vLook = m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK);

	_vector vPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vPlayerPos = m_pPlayer->Get_PlayerPos();
	_vector vDirection = XMVector3Normalize(vPos - vPlayerPos);
	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, vDirection));
	_float fY = XMVectorGetY(::XMVector3Cross(vLook, vDirection));
	if (fY < 0)
		fAngle = -fAngle;
	if (abs(fAngle) >= XMConvertToRadians(2.f))
		m_pTransformCom->Turn(::XMVectorSet(0, 1, 0, 0), fTimeDelta * fAngle * 7.f);
}

HRESULT CSlimeBoss::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = {};
	if (m_pModelCom->Get_ModelType() == TYPE_ANIM)
		wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	else
		wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");

	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
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
	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

}
