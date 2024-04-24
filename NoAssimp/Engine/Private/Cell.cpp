#include "Cell.h"
#include "VIBuffer_Cell.h"
#include "GameInstance.h"
#include "Collider.h"

CCell::CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CCell::CCell()
{
}

_bool CCell::Get_Alone()
{
	for (_int i = 0; i < 3; i++)
	{
		if(m_iNeighborIndices[i] != -1)
			return false;
	}
	return true;
}

HRESULT CCell::Initialize(_int iTerrainIndex, const _float3* vPoints, const _int* pNeighborIndices, _uint iIndex)
{
	m_iTerrainIndex = iTerrainIndex;
	memcpy(m_vPoints, vPoints, sizeof(_float3) * POINT_END);
	memcpy(m_iNeighborIndices, pNeighborIndices, sizeof(_int) * LINE_END);
	m_iIndex = iIndex;

#ifdef _DEBUG
	m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_vPoints);
	if (nullptr == m_pVIBuffer)
		return E_FAIL;
#endif

	return S_OK;
}

HRESULT CCell::Initialize(_fvector v0, _fvector v1, _gvector v2)
{
	XMStoreFloat3(&m_vPoints[0], v0);
	XMStoreFloat3(&m_vPoints[1], v1);
	XMStoreFloat3(&m_vPoints[2], v2);

//#ifdef _DEBUG
//		m_pVIBuffer = CVIBuffer_Cell::Create(m_pDevice, m_pContext, m_vPoints);
//	if (nullptr == m_pVIBuffer)
//		return E_FAIL;
//#endif
	return S_OK;
}

HRESULT CCell::Initialize(_float3 vA, _float3 vB, _float3 vC)
{
	m_vPoints[0] = vA;
	m_vPoints[1] = vB;
	m_vPoints[2] = vC;

	return S_OK;
}

_bool CCell::IsIn(_fvector vPosition, _fmatrix TerrainWorldMatrix, _int* pNeighborIndex)
{
	_vector		vPoints[3];

	for (size_t i = 0; i < POINT_END; i++)
		vPoints[i] = XMVector3TransformCoord(XMLoadFloat3(&m_vPoints[i]), TerrainWorldMatrix);

	for (size_t i = 0; i < LINE_END; i++)
	{
		_vector		vSourDir = XMVector3Normalize(vPosition - vPoints[i]);

		_vector		vDir = vPoints[i + 1 >= POINT_END ? 0 : i + 1] - vPoints[i];
		_vector		vDestDir = XMVector3Normalize(XMVectorSet(XMVectorGetZ(vDir) * -1.f, 0.f, XMVectorGetX(vDir), 0.0f));

		if (0 < XMVectorGetX(XMVector3Dot(vSourDir, vDestDir)))
		{
			*pNeighborIndex = m_iNeighborIndices[i];
			XMStoreFloat3(&m_vLineDir, vDir);
			return false;
		}
	}

	return true;
}

_bool CCell::IsIn(_fvector vPos)
{
	_vector		vPoints[3];

	for (size_t i = 0; i < POINT_END; i++)
		vPoints[i] = XMLoadFloat3(&m_vPoints[i]);

	for (size_t i = 0; i < LINE_END; i++)
	{
		_vector		vSourDir = XMVector3Normalize(vPos - vPoints[i]);

		_vector		vDir = vPoints[i + 1 >= POINT_END ? 0 : i + 1] - vPoints[i];
		_vector		vDestDir = XMVector3Normalize(XMVectorSet(XMVectorGetZ(vDir) * -1.f, 0.f, XMVectorGetX(vDir), 0.0f));

		if (0 < XMVectorGetX(XMVector3Dot(vSourDir, vDestDir)))
			return false;
	}

	return true;
}

_bool CCell::Compare_Points(_fvector vSourPoint, _fvector vDestPoint)
{
	// XMVectorEqual();
	if (true == XMVector3Equal(vSourPoint, XMLoadFloat3(&m_vPoints[POINT_A])))
	{
		if (true == XMVector3Equal(vDestPoint, XMLoadFloat3(&m_vPoints[POINT_B])))
			return true;

		if (true == XMVector3Equal(vDestPoint, XMLoadFloat3(&m_vPoints[POINT_C])))
			return true;
	}

	if (true == XMVector3Equal(vSourPoint, XMLoadFloat3(&m_vPoints[POINT_B])))
	{
		if (true == XMVector3Equal(vDestPoint, XMLoadFloat3(&m_vPoints[POINT_C])))
			return true;

		if (true == XMVector3Equal(vDestPoint, XMLoadFloat3(&m_vPoints[POINT_A])))
			return true;
	}

	if (true == XMVector3Equal(vSourPoint, XMLoadFloat3(&m_vPoints[POINT_C])))
	{
		if (true == XMVector3Equal(vDestPoint, XMLoadFloat3(&m_vPoints[POINT_A])))
			return true;

		if (true == XMVector3Equal(vDestPoint, XMLoadFloat3(&m_vPoints[POINT_B])))
			return true;
	}

	return false;
}

_bool CCell::Check_Redundancy(CCell* pCell)
{
	if (!XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_A]), pCell->Get_Point(POINT_A)))
		return false;
	if (!XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_B]), pCell->Get_Point(POINT_B)))
		return false;
	if (!XMVector3Equal(XMLoadFloat3(&m_vPoints[POINT_C]), pCell->Get_Point(POINT_C)))
		return false;

	return true;
}

#ifdef _DEBUG

HRESULT CCell::Render()
{
	m_pVIBuffer->Bind_Buffers();
	return m_pVIBuffer->Render();
}

#endif

CCell* CCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _int iTerrainIndex, const _float3* pPoints, const _int* pNeighborIndices, _uint iIndex)
{
	CCell* pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(iTerrainIndex, pPoints, pNeighborIndices, iIndex)))
	{
		MSG_BOX(TEXT("Failed To Create : CCell"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CCell* CCell::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _fvector v0, _fvector v1, _gvector v2)
{
	CCell* pInstance = new CCell(pDevice, pContext);

	if (FAILED(pInstance->Initialize(v0, v1, v2)))
	{
		MSG_BOX(TEXT("Failed To Create : CCell"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CCell* CCell::Create(_float3 vA, _float3 vB, _float3 vC)
{
	CCell* pInstance = new CCell();

	if (FAILED(pInstance->Initialize(vA, vB, vC)))
	{
		MSG_BOX(TEXT("Failed To Create : CCell"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCell::Free()
{
#ifdef _DEBUG
	Safe_Release(m_pVIBuffer);
#endif

	Safe_Release(m_pContext);
	Safe_Release(m_pDevice);
}
