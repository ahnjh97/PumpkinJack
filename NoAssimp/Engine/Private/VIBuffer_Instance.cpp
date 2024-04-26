#include "..\Public\VIBuffer_Instance.h"

CVIBuffer_Instance::CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{

}

CVIBuffer_Instance::CVIBuffer_Instance(const CVIBuffer_Instance& rhs)
	: CVIBuffer{ rhs }
	, m_pVBInstance{ rhs.m_pVBInstance }
	, m_iInstanceStride{ rhs.m_iInstanceStride }
	, m_iNumInstance{ rhs.m_iNumInstance }
	, m_iIndexCountPerInstance{ rhs.m_iIndexCountPerInstance }
	, m_pInstanceVertices{ rhs.m_pInstanceVertices }
	, m_InstanceBufferDesc{ rhs.m_InstanceBufferDesc }
	, m_InstanceSubResourceData{ rhs.m_InstanceSubResourceData }
	, m_pSpeeds{ rhs.m_pSpeeds }
	, m_InstanceDesc{ rhs.m_InstanceDesc }
{
	m_pLifeTimes = new _float2[m_iNumInstance];
	memcpy(m_pLifeTimes, rhs.m_pLifeTimes, sizeof(_float2) * m_iNumInstance);

	Safe_AddRef(m_pVBInstance);
}

HRESULT CVIBuffer_Instance::Initialize_Prototype(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	m_RandomNumber = mt19937_64(m_RandomDevice());

	m_InstanceDesc = InstanceDesc;

	return S_OK;
}

HRESULT CVIBuffer_Instance::Initialize(void* pArg)
{
 	if (FAILED(m_pDevice->CreateBuffer(&m_InstanceBufferDesc, &m_InstanceSubResourceData, &m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}

HRESULT CVIBuffer_Instance::Bind_Buffers()
{
	ID3D11Buffer* pVertexBuffers[] = {
		m_pVB,
		m_pVBInstance,
	};

	_uint				iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceStride
	};

	_uint				iOffsets[] = {
		0,
		0
	};

	/* 정점버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);

	/* 인덱스버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	/* 그릴때 어떤 형태로 정점들을 이어 그릴건지. */
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);

	return S_OK;
}

HRESULT CVIBuffer_Instance::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	/* 인덱스버퍼의 값을 이용하여 정점 버퍼의 정점들을 그려낸다. */
	m_pContext->DrawIndexedInstanced(m_iIndexCountPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}

_float4 CVIBuffer_Instance::Compute_RandPosition(INSTANCE_SHAPE eShape, _bool bRespawnAtMinHeight)
{
	if (eShape == SHAPE_DEFAULT) {
		uniform_real_distribution<_float>	RangePosX(m_InstanceDesc.vCenter.x - m_InstanceDesc.vRange.x * 0.5f, m_InstanceDesc.vCenter.x + m_InstanceDesc.vRange.x * 0.5f);
		uniform_real_distribution<_float>	RangePosY(m_InstanceDesc.vCenter.y - m_InstanceDesc.vRange.y * 0.5f, m_InstanceDesc.vCenter.y + m_InstanceDesc.vRange.y * 0.5f);
		uniform_real_distribution<_float>	RangePosZ(m_InstanceDesc.vCenter.z - m_InstanceDesc.vRange.z * 0.5f, m_InstanceDesc.vCenter.z + m_InstanceDesc.vRange.z * 0.5f);
		
		if(bRespawnAtMinHeight)
			return _float4(RangePosX(m_RandomNumber), m_InstanceDesc.vCenter.y - m_InstanceDesc.vRange.y * 0.5f, RangePosZ(m_RandomNumber), 1.f);
		else
			return _float4(RangePosX(m_RandomNumber), RangePosY(m_RandomNumber), RangePosZ(m_RandomNumber), 1.f);
	}
	else if (eShape == SHAPE_HEMISPHERE) {
		uniform_real_distribution<_float> Angle(0, XM_2PI);
		uniform_real_distribution<_float> Radius(0, m_InstanceDesc.vRange.x);
		uniform_real_distribution<_float> RangePosZ(m_InstanceDesc.vCenter.z - m_InstanceDesc.vRange.z * 0.5f, m_InstanceDesc.vCenter.z + m_InstanceDesc.vRange.z * 0.5f);

		_float fRadius = Radius(m_RandomNumber);
		_float fAngle = Angle(m_RandomNumber);

		_float fX = fRadius * cos(fAngle);
		_float fY = abs(fRadius * sin(fAngle));

		_float fRandRadius = Radius(m_RandomNumber);
		_float fZSquared = fRandRadius * fRandRadius - fX * fX - fY * fY;
		_float fZ = sqrt(fZSquared);

		_float fDirection = Angle(m_RandomNumber); // 음수 양수 결정을 위한 변수
		if (fDirection > XM_PI)
			fZ *= -1;

		return _float4(fX, fY, fZ, 1);
	}
	else if (eShape == SHAPE_RING) {
		_float fOuterRadius = m_InstanceDesc.vRange.x;
		_float fInnerRadius = fOuterRadius * 0.8f;
		uniform_real_distribution<_float> Angle(0, XM_2PI);
		uniform_real_distribution<_float> Radius(fInnerRadius, fOuterRadius);
		uniform_real_distribution<_float> RangePosZ(m_InstanceDesc.vCenter.z - m_InstanceDesc.vRange.z * 0.5f, m_InstanceDesc.vCenter.z + m_InstanceDesc.vRange.z * 0.5f);

		_float fRadius = Radius(m_RandomNumber);
		_float fAngle = Angle(m_RandomNumber);

		return _float4(fRadius * cos(fAngle), fRadius * sin(fAngle), RangePosZ(m_RandomNumber), 1.f);
	}

	return _float4();
}

void CVIBuffer_Instance::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		pVertices[i].vPosition.y -= m_pSpeeds[i] * fTimeDelta;

		Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Spread(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);

	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector		vDir = XMVectorSetW(XMLoadFloat4(&pVertices[i].vPosition) - XMLoadFloat3(&m_InstanceDesc.vPivot), 0.f);

		XMStoreFloat4(&pVertices[i].vPosition,
			XMLoadFloat4(&pVertices[i].vPosition) + XMVector3Normalize(vDir) * m_pSpeeds[i] * fTimeDelta);

		Compute_LifeTime(pVertices, i, fTimeDelta);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Check_LifeTimes(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);
	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
		Compute_LifeTime(pVertices, i, fTimeDelta);

	m_pContext->Unmap(m_pVBInstance, 0);
}

