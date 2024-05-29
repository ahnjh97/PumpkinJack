#include "Mesh.h"
#include "Bone.h"
#include <fstream>
#include "GameInstance.h"
#include "QuadTreeCell.h"
#include "Cell.h"

CMesh::CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileStream)
	: CVIBuffer(pDevice, pContext), m_InputFile(fileStream)
{
}

CMesh::CMesh(const CMesh& rhs)
	: CVIBuffer(rhs), m_iFaces{rhs.m_iFaces}, m_InputFile(ifstream())
{
}

HRESULT CMesh::Initialize_Prototype(TYPE eModelType, string strDirectory, const vector<CBone*>& Bones, _fmatrix TransformMatrix, _bool bOctreeCulling)
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

	if(true == bOctreeCulling)
		m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	else
		m_BufferDesc.Usage = D3D11_USAGE_DEFAULT;

	m_BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	if (true == bOctreeCulling)
		m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	else
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

			_matrix pWorldMatrix = pTransform->Get_WorldMatrix();
			vPickPos = XMVector3TransformCoord(vPickPos, pWorldMatrix);
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
	for (auto& iter : vecPickPos) // 카메라와 가장 거리가 가까운 위치 계산 
	{
		if (iter.w < fShortest) {
			fShortest = iter.w;
			fResult = iter;
		}
	}
	
	return fResult;
}

//vector<vector<CCell*>> CMesh::SetUp_MeshQuadGroundCells()
//{
//	vector<vector<CCell*>> vectorQuadGroundCells; // 1,2,3,4분면의 셀들을 가지고 있는 벡터들을 가지고 있는 벡터
//	unordered_set<CCell::QUAD> quadSet; // 어떤 몇 사분면의 벡터에 추가할지
//	vectorQuadGroundCells.resize(CCell::QUAD_END);
//
//	_uint		iNumIndices = { 0 };
//	for (size_t i = 0; i < m_iFaces; i++)
//	{
//		_vector p0 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
//		_vector p1 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
//		_vector p2 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
//
//		_vector v1 = XMVectorSubtract(p1, p0);
//		_vector v2 = XMVectorSubtract(p2, p0);
//
//		_vector vNormal = XMVector3Cross(v1, v2);
//		vNormal = XMVector3Normalize(vNormal);
//
//		_float fDotProduct = XMVectorGetX(XMVector3Dot(vNormal, XMVectorSet(0, 1, 0, 0)));
//
//		if (fDotProduct > 0.1f) {
//			quadSet.insert((CCell::QUAD)Check_Quad(p0));
//			quadSet.insert((CCell::QUAD)Check_Quad(p1));
//			quadSet.insert((CCell::QUAD)Check_Quad(p2));
//
//			for (auto& quad : quadSet) {
//				CCell* pCell = CCell::Create(m_pDevice, m_pContext, p0, p1, p2);
//				vectorQuadGroundCells[quad].emplace_back(pCell);
//			}		
//		}
//	}
//
//	return vectorQuadGroundCells;
//}

//_int CMesh::Check_Quad(_fvector vPoint)
//{
//	if (XMVectorGetX(vPoint) >= 0) {
//		if (XMVectorGetZ(vPoint) >= 0) 
//			return CCell::RT;		
//		else
//			return CCell::RB;
//	}
//	else {
//		if (XMVectorGetZ(vPoint) <= 0)
//			return CCell::LB;
//		else
//			return CCell::LT;
//	}
//}

vector<FACE> CMesh::SetUp_QuadTreeCells(_float& _fMinX, _float& _fMinZ, _float& _fMaxX, _float& _fMaxZ)
{
	vector<FACE> vecMeshFaces;

	_uint		iNumIndices = { 0 };
	for (size_t i = 0; i < m_iFaces; i++)
	{
		_vector p0 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
		_vector p1 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);
		_vector p2 = ::XMLoadFloat3(&m_pVerticesPos[m_pIndices[iNumIndices++]]);

		_vector v1 = XMVectorSubtract(p1, p0);
		_vector v2 = XMVectorSubtract(p2, p0);

		_vector vNormal = XMVector3Cross(v1, v2);
		vNormal = XMVector3Normalize(vNormal);

		_float fDotProduct = XMVectorGetX(XMVector3Dot(vNormal, XMVectorSet(0, 1, 0, 0)));

		if (fDotProduct > -0.1f) {
			for (_int j = 0; j < 3; j++) {
				_int iIndex = m_pIndices[iNumIndices - 3 + j]; // 현재 인덱스
				_fMinX = min(_fMinX, m_pVerticesPos[iIndex].x);
				_fMinZ = min(_fMinZ, m_pVerticesPos[iIndex].z);
				_fMaxX = max(_fMaxX, m_pVerticesPos[iIndex].x);
				_fMaxZ = max(_fMaxZ, m_pVerticesPos[iIndex].z);
			}

			FACE tFace{};
			XMStoreFloat3(&tFace.vA, p0);
			XMStoreFloat3(&tFace.vB, p1);
			XMStoreFloat3(&tFace.vC, p2);
			vecMeshFaces.emplace_back(tFace);
		}
	}

	return vecMeshFaces;
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

CMesh* CMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eModelType, string strDirectory, ifstream& fileStream, const vector<CBone*>& Bones, _fmatrix TransformMatrix, _bool bOctreeCulling)
{
	CMesh* pInstance = new CMesh(pDevice, pContext, fileStream);

	if (FAILED(pInstance->Initialize_Prototype(eModelType, strDirectory, Bones, TransformMatrix, bOctreeCulling)))
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
