#include "stdafx.h"
#include "Pillar_SlimeBossRoom.h"
#include "Player.h"

CPillar_SlimeBossRoom::CPillar_SlimeBossRoom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPillar_SlimeBossRoom::CPillar_SlimeBossRoom(const CPillar_SlimeBossRoom& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CPillar_SlimeBossRoom::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPillar_SlimeBossRoom::Initialize(void* pArg)
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

	//m_pTransformCom->Rotation(XMVectorSet(0, 1, 0, 0), XMConvertToRadians(m_pGameInstance->RandomNumber(0, 360)));
	m_pTransformCom->Turn(XMVectorSet(1, 0, 1, 0), 0.1f);
	m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), XMConvertToRadians(m_pGameInstance->RandomNumber(0, 360) / 90.f));
	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), XMConvertToRadians(m_pGameInstance->RandomNumber(0, 360) / 90.f));
	m_iPreHp = m_iHp = 40;
	return S_OK;
}

void CPillar_SlimeBossRoom::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	m_fPlayRatio = m_fTime / m_fEndTime;
	if (m_fPlayRatio > 1.f)
		m_fPlayRatio = 1.f;

	if(m_iHp <=0)
		Set_Dead();

	if (Check_HeadCol())
		Set_Dead();

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CPillar_SlimeBossRoom::Late_Tick(_float fTimeDelta)
{
	if (m_iPreHp != m_iHp)
	{
		if (m_fTime >= 0.3f)
			m_fTime = 0.f;
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_iPreHp = m_iHp;
}

HRESULT CPillar_SlimeBossRoom::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(11)))  // Damagable
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif

	return S_OK;
}

_bool CPillar_SlimeBossRoom::Check_HeadCol()
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
	_bool bInSlimeBossRoom = false;
	if (pPlayer)
		bInSlimeBossRoom = pPlayer->Get_InSlimeBossRoom();

	if (bInSlimeBossRoom == false)
		return false;

	list<CGameObject*> headList = m_pGameInstance->Get_ObjectList(LEVEL_SLIMEBOSS, TEXT("Layer_SlimeBossHead"));
	if (headList.empty())
		return false;

	for (auto& head : headList)
	{
		if (head == nullptr)
			continue;

		CCollider* pHeadCollider = dynamic_cast<CCollider*>(head->Get_Component(TEXT("Com_Collider")));

		if (m_pColliderCom->Intersect(pHeadCollider))
			return true;
	}

	return false;
}

HRESULT CPillar_SlimeBossRoom::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(10, 39, 8);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;
	return S_OK;
}

HRESULT CPillar_SlimeBossRoom::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CPillar_SlimeBossRoom* CPillar_SlimeBossRoom::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPillar_SlimeBossRoom* pInstance = new CPillar_SlimeBossRoom(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPillar_SlimeBossRoom"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CPillar_SlimeBossRoom::Clone(void* pArg)
{
	CPillar_SlimeBossRoom* pInstance = new CPillar_SlimeBossRoom(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CPillar_SlimeBossRoom"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPillar_SlimeBossRoom::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
