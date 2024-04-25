#include "Frustum.h"
#include "GameInstance.h"

CFrustum::CFrustum()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CFrustum::Initialize()
{
	m_vOriginalPoints[0] = _float3(-1.f, 1.f, 0.f);
	m_vOriginalPoints[1] = _float3(1.f, 1.f, 0.f);
	m_vOriginalPoints[2] = _float3(1.f, -1.f, 0.f);
	m_vOriginalPoints[3] = _float3(-1.f, -1.f, 0.f);

	m_vOriginalPoints[4] = _float3(-1.f, 1.f, 1.f);
	m_vOriginalPoints[5] = _float3(1.f, 1.f, 1.f);
	m_vOriginalPoints[6] = _float3(1.f, -1.f, 1.f);
	m_vOriginalPoints[7] = _float3(-1.f, -1.f, 1.f);

	return S_OK;
}

void CFrustum::Tick()
{
	_matrix			ViewMatrixInv, ProjMatrixInv;

	ViewMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);
	ProjMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);

	_vector			vPoints[8];

	for (size_t i = 0; i < 8; i++)
	{
		vPoints[i] = XMVector3TransformCoord(XMLoadFloat3(&m_vOriginalPoints[i]), ProjMatrixInv);
		XMStoreFloat3(&m_vWorldPoints[i], XMVector3TransformCoord(vPoints[i], ViewMatrixInv));
	}

	Make_Planes(m_vWorldPoints, m_vWorldPlanes);
}

void CFrustum::Transform_LocalSpace(_fmatrix WorldMatrixInv)
{
	_float3		vLocalPos[8];

	for (size_t i = 0; i < 8; i++)
		XMStoreFloat3(&vLocalPos[i], XMVector3TransformCoord(XMLoadFloat3(&m_vWorldPoints[i]), WorldMatrixInv));

	Make_Planes(vLocalPos, m_vLocalPlanes);
}

_bool CFrustum::isIn_WorldSpace(_fvector vWorldPos, _float fRange)
{
	for (size_t i = 0; i < 6; i++)
	{
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vWorldPlanes[i]), vWorldPos)))
			return false;
	}

	return true;
}

_bool CFrustum::isIn_LocalSpace(_fvector vLocalPos, _float fRange)
{
	for (size_t i = 0; i < 6; i++)
	{
		if (fRange < XMVectorGetX(XMPlaneDotCoord(XMLoadFloat4(&m_vLocalPlanes[i]), XMVectorSetW(vLocalPos, 1.f))))
			return false;
	}

	return true;
}


void CFrustum::Make_Planes(const _float3* pPoints, _float4* pPlanes)
{
	XMStoreFloat4(&pPlanes[0], XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&pPoints[1]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[5]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[6]), 1.f)));
	XMStoreFloat4(&pPlanes[1], XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&pPoints[7]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[4]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[0]), 1.f)));

	XMStoreFloat4(&pPlanes[2], XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&pPoints[4]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[5]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[1]), 1.f)));
	XMStoreFloat4(&pPlanes[3], XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&pPoints[2]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[6]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[7]), 1.f)));

	XMStoreFloat4(&pPlanes[4], XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&pPoints[5]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[4]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[7]), 1.f)));
	XMStoreFloat4(&pPlanes[5], XMPlaneFromPoints(XMVectorSetW(XMLoadFloat3(&pPoints[0]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[1]), 1.f), XMVectorSetW(XMLoadFloat3(&pPoints[2]), 1.f)));
}

CFrustum* CFrustum::Create()
{
	CFrustum* pInstance = new CFrustum();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CFrustum"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CFrustum::Free()
{
	Safe_Release(m_pGameInstance);
}
