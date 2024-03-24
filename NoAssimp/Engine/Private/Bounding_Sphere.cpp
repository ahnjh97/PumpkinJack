#include "Bounding_Sphere.h"
#include "DebugDraw.h"

CBounding_Sphere::CBounding_Sphere(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBounding{ pDevice, pContext }
{

}

HRESULT CBounding_Sphere::Initialize(CBounding::BOUNDING_DESC* pBoundingDesc)
{
	BOUNDING_SPHERE_DESC* pDesc = (BOUNDING_SPHERE_DESC*)pBoundingDesc;

	m_pBoundingDesc_Original = new BoundingSphere(pDesc->vCenter, pDesc->fRadius);
	m_pBoundingDesc = new BoundingSphere(*m_pBoundingDesc_Original);

	return S_OK;
}

void CBounding_Sphere::Tick(_fmatrix WorldMatrix)
{
	m_pBoundingDesc_Original->Transform(*m_pBoundingDesc, WorldMatrix);
}

_bool CBounding_Sphere::Intersect(CCollider::TYPE eType, CBounding* pBounding)
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

#ifdef _DEBUG

HRESULT CBounding_Sphere::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
	DX::Draw(pBatch, *m_pBoundingDesc, m_isCollision == true ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

#endif

CBounding_Sphere* CBounding_Sphere::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBounding::BOUNDING_DESC* pBoundingDesc)
{
	CBounding_Sphere* pInstance = new CBounding_Sphere(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pBoundingDesc)))
	{
		MSG_BOX(TEXT("Failed To Created : CBounding_Sphere"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

void CBounding_Sphere::Free()
{
	__super::Free();
	Safe_Delete(m_pBoundingDesc);
	Safe_Delete(m_pBoundingDesc_Original);
}
