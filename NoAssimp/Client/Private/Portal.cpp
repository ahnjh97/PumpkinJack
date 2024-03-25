#include "stdafx.h"
#include "Portal.h"
#include <Player.h>

CPortal::CPortal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPortal::CPortal(const CPortal& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CPortal::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPortal::Initialize(void* pArg)
{
	GAMEOBJECT_DESC		GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	return S_OK;
}

void CPortal::Tick(_float fTimeDelta) 
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_GameObject(LEVEL_SLIMEBOSS, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player")));
	if (pPlayer)
	{
		if (pPlayer->Get_InSlimeBossRoom())
			Set_Dead();
	}

	if (m_pColliderCom->Intersect((CCollider*)m_pGameInstance->Get_Component(LEVEL_SLIMEBOSS, TEXT("Layer_Player"), TEXT("Com_Collider"))) 
		&& m_bSpawnSlimeBossRoomPillars == false)
	{
		_float4 vPos = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION);
		CGameObject::GAMEOBJECT_DESC tDesc{};
		tDesc.iLevel = LEVEL_SLIMEBOSS;
		tDesc.wstrModelTag = TEXT("Prototype_Component_Model_cemeteryPillar"); 

		_float fDis = 40.f;
		for (_int i = 0; i < 3; i++)
		{
			_float fAngle = i * XM_2PI / 3.f;
			tDesc.matWorld._41 = 205 + cosf(fAngle) * fDis;
			tDesc.matWorld._42 = -162;
			tDesc.matWorld._43 = 150 + sinf(fAngle) * fDis;
			m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_Pillar2"), TEXT("Prototype_GameObject_Pillar_SlimeBossRoom"), &tDesc);
		}

		_float fHeadAngle = { XM_PI * 3.f };
		_float3 vHeadPos = _float3(205 + cosf(fHeadAngle) * fDis, -138, 150 + sinf(fHeadAngle) * fDis);
		SlimeBossHeadSpawn(vHeadPos);
		m_bSpawnSlimeBossRoomPillars = true;
	}

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CPortal::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CPortal::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	m_pColliderCom->Render();
#endif

	return S_OK;
}

void CPortal::SlimeBossHeadSpawn(_float3 vPos)
{
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_SLIMEBOSS;
	tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SlimeBossHead");
	tDesc.matWorld._41 = vPos.x;
	tDesc.matWorld._42 = vPos.y;
	tDesc.matWorld._43 = vPos.z;
	m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_SlimeBossHead"), TEXT("Prototype_GameObject_SlimeBossHead"), &tDesc);
}

HRESULT CPortal::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Portal"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	_float fHeight = m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC	 ColliderDesc{};
	ColliderDesc.fRadius = 18;
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.fRadius * 0.5f + fHeight, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPortal::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

CPortal* CPortal::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPortal* pInstance = new CPortal(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CPortal"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CPortal::Clone(void* pArg)
{
	CPortal* pInstance = new CPortal(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CPortal"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPortal::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
