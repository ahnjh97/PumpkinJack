#include "VIBuffer_Instance_Mesh.h"
#include "Transform.h"

CVIBuffer_Instance_Mesh::CVIBuffer_Instance_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileStream)
	: CVIBuffer_Instance{ pDevice, pContext }, m_InputFile(fileStream)
{
}

CVIBuffer_Instance_Mesh::CVIBuffer_Instance_Mesh(const CVIBuffer_Instance_Mesh& rhs)
	: CVIBuffer_Instance{ rhs }, m_iFaces{ rhs.m_iFaces }, m_InputFile(ifstream())
{
}

HRESULT CVIBuffer_Instance_Mesh::Initialize_Prototype(ifstream& fileStream, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	if (!m_InputFile.is_open()) {
		MB("Error: CVIBuffer_Instance_Mesh FileStream");
		return E_FAIL;
	}

	m_InputFile.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	m_InputFile.read(reinterpret_cast<char*>(&m_iMaterialIndex), sizeof(m_iMaterialIndex));
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumVertices), sizeof(m_iNumVertices));
 	m_InputFile.read(reinterpret_cast<char*>(&m_iFaces), sizeof(m_iFaces));

	if (FAILED(__super::Initialize_Prototype(InstanceDesc)))
		return E_FAIL;

	m_iNumInstance = InstanceDesc.iNumInstance;
	m_iInstanceStride = sizeof(VTXMATRIX);
	m_iIndexCountPerInstance = m_iFaces * 3;

	m_iVertexStride = sizeof(VTXMESH);

	m_iNumIndices = m_iIndexCountPerInstance * m_iNumInstance;
	m_iIndexStride = sizeof(_uint);
	m_iNumVertexBuffers = 2;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXMESH* pVertices = new VTXMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_iNumVertices);

	for (size_t i = 0; i < m_iNumVertices; i++)	
		m_InputFile.read(reinterpret_cast<char*>(&pVertices[i]), sizeof(VTXMESH));

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

	_uint* pOriginalIndices = new _uint[m_iIndexCountPerInstance];
	ZeroMemory(pOriginalIndices, sizeof(_uint) * m_iIndexCountPerInstance);

	for (size_t i = 0; i < m_iIndexCountPerInstance; i++)
		m_InputFile.read(reinterpret_cast<char*>(&pOriginalIndices[i]), sizeof(_uint));


	_uint* pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);

	for (size_t j = 0; j < m_iNumInstance; j++) {
		for (size_t k = 0; k < m_iIndexCountPerInstance; k++)
 			pIndices[k + j * m_iIndexCountPerInstance] = pOriginalIndices[k];
	}
		
	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pOriginalIndices);
	Safe_Delete_Array(pIndices);

#pragma endregion

#pragma region INSTANCE_BUFFER

	ZeroMemory(&m_InstanceBufferDesc, sizeof m_InstanceBufferDesc);

	uniform_real_distribution<_float>	ScaleX(InstanceDesc.vMinScale.x, InstanceDesc.vMaxScale.x);
	uniform_real_distribution<_float>	ScaleY(InstanceDesc.vMinScale.y, InstanceDesc.vMaxScale.y);
	uniform_real_distribution<_float>	ScaleZ(InstanceDesc.vMinScale.z, InstanceDesc.vMaxScale.z);

	uniform_real_distribution<_float>	LifeTime(InstanceDesc.vLifeTime.x, InstanceDesc.vLifeTime.y);
	uniform_real_distribution<_float>	Speed(InstanceDesc.vSpeed.x, InstanceDesc.vSpeed.y);
	uniform_real_distribution<_float>	RotationAxis(-1, 1);

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

	m_pRotationAxes = new _float3[m_iNumInstance];
	ZeroMemory(m_pRotationAxes, sizeof(_float3) * m_iNumInstance);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_float fScale = ScaleX(m_RandomNumber);
		XMStoreFloat4(&m_pInstanceVertices[i].vRight, XMVectorSet(1.f, 0.f, 0.f, 0.f) * fScale);
		XMStoreFloat4(&m_pInstanceVertices[i].vUp, XMVectorSet(0.f, 1.f, 0.f, 0.f) * fScale);
		XMStoreFloat4(&m_pInstanceVertices[i].vLook, XMVectorSet(0.f, 0.f, 1.f, 0.f) * fScale);
		m_pInstanceVertices[i].vPosition = Compute_RandPosition(InstanceDesc.eShape);

		m_pInstanceVertices[i].fLifeRatio = 0;


		/* 0.f 현재 라이프타임 */
		/* LifeTime(m_RandomNumber) 인스턴스마다 랜덤하게 설정된 각각의 라이프타임디다. */
		m_pLifeTimes[i] = _float2(0.f, LifeTime(m_RandomNumber));
		m_pSpeeds[i] = Speed(m_RandomNumber);
		m_pRotationAxes[i] = _float3(RotationAxis(m_RandomNumber), RotationAxis(m_RandomNumber), RotationAxis(m_RandomNumber));
	}

	ZeroMemory(&m_InstanceSubResourceData, sizeof m_InstanceSubResourceData);
	m_InstanceSubResourceData.pSysMem = m_pInstanceVertices;

#pragma endregion

	return S_OK;
}

HRESULT CVIBuffer_Instance_Mesh::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CVIBuffer_Instance_Mesh::TurnAndSpread(_float fTimeDelta, _float fRotationSpeed)
{
	D3D11_MAPPED_SUBRESOURCE		SubResource{};
	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &SubResource);
	VTXMATRIX* pVertices = ((VTXMATRIX*)SubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// 자전
		_vector vRotationAxis = XMVector3Normalize(XMLoadFloat3(&m_pRotationAxes[i]));
		_matrix	RotationMatrix = XMMatrixRotationAxis(vRotationAxis, m_pSpeeds[i] * fTimeDelta * fRotationSpeed);

		XMStoreFloat4(&pVertices[i].vRight, XMVector4Transform(XMLoadFloat4(&pVertices[i].vRight), RotationMatrix));
		XMStoreFloat4(&pVertices[i].vUp, XMVector4Transform(XMLoadFloat4(&pVertices[i].vUp), RotationMatrix));
		XMStoreFloat4(&pVertices[i].vLook, XMVector4Transform(XMLoadFloat4(&pVertices[i].vLook), RotationMatrix));
			
		// 이동
		_vector		vDir = XMVectorSetW(XMLoadFloat4(&pVertices[i].vPosition) - XMLoadFloat3(&m_InstanceDesc.vPivot), 0.f);

		XMStoreFloat4(&pVertices[i].vPosition,
			XMLoadFloat4(&pVertices[i].vPosition) + XMVector3Normalize(vDir) * m_pSpeeds[i] * fTimeDelta);

		Compute_LifeTime(pVertices, i, fTimeDelta, true);
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

CVIBuffer_Instance_Mesh* CVIBuffer_Instance_Mesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileStream, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	CVIBuffer_Instance_Mesh* pInstance = new CVIBuffer_Instance_Mesh(pDevice, pContext, fileStream);

	if (FAILED(pInstance->Initialize_Prototype(fileStream, InstanceDesc)))
	{
		MSG_BOX(TEXT("Failed To Create : CVIBuffer_Instance_Mesh"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CComponent* CVIBuffer_Instance_Mesh::Clone(void* pArg)
{
	return nullptr;
}

void CVIBuffer_Instance_Mesh::Free()
{
	__super::Free();

	Safe_Delete_Array(m_pRotationAxes);
}
