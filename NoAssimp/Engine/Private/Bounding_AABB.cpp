#include "Bounding_AABB.h"
#include "DebugDraw.h"

CBounding_AABB::CBounding_AABB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBounding{ pDevice, pContext }
{

}

HRESULT CBounding_AABB::Initialize(CBounding::BOUNDING_DESC* pBoundingDesc)
{
	BOUNDING_AABB_DESC* pDesc = (BOUNDING_AABB_DESC*)pBoundingDesc;

	m_pBoundingDesc_Original = new BoundingBox(pDesc->vCenter, _float3(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f));
	m_pBoundingDesc = new BoundingBox(*m_pBoundingDesc_Original);

	return S_OK;
}

void CBounding_AABB::Tick(_fmatrix WorldMatrix)
{
	_matrix		TransformMatrix = WorldMatrix;

	TransformMatrix.r[0] = XMVectorSet(1.f, 0.f, 0.f, 0.f) * XMVector3Length(WorldMatrix.r[0]);
	TransformMatrix.r[1] = XMVectorSet(0.f, 1.f, 0.f, 0.f) * XMVector3Length(WorldMatrix.r[1]);
	TransformMatrix.r[2] = XMVectorSet(0.f, 0.f, 1.f, 0.f) * XMVector3Length(WorldMatrix.r[2]);

	m_pBoundingDesc_Original->Transform(*m_pBoundingDesc, TransformMatrix);
}

_bool CBounding_AABB::Intersect(CCollider::TYPE eType, CBounding* pBounding)
{
	void* pTargetBoundingDesc = pBounding->Get_BoundingDesc();

	m_isCollision = { false };

	switch (eType)
	{
	case CCollider::TYPE_AABB:
		m_isCollision = m_pBoundingDesc->Intersects(*((BoundingBox*)pTargetBoundingDesc));
		break;
	case CCollider::TYPE_OBB:
		m_isCollision = m_pBoundingDesc->Intersects(*((BoundingOrientedBox*)pTargetBoundingDesc));
		break;
	case CCollider::TYPE_SPHERE:
		m_isCollision = m_pBoundingDesc->Intersects(*((BoundingSphere*)pTargetBoundingDesc));
		break;
	}

	return m_isCollision;
}

_bool CBounding_AABB::Intersect(CBounding_AABB* pTargetBounding)
{
	m_isCollision = false;

	_float3 vSourMin = Compute_Min();
	_float3 vSourMax = Compute_Max();

	_float3 vDestMin = pTargetBounding->Compute_Min();
	_float3 vDestMax = pTargetBounding->Compute_Max();

	if (max(vSourMin.x, vDestMin.x) > min(vSourMax.x, vDestMax.x))
		return m_isCollision;
	if (max(vSourMin.y, vDestMin.y) > min(vSourMax.y, vDestMax.y))
		return m_isCollision;
	if (max(vSourMin.z, vDestMin.z) > min(vSourMax.z, vDestMax.z))
		return m_isCollision;

	return m_isCollision = true;
}

#ifdef _DEBUG

HRESULT CBounding_AABB::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
	DX::Draw(pBatch, *m_pBoundingDesc, m_isCollision == true ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

#endif

CBounding_AABB* CBounding_AABB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBounding::BOUNDING_DESC* pBoundingDesc)
{
	CBounding_AABB* pInstance = new CBounding_AABB(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pBoundingDesc)))
	{
		MSG_BOX(TEXT("Failed To Created : CBounding_AABB"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

void CBounding_AABB::Free()
{
	__super::Free();

	Safe_Delete(m_pBoundingDesc);
	Safe_Delete(m_pBoundingDesc_Original);
}
