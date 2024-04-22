#include "stdafx.h"
#include "Portal.h"
#include <Player.h>
#include "Bone.h"
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
	PORTAL_DESC		GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(PORTAL_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_pParentMatrix = GameObjectDesc.pParentMatrix;
	m_pSocket = GameObjectDesc.pPortalBone;
	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(GameObjectDesc.matWorld);
	return S_OK;
}

void CPortal::Tick(_float fTimeDelta) 
{
	if (m_pPlayer == nullptr || m_pPlayerCollider == nullptr)
	{
		m_pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
		if (m_pPlayer != nullptr)
		{
			Safe_AddRef(m_pPlayer);
			m_pPlayerWorldMat = m_pPlayer->Get_PlayerWorldMatrixPtr();
		}

		m_pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));
		if(m_pPlayerCollider != nullptr)
			Safe_AddRef(m_pPlayerCollider);
	}
	
	if (m_pPlayer == nullptr)
		return;

	if (m_pPlayer->Get_InSlimeBossRoom())
		Set_Dead();

	m_pTransformCom->Turn(XMVectorSet(0, 0, 1, 0), -fTimeDelta * 1.7f);

	if (m_pColliderCom->Intersect(m_pPlayerCollider) && m_bSpawnSlimeBossRoomPillars == false)
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
}

void CPortal::Late_Tick(_float fTimeDelta)
{
	_matrix matCombined = XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix());
	_vector vScale, vRot, vTrans;
	::XMMatrixDecompose(&vScale, &vRot, &vTrans, matCombined);

	_vector vNewScale = XMVectorSet(m_fScale, m_fScale, m_fScale, 0.0f);
	_matrix matNew = ::XMMatrixAffineTransformation(vNewScale, XMVectorZero(), XMVectorZero(), vTrans);

	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * matNew * XMLoadFloat4x4(m_pParentMatrix));

	if (m_bInitializeColliderPos)
	{
		m_pColliderCom->Tick(XMLoadFloat4x4(&m_WorldMatrix));
		m_bInitializeColliderPos = false;
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CPortal::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(3))) // Portal
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
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
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
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

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC	 ColliderDesc{};
	ColliderDesc.fRadius = 20 / m_fScale;
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPortal::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
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

	Safe_Release(m_pSocket);
	Safe_Release(m_pPlayerCollider);
	Safe_Release(m_pPlayer);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
