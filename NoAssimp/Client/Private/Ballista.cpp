#include "stdafx.h"
#include "Ballista.h"
#include "Player.h"
#include "SkullCrossBow.h"

CBallista::CBallista(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CBallista::CBallista(const CBallista& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CBallista::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBallista::Initialize(void* pArg)
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

	m_pModelCom->Set_Animation(0, true); // 999: 랜덤 애니메이션
	m_pModelCom->Set_AnimSpeed(0, 0.6f);

	return S_OK;
}

void CBallista::Tick(_float fTimeDelta)
{
	if (m_pPlayerWorldMat == nullptr)
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_GAMEPLAY));
		if (pPlayer != nullptr)
		{
			m_pPlayerWorldMat = pPlayer->Get_PlayerWorldMatrixPtr();
			m_vOriginalPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		}		
	}

	m_pTransformCom->Look_At(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42 + 6.f, m_pPlayerWorldMat->_43, 1.f));
	m_pTransformCom->Set_Pos(m_vOriginalPos);

	if (BALLISTA_IDLE == m_eState && m_fIdleTime > 6.5f)
		m_eState = BALLISTA_SHOOT;

	if (BALLISTA_SHOOT == m_eState) {
		Emit_AlarmLight();

		_float fAnimPlayRatio = m_pModelCom->Get_AnimPlayRatio();
		if (m_iArrowCount == m_pModelCom->Get_AnimPlayCount(0))	{
			Add_Arrow();
			m_iArrowCount++;
		}

		if (m_pModelCom->Get_AnimPlayCount(0) >= 6) {
			m_eState = BALLISTA_IDLE;
			m_pModelCom->Reset_AnimPlayCount(0);
			m_iArrowCount = 0;
		}
	}

	if (BALLISTA_IDLE == m_eState)
		m_fIdleTime += fTimeDelta;
	else
		m_fIdleTime = 0.f;

	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
}

void CBallista::Late_Tick(_float fTimeDelta)
{
	if(BALLISTA_SHOOT == m_eState)
		m_pModelCom->Play_Animation(fTimeDelta);
	else
		m_pModelCom->Play_Animation(0);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
}

HRESULT CBallista::Render()
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

void CBallista::Add_Arrow()
{
	_float4 vPos = m_pTransformCom->Get_Pos_Float4();

	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_GAMEPLAY;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_ballistaArrow");
	tDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Arrow"), TEXT("Prototype_GameObject_Arrow"), &tDesc);
}

void CBallista::Emit_AlarmLight()
{
}

HRESULT CBallista::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBallista::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCamFar", m_pGameInstance->Get_CamFarPtr(), sizeof(_float))))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CBallista::Add_PartObjects(LEVEL eLevel)
{
	CModel* pModel = dynamic_cast<CModel*>(Get_Component(TEXT("Com_Model")));

	/* For.SkullCrossBow */
	CPart* pSkullCrossBow = { nullptr };
	CSkullCrossBow::SKULL_CROSSBOW_DESC	SkullCrossBowDesc{};

	SkullCrossBowDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SkullCrossBowDesc.pSocket = pModel->Get_BonePtr("Bone");
	SkullCrossBowDesc.pState = &m_eState;
	SkullCrossBowDesc.iLevel = eLevel;
	SkullCrossBowDesc.wstrModelTag = TEXT("Prototype_Component_Model_SkullCrossBow");
	pSkullCrossBow = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_SkullCrossBow"), &SkullCrossBowDesc));

	if (nullptr == pSkullCrossBow)
		return E_FAIL;

	m_Parts.emplace(TEXT("Part_SkullCrossBow"), pSkullCrossBow);

	return S_OK;
}

CBallista* CBallista::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBallista* pInstance = new CBallista(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBallista"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CBallista::Clone(void* pArg)
{
	CBallista* pInstance = new CBallista(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CBallista"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBallista::Free()
{
	__super::Free();

	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
