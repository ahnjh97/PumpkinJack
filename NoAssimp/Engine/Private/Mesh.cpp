#include "Mesh.h"
#include "Bone.h"
#include <fstream>
#include "GameInstance.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileStream)
	: CVIBuffer(pDevice, pContext), m_InputFile(fileStream)
{
}

CMesh::CMesh(const CMesh& rhs)
	: CVIBuffer(rhs), m_iFaces{rhs.m_iFaces}, m_InputFile(ifstream())
{
}

HRESULT CMesh::Initialize_Prototype(TYPE eModelType, string strDirectory, const vector<CBone*>& Bones, _fmatrix TransformMatrix)
{
	m_strDirectory = strDirectory;
	if (!m_InputFile.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}

	m_InputFile.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	m_InputFile.read(reinterpret_cast<char*>(&m_iMaterialIndex), sizeof(m_iMaterialIndex));
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumVertices), sizeof(m_iNumVertices));
	m_InputFile.read(reinterpret_cast<char*>(&m_iFaces), sizeof(m_iFaces));

	m_pVerticesPos = new _float3[m_iNumVertices];

	m_iNumIndices = m_iFaces * 3;
	m_iIndexStride = sizeof(_uint);
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER

	HRESULT hr = TYPE_NONANIM == eModelType ? Ready_Vertices_For_NonAnimModel(TransformMatrix) : Ready_Vertices_For_AnimModel(Bones);
	if (FAILED(hr))
		return E_FAIL;

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


	_uint* pIndices = new _uint[m_iNumIndices];
	m_pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);


	for (size_t i = 0; i < m_iNumIndices; i++)
	{
		m_InputFile.read(reinterpret_cast<char*>(&pIndices[i]), sizeof(_uint));
		m_pIndices[i] = pIndices[i];
	}

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pIndices;

	if (FAILED(__super::Create_Buffer(&m_pIB)))
		return E_FAIL;

	Safe_Delete_Array(pIndices);

#pragma endregion

	return S_OK;
}

HRESULT CMesh::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CMesh::Stock_Matrices(const vector<CBone*>& Bones, _float4x4* pMeshBoneMatrices)
{
	for (_uint i = 0; i < m_iNumBones; ++i)
	{
		XMStoreFloat4x4(&pMeshBoneMatrices[i], XMLoadFloat4x4(&m_OffsetMatrices[i]) * XMLoadFloat4x4(Bones[m_Bones[i]]->Get_CombinedTransformationMatrix()));
	}

	return S_OK;
}

_float4 CMesh::Get_PickPos(const CTransform* pTransform) const
{
	vector<_float4> vecPickPos;

	_float3 vRayDir, vRayPos;
	m_pGameInstance->Transform_PickingToLocalSpace(pTransform, &vRayDir, &vRayPos);
	_vector vPickPos;
	_float fDis;

	_uint		iNumIndices = { 0 };
	for (size_t i = 0; i < m_iFaces; i++)
	{
		_vector v0 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
		_vector v1 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
		_vector v2 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);

		_vector vecRayDir = ::XMVector3Normalize(::XMLoadFloat3(&vRayDir));
		_vector vecRayPos = ::XMLoadFloat3(&vRayPos);
		if (::TriangleTests::Intersects(vecRayPos, vecRayDir, v0, v1, v2, fDis))
		{
			vPickPos = vecRayPos + vecRayDir * fDis;

			_matrix* pWorldMatrix = &pTransform->Get_WorldMatrix();
			vPickPos = XMVector3TransformCoord(vPickPos, *pWorldMatrix);
			_float4 tempPos;
			::XMStoreFloat4(&tempPos, vPickPos);
			tempPos.w = fDis;
			if (tempPos.w != 0)
				vecPickPos.push_back(tempPos);
		}
	}

	if (vecPickPos.empty())
		return _float4();

	_float fShortest = { FLT_MAX };
	_float4 fResult = {};
	for (auto& iter : vecPickPos) // 카메라와 가장 거리가 가까운놈 계산 
	{
		if (iter.w <= fShortest)
			fResult = iter;
	}
	
	return fResult;
}

HRESULT CMesh::Ready_Vertices_For_NonAnimModel(_fmatrix TransformMatrix)
{
	m_iVertexStride = sizeof(VTXMESH);

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
	{
		m_InputFile.read(reinterpret_cast<char*>(&pVertices[i]), sizeof(VTXMESH));
		XMStoreFloat3(&pVertices[i].vPosition, XMVector3TransformCoord(XMLoadFloat3(&pVertices[i].vPosition), TransformMatrix));
		XMStoreFloat3(&pVertices[i].vNormal, XMVector3TransformNormal(XMLoadFloat3(&pVertices[i].vNormal), TransformMatrix));
		m_pVerticesPos[i] = pVertices[i].vPosition;
	}

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMesh::Ready_Vertices_For_AnimModel(const vector<CBone*>& Bones)
{
	m_iVertexStride = sizeof(VTXANIMMESH);
	ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	/* 정점버퍼의 byte크기 */
	m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = 0;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iVertexStride;

	VTXANIMMESH* pVertices = new VTXANIMMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);


	m_InputFile.read(reinterpret_cast<char*>(&m_iNumBones), sizeof(m_iNumBones));
	m_OffsetMatrices.reserve(m_iNumBones);

	_float4x4 offsetMat = {};
	_char szBoneName[MAX_PATH] = {};
	for (_uint i = 0; i < m_iNumBones; i++)
	{
		m_InputFile.read(reinterpret_cast<char*>(&szBoneName), sizeof(szBoneName));
		m_InputFile.read(reinterpret_cast<char*>(&offsetMat), sizeof(offsetMat));
		m_OffsetMatrices.push_back(offsetMat);

		_int	iBoneIndex = { -1 };
		auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool{
				++iBoneIndex;
				return pBone->Compare_Name(szBoneName);});

		m_Bones.push_back(iBoneIndex);
	}

	if (0 == m_iNumBones)
	{
		m_iNumBones = 1;
		_int	iBoneIndex = { -1 };
		auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool	{
				++iBoneIndex; 
				return pBone->Compare_Name(m_szName);	});

		m_Bones.push_back(iBoneIndex);

		_float4x4		OffsetMatrix;
		XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
		m_OffsetMatrices.push_back(OffsetMatrix);
	}

	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		m_InputFile.read(reinterpret_cast<char*>(&pVertices[i]), sizeof(VTXANIMMESH));
	}

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);
	return S_OK;
}

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eModelType, string strDirectory, ifstream& fileStream, const vector<CBone*>& Bones, _fmatrix TransformMatrix)
{
	CMesh* pInstance = new CMesh(pDevice, pContext, fileStream);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, strDirectory, Bones, TransformMatrix)))
	{
		MSG_BOX(TEXT("Failed To Create : CMesh"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CMesh* CMesh::Clone(void* pArg)
{
	return nullptr;
}

void CMesh::Free()
{

	__super::Free();	
	
	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pIndices);
		Safe_Delete_Array(m_pVerticesPos);
	}
}
