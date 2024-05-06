#include "Collider.h"
#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_SPHERE.h"
#include "GameInstance.h"
#include "GameObject.h"

CCollider::CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CCollider::CCollider(const CCollider& rhs)
	: CComponent{ rhs }
	, m_eType{ rhs.m_eType }
#ifdef _DEBUG
	, m_pInputLayout{ rhs.m_pInputLayout }
	, m_pEffect{ rhs.m_pEffect }
	, m_pBatch{ rhs.m_pBatch }
#endif
{
#ifdef _DEBUG
	Safe_AddRef(m_pInputLayout);
#endif
}

_float CCollider::Get_HightestPoint()
{
	if (m_eType == TYPE_AABB)
	{
		BoundingBox* pAABB = (BoundingBox*)m_pBounding->Get_BoundingDesc();
		return _float(pAABB->Center.y + pAABB->Extents.y);
	}
	else if (m_eType == TYPE_OBB)
	{
		BoundingOrientedBox* pOBB = (BoundingOrientedBox*)m_pBounding->Get_BoundingDesc();
		return _float(pOBB->Center.y + pOBB->Extents.y);
	}
	else if (m_eType == TYPE_SPHERE)
	{
		BoundingSphere* pSphere = (BoundingSphere*)m_pBounding->Get_BoundingDesc();
		return _float(pSphere->Center.y + pSphere->Radius);
	}

	return _float();
}

_float3 CCollider::Get_Center()
{
	if (m_eType == TYPE_AABB)
	{
		BoundingBox* pAABB = (BoundingBox*)m_pBounding->Get_BoundingDesc();
		return pAABB->Center;
	}
	else if (m_eType == TYPE_OBB)
	{
		BoundingOrientedBox* pOBB = (BoundingOrientedBox*)m_pBounding->Get_BoundingDesc();
		return pOBB->Center;
	}
	else if (m_eType == TYPE_SPHERE)
	{
		BoundingSphere* pSphere = (BoundingSphere*)m_pBounding->Get_BoundingDesc();
		return pSphere->Center;
	}

	return _float3();
}

_float3 CCollider::Get_ColliderLook()
{
	return m_pBounding->Get_ColliderLook();
}

_bool CCollider::Check_AlreadyCollided(CCollider* pEnemyCollider)
{
	if (m_CollidedColliders.find(pEnemyCollider) != m_CollidedColliders.end())
		return true;

	return _bool();
}

HRESULT CCollider::Initialize_Prototype(TYPE eType)
{
	m_eType = eType;

#ifdef _DEBUG
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);

	m_pEffect = new BasicEffect(m_pDevice);

	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode = { nullptr };
	size_t iShaderByteCodeLength = { 0 };

	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

	m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iShaderByteCodeLength, &m_pInputLayout);
#endif

	return S_OK;
}

HRESULT CCollider::Initialize(void* pArg)
{
	CBounding::BOUNDING_DESC* pBoundDesc = (CBounding::BOUNDING_DESC*)pArg;
	switch (m_eType)
	{
	case TYPE_AABB:
		m_pBounding = CBounding_AABB::Create(m_pDevice, m_pContext, pBoundDesc);
		break;
	case TYPE_OBB:
		m_pBounding = CBounding_OBB::Create(m_pDevice, m_pContext, pBoundDesc);
		break;
	case TYPE_SPHERE:
		m_pBounding = CBounding_Sphere::Create(m_pDevice, m_pContext, pBoundDesc);
		break;
	}
	
	return S_OK;
}

void CCollider::Tick(_fmatrix WorldMatrix)
{
	m_pBounding->Tick(WorldMatrix);
}

_bool CCollider::Intersect(CCollider* pTargetCollider)
{
	if (m_bDisabled)
		return false;

	_bool bIntersect = false;
	if (pTargetCollider != nullptr)
	{
		bIntersect = m_pBounding->Intersect(pTargetCollider->m_eType, pTargetCollider->m_pBounding);
		if (true == bIntersect)
			m_CollidedColliders.insert(pTargetCollider);
	}

	return bIntersect;
}

_bool CCollider::Intersect_Ray(_fvector vRayOrigin, _fvector vRayDir, _float& fDis)
{
	return m_pBounding->Intersect_Ray(vRayOrigin, vRayDir, fDis);
}

#ifdef _DEBUG
HRESULT CCollider::Render()
{
	if (!m_pGameInstance->Get_Toggle())
		return S_OK;

	if (m_bDisabled)
		return S_OK;

	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW));
	m_pEffect->SetProjection(m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ));

	m_pContext->IASetInputLayout(m_pInputLayout);

	m_pEffect->Apply(m_pContext);

	m_pBatch->Begin();

	m_pBounding->Render(m_pBatch);

	m_pBatch->End();

	return S_OK;
}
#endif // DEBUG

CCollider* CCollider::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType)
{
	CCollider* pInstance = new CCollider(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType)))
	{
		MSG_BOX(TEXT("Failed To Create : CCollider"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CComponent* CCollider::Clone(void* pArg)
{
	CComponent* pInstance = new CCollider(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CCollider"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCollider::Free()
{
	__super::Free();

#ifdef _DEBUG
	Safe_Release(m_pInputLayout);

	if (false == m_isCloned)
	{
		Safe_Delete(m_pEffect);
		Safe_Delete(m_pBatch);
	}

#endif

	m_CollidedColliders.clear();
	Safe_Release(m_pBounding);
}
