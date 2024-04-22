#include "stdafx.h"
#include "Trigger.h"

CTrigger::CTrigger(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTrigger::CTrigger(const CTrigger& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTrigger::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTrigger::Initialize(void* pArg)
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

	return S_OK;
}

void CTrigger::Tick(_float fTimeDelta)
{
	CGameObject* pKart = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Kart"), TEXT("Prototype_GameObject_Kart"));
	if (nullptr == pKart)
		return;

	CCollider* pKartCollider = dynamic_cast<CCollider*>(pKart->Get_Component(TEXT("Com_Collider_Wall")));
	if (nullptr == pKartCollider)
		return;

	if (m_pColliderCom->Intersect(pKartCollider)) {
		Summon_Monsters(LEVEL_GAMEPLAY);
		Set_Dead();
	}

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CTrigger::Late_Tick(_float fTimeDelta)
{
	//m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTrigger::Render()
{
	
#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif

	return S_OK;
}

void CTrigger::Summon_Monsters(LEVEL eLevel)
{
	_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
	
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_GAMEPLAY;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_Soldier");
	tDesc.matWorld._41 = vPos.x;
	tDesc.matWorld._42 = vPos.y;
	tDesc.matWorld._43 = vPos.z;
	if(eLevel == LEVEL_GAMEPLAY)
		m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_Soldier"), TEXT("Prototype_GameObject_Soldier"), &tDesc);

}

HRESULT CTrigger::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(36, 1.5f, 30.f);
	ColliderDesc.vCenter = _float3(-ColliderDesc.vSize.x * 0.5f, ColliderDesc.vSize.y * 0.5f, 0);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTrigger::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CTrigger* CTrigger::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTrigger* pInstance = new CTrigger(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTrigger"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CTrigger::Clone(void* pArg)
{
	CTrigger* pInstance = new CTrigger(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTrigger"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTrigger::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
