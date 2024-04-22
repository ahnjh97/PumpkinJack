#include "Bounding_OBB.h"
#include "DebugDraw.h"

CBounding_OBB::CBounding_OBB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBounding{ pDevice, pContext }
{

}

_float3 CBounding_OBB::Get_ColliderLook()
{
	_vector vQuaternion = XMLoadFloat4(&m_pBoundingDesc->Orientation);
	_matrix matRotation = XMMatrixRotationQuaternion(vQuaternion);
	_vector vLook = matRotation.r[2]; // ZÃà
	vLook = XMVectorSetY(vLook, 0.f);
	vLook = XMVector3Normalize(vLook);
	_float3 vResult{};
	XMStoreFloat3(&vResult, vLook);

	return vResult;
}

HRESULT CBounding_OBB::Initialize(CBounding::BOUNDING_DESC* pBoundingDesc)
{
	BOUNDING_OBB_DESC* pDesc = (BOUNDING_OBB_DESC*)pBoundingDesc;

	_float4 vOrientation;
	XMStoreFloat4(&vOrientation, XMQuaternionRotationRollPitchYaw(pDesc->vRotation.x, pDesc->vRotation.y, pDesc->vRotation.z));

	m_pBoundingDesc_Original = new BoundingOrientedBox(pDesc->vCenter, _float3(pDesc->vSize.x * 0.5f, pDesc->vSize.y * 0.5f, pDesc->vSize.z * 0.5f), vOrientation);
	m_pBoundingDesc = new BoundingOrientedBox(*m_pBoundingDesc_Original);


	return S_OK;
}

void CBounding_OBB::Tick(_fmatrix WorldMatrix)
{
	m_pBoundingDesc_Original->Transform(*m_pBoundingDesc, WorldMatrix);
}

_bool CBounding_OBB::Intersect(CCollider::TYPE eType, CBounding* pBounding)
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

_bool CBounding_OBB::Intersect_Ray(_fvector vRayOrigin, _fvector vRayDir, _float& fDis)
{
	return m_pBoundingDesc->Intersects(vRayOrigin, vRayDir, fDis);
}

_bool CBounding_OBB::Intersect(CBounding_OBB* pTargetBounding)
{
	m_isCollision = false;

	OBB_DESC		OBBDesc[2];

	OBBDesc[0] = Compute_OBBDesc();
	OBBDesc[1] = pTargetBounding->Compute_OBBDesc();

	_float		fDistance[3] = { 0.0f };


	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < 3; j++)
		{
			fDistance[0] = fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vCenter) - XMLoadFloat3(&OBBDesc[0].vCenter), XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

			fDistance[1] = fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vExtentDir[0]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vExtentDir[1]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[0].vExtentDir[2]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

			fDistance[2] = fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vExtentDir[0]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vExtentDir[1]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j])))) +
				fabs(XMVectorGetX(XMVector3Dot(XMLoadFloat3(&OBBDesc[1].vExtentDir[2]), XMLoadFloat3(&OBBDesc[i].vAlignDir[j]))));

			if (fDistance[0] > fDistance[1] + fDistance[2])
				return m_isCollision;
		}
	}

	//_float3		vAlignDir[9];




	return m_isCollision = true;
}

#ifdef _DEBUG

HRESULT CBounding_OBB::Render(PrimitiveBatch<VertexPositionColor>* pBatch)
{
	DX::Draw(pBatch, *m_pBoundingDesc, m_isCollision == true ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

	return S_OK;
}

#endif

CBounding_OBB::OBB_DESC CBounding_OBB::Compute_OBBDesc()
{
	OBB_DESC			OBBDesc{};
	_float3				vPoints[8];

	OBBDesc.vCenter = m_pBoundingDesc->Center;
	m_pBoundingDesc->GetCorners(vPoints);

	XMStoreFloat3(&OBBDesc.vAlignDir[0], XMVector3Normalize(XMLoadFloat3(&vPoints[5]) - XMLoadFloat3(&vPoints[4])));
	XMStoreFloat3(&OBBDesc.vAlignDir[1], XMVector3Normalize(XMLoadFloat3(&vPoints[7]) - XMLoadFloat3(&vPoints[4])));
	XMStoreFloat3(&OBBDesc.vAlignDir[2], XMVector3Normalize(XMLoadFloat3(&vPoints[0]) - XMLoadFloat3(&vPoints[4])));


	XMStoreFloat3(&OBBDesc.vExtentDir[0],
		XMLoadFloat3(&OBBDesc.vAlignDir[0]) * m_pBoundingDesc->Extents.x);
	XMStoreFloat3(&OBBDesc.vExtentDir[1],
		XMLoadFloat3(&OBBDesc.vAlignDir[1]) * m_pBoundingDesc->Extents.y);
	XMStoreFloat3(&OBBDesc.vExtentDir[2],
		XMLoadFloat3(&OBBDesc.vAlignDir[2]) * m_pBoundingDesc->Extents.z);

	return OBBDesc;
}

CBounding_OBB* CBounding_OBB::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBounding::BOUNDING_DESC* pBoundingDesc)
{
	CBounding_OBB* pInstance = new CBounding_OBB(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pBoundingDesc)))
	{
		MSG_BOX(TEXT("Failed To Create : CBounding_OBB"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

void CBounding_OBB::Free()
{
	__super::Free();
	Safe_Delete(m_pBoundingDesc);
	Safe_Delete(m_pBoundingDesc_Original);
}