void CVIBuffer_Instance::Compute_LifeTime(VTXMATRIX* pVertices, _uint iInstanceIndex, _float fTimeDelta, _bool bRespawnAtMinHeight)
{
	m_pLifeTimes[iInstanceIndex].x += fTimeDelta;

	if (m_pLifeTimes[iInstanceIndex].x > m_pLifeTimes[iInstanceIndex].y) {
		if (true == m_InstanceDesc.isLoop){
			m_pLifeTimes[iInstanceIndex].x = 0.f;
			pVertices[iInstanceIndex].vPosition = Compute_RandPosition(m_InstanceDesc.eShape, bRespawnAtMinHeight);
		}
	}

	if (bRespawnAtMinHeight)
	{
		if (((m_InstanceDesc.vCenter.x - m_InstanceDesc.vRange.x * 0.5f) > pVertices[iInstanceIndex].vPosition.x)
			|| ((m_InstanceDesc.vCenter.x + m_InstanceDesc.vRange.x * 0.5f) < pVertices[iInstanceIndex].vPosition.x)
			|| ((m_InstanceDesc.vCenter.z - m_InstanceDesc.vRange.z * 0.5f) > pVertices[iInstanceIndex].vPosition.z)
			|| ((m_InstanceDesc.vCenter.z + m_InstanceDesc.vRange.z * 0.5f) < pVertices[iInstanceIndex].vPosition.z)) 
		{
			if (true == m_InstanceDesc.isLoop) {
				m_pLifeTimes[iInstanceIndex].x = 0.f;
				pVertices[iInstanceIndex].vPosition = Compute_RandPosition(m_InstanceDesc.eShape, bRespawnAtMinHeight);
			}
		}
	}

	pVertices[iInstanceIndex].fLifeRatio = m_pLifeTimes[iInstanceIndex].x / m_pLifeTimes[iInstanceIndex].y;
}

void CVIBuffer_Instance::Free()
{
	__super::Free();

	if (false == m_isCloned) {
		Safe_Delete_Array(m_pSpeeds);
		Safe_Delete_Array(m_pInstanceVertices);
	}

	Safe_Delete_Array(m_pLifeTimes);
	Safe_Release(m_pVBInstance);
}
