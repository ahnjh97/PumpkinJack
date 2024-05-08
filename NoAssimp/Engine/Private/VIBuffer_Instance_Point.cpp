#include "VIBuffer_Instance_Point.h"

CVIBuffer_Instance_Point::CVIBuffer_Instance_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer_Instance{ pDevice, pContext }
{
}

CVIBuffer_Instance_Point::CVIBuffer_Instance_Point(const CVIBuffer_Instance_Point& rhs)
	: CVIBuffer_Instance{ rhs }
{
}

HRESULT CVIBuffer_Instance_Point::Initialize_Prototype(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	if (FAILED(__super::Initialize_Prototype(InstanceDesc)))
		return E_FAIL;

	m_iNumInstance = InstanceDesc.iNumInstance;
	m_iInstanceStride = sizeof(VTXMATRIX);
	m_iIndexCountPerInstance = 1;

	m_iNumVertices = 1;
	m_iVertexStride = sizeof(VTXPOS);

	m_iNumIndices = m_iIndexCountPerInstance * m_iNumInstance;
	m_iIndexStride = sizeof(_ushort);
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R16_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXPOS* pVertices = new VTXPOS[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXPOS) * m_iNumVertices);

	pVertices[0].vPosition = _float3(0.0f, 0.0f, 0.f);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
#pragma endregion


#pragma region INDEX_BUFFER

	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 인덱스 버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iIndexStride * m_iNumIndices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = 0;


	_ushort* pIndices = new _ushort[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_ushort) * m_iNumIndices);

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

#pragma region INSTANCE_BUFFER

	ZeroMemory(&m_InstanceBufferDesc, sizeof m_InstanceBufferDesc);

	uniform_real_distribution<_float>	ScaleX(InstanceDesc.vMinScale.x, InstanceDesc.vMaxScale.x);
	uniform_real_distribution<_float>	ScaleY(InstanceDesc.vMinScale.y, InstanceDesc.vMaxScale.y);
	uniform_real_distribution<_float>	ScaleZ(InstanceDesc.vMinScale.z, InstanceDesc.vMaxScale.z);

	uniform_real_distribution<_float>	LifeTime(InstanceDesc.vLifeTime.x, InstanceDesc.vLifeTime.y);
	uniform_real_distribution<_float>	Speed(InstanceDesc.vSpeed.x, InstanceDesc.vSpeed.y);
	uniform_int_distribution<_uint>		SpriteIndexOffset(0, InstanceDesc.iSpriteIndexOffset);


	/* 인덱스 버퍼의 byte크기 */
	m_InstanceBufferDesc.ByteWidth = m_iInstanceStride * m_iNumInstance;
	m_InstanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_InstanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_InstanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_InstanceBufferDesc.MiscFlags = 0;
	m_InstanceBufferDesc.StructureByteStride = m_iInstanceStride;

	m_pInstanceVertices = new VTXMATRIX[m_iNumInstance];
	ZeroMemory(m_pInstanceVertices, sizeof(VTXMATRIX) * m_iNumInstance);

	m_pLifeTimes = new _float2[m_iNumInstance];
	ZeroMemory(m_pLifeTimes, sizeof(_float2) * m_iNumInstance);

	m_pSpeeds = new _float[m_iNumInstance];
	ZeroMemory(m_pSpeeds, sizeof(_float) * m_iNumInstance);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		XMStoreFloat4(&m_pInstanceVertices[i].vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f) * ScaleX(m_RandomNumber));
		XMStoreFloat4(&m_pInstanceVertices[i].vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f) * ScaleY(m_RandomNumber));
		XMStoreFloat4(&m_pInstanceVertices[i].vLook, XMVectorSet(0.f, 0.f, 1.f, 0.f) * ScaleZ(m_RandomNumber));
		m_pInstanceVertices[i].vPosition = Compute_RandPosition(InstanceDesc.eShape);
		m_pInstanceVertices[i].fLifeRatio = 0;

		/* 0.f 현재 라이프타임 */
		/* LifeTime(m_RandomNumber) 인스턴스마다 랜덤하게 설정된 각각의 라이프타임디다. */
		m_pLifeTimes[i] = _float2(0.f, LifeTime(m_RandomNumber));
		m_pSpeeds[i] = Speed(m_RandomNumber);
	}

	if (InstanceDesc.iSpriteIndexOffset != 0) {
		for (size_t j = 0; j < m_iNumInstance; j++)
			m_pInstanceVertices[j].iSpriteOffset = SpriteIndexOffset(m_RandomNumber);
	}

	ZeroMemory(&m_InstanceSubResourceData, sizeof m_InstanceSubResourceData);
	m_InstanceSubResourceData.pSysMem = m_pInstanceVertices;

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Instance_Point::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

CVIBuffer_Instance_Point* CVIBuffer_Instance_Point::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	CVIBuffer_Instance_Point* pInstance = new CVIBuffer_Instance_Point(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(InstanceDesc)))
	{
		MSG_BOX(TEXT("Failed To Create : CVIBuffer_Instance_Point"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CVIBuffer_Instance_Point::Clone(void* pArg)
{
	CComponent* pInstance = new CVIBuffer_Instance_Point(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CVIBuffer_Instance_Point"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVIBuffer_Instance_Point::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pSpriteIndexOffsets);
}
