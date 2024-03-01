#include "Mesh.h"
#include "Bone.h"
#include <fstream>

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer(pDevice, pContext)
{
}

CMesh::CMesh(const CMesh& rhs)
	: CVIBuffer(rhs), m_iFaces{rhs.m_iFaces}
{
}

HRESULT CMesh::Initialize_Prototype(CModel::TYPE eModelType, string strDirectory, streampos filePtPos, const vector<CBone*>& Bones, _fmatrix TransformMatrix)
{
	m_strDirectory = strDirectory;
	ifstream inputFile(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!inputFile.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}
	inputFile.seekg(filePtPos); // 여기서부터 읽기 시작
	inputFile.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	inputFile.read(reinterpret_cast<char*>(&m_iMaterialIndex), sizeof(m_iMaterialIndex));
	inputFile.read(reinterpret_cast<char*>(&m_iNumVertices), sizeof(m_iNumVertices));
	inputFile.read(reinterpret_cast<char*>(&m_iFaces), sizeof(m_iFaces));
	m_filePointerPos = inputFile.tellg();
	inputFile.close();

	m_iNumIndices = m_iFaces * 3;
	m_iIndexStride = sizeof(_uint);
	m_iNumVertexBuffers = 1;
	m_eIndexFormat = DXGI_FORMAT_R32_UINT;
	m_ePrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

#pragma region VERTEX_BUFFER

	HRESULT hr = CModel::TYPE_NONANIM == eModelType ? Ready_Vertices_For_NonAnimModel(TransformMatrix) : Ready_Vertices_For_AnimModel(Bones);
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


	ifstream inputFile2(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!inputFile2.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}
	inputFile2.seekg(m_filePointerPos); // 여기서부터 읽기 시작


	_uint* pIndices = new _uint[m_iNumIndices];
	ZeroMemory(pIndices, sizeof(_uint) * m_iNumIndices);


	for (size_t i = 0; i < m_iNumIndices; i++)
	{
		inputFile2.read(reinterpret_cast<char*>(&pIndices[i]), sizeof(_uint));
	}
	m_filePointerPos = inputFile2.tellg();
	inputFile2.close();

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

	
	ifstream inputFile(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!inputFile.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}
	inputFile.seekg(m_filePointerPos); // 여기서부터 읽기 시작


	VTXMESH* pVertices = new VTXMESH[m_iNumVertices];
	ZeroMemory(pVertices, sizeof(VTXMESH) * m_iNumVertices);
	for (size_t i = 0; i < m_iNumVertices; i++)
	{
		inputFile.read(reinterpret_cast<char*>(&pVertices[i]), sizeof(VTXMESH));
	}
	m_filePointerPos = inputFile.tellg();
	inputFile.close();


	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = pVertices;

	if (FAILED(__super::Create_Buffer(&m_pVB)))
		return E_FAIL;

	Safe_Delete_Array(pVertices);

	return S_OK;
}

HRESULT CMesh::Ready_Vertices_For_AnimModel(const vector<CBone*>& Bones)
{
	//m_iVertexStride = sizeof(VTXANIMMESH);

	//ZeroMemory(&m_BufferDesc, sizeof m_BufferDesc);

	///* 정점버퍼의 byte크기 */
	//m_BufferDesc.ByteWidth = m_iVertexStride * m_iNumVertices;
	//m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	//m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//m_BufferDesc.CPUAccessFlags = 0;
	//m_BufferDesc.MiscFlags = 0;
	//m_BufferDesc.StructureByteStride = m_iVertexStride;

	//VTXANIMMESH* pVertices = new VTXANIMMESH[m_iNumVertices];
	//ZeroMemory(pVertices, sizeof(VTXANIMMESH) * m_iNumVertices);

	//for (size_t i = 0; i < m_iNumVertices; i++)
	//{
	//	memcpy(&pVertices[i].vPosition, &pAIMesh->mVertices[i], sizeof(_float3));
	//	memcpy(&pVertices[i].vNormal, &pAIMesh->mNormals[i], sizeof(_float3));
	//	memcpy(&pVertices[i].vTexcoord, &pAIMesh->mTextureCoords[0][i], sizeof(_float2));
	//	memcpy(&pVertices[i].vTangent, &pAIMesh->mTangents[i], sizeof(_float3));
	//}

	//m_iNumBones = pAIMesh->mNumBones;

	//for (size_t i = 0; i < m_iNumBones; i++)
	//{
	//	aiBone* pAIBone = pAIMesh->mBones[i];

	//	_int	iBoneIndex = { -1 };

	//	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
	//		{
	//			++iBoneIndex;
	//			return pBone->Compare_Name(pAIBone->mName.data);
	//		});

	//	m_Bones.push_back(iBoneIndex);

	//	/* 이 뼈는 몇개의 정점들에게 영향을 준다. */
	//	_uint		iNumWeights = pAIBone->mNumWeights;

	//	for (size_t j = 0; j < iNumWeights; j++)
	//	{
	//		if (0.0f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x)
	//		{
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.x = i;
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.x = pAIBone->mWeights[j].mWeight;
	//		}

	//		else if (0.0f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y)
	//		{
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.y = i;
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.y = pAIBone->mWeights[j].mWeight;
	//		}

	//		else if (0.0f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z)
	//		{
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.z = i;
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.z = pAIBone->mWeights[j].mWeight;
	//		}

	//		else if (0.0f == pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w)
	//		{
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendIndices.w = i;
	//			pVertices[pAIBone->mWeights[j].mVertexId].vBlendWeights.w = pAIBone->mWeights[j].mWeight;
	//		}
	//	}
	//}

	//ZeroMemory(&m_InitialData, sizeof m_InitialData);
	//m_InitialData.pSysMem = pVertices;

	//if (FAILED(__super::Create_Buffer(&m_pVB)))
	//	return E_FAIL;

	//Safe_Delete_Array(pVertices);

	return S_OK;
}

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CModel::TYPE eModelType, string strDirectory, streampos filePtPos, const vector<CBone*>& Bones, _fmatrix TransformMatrix)
{
	CMesh* pInstance = new CMesh(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, strDirectory, filePtPos, Bones, TransformMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CMesh"));

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
}
