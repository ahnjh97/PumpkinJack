#include "Model.h"
#include "Bone.h"
#include "Mesh.h"

#include "Shader.h"
#include "Texture.h"
#include <fstream>

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CModel::CModel(const CModel& rhs)
	: CComponent{ rhs }
	, m_iNumMeshes{ rhs.m_iNumMeshes }
	, m_Meshes{ rhs.m_Meshes }
	, m_iNumMaterials{ rhs.m_iNumMaterials }
	, m_Materials{ rhs.m_Materials }
	, m_TransformMatrix{ rhs.m_TransformMatrix }
	, m_Bones{ rhs.m_Bones }
{
	for (auto& pBone : m_Bones)
		Safe_AddRef(pBone);

	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& Material : m_Materials)
	{
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_AddRef(Material.MaterialTextures[i]);
	}
}

HRESULT CModel::Initialize_Prototype(TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix)
{
	m_eModelType = eType;
	size_t lastSlashPos = strModelFilePath.find_last_of("/");
	size_t secondLastSlashPos = strModelFilePath.find_last_of("/", lastSlashPos - 1);

	if (secondLastSlashPos != string::npos) 
		m_strDirectory = "../../../model_txt/" +  strModelFilePath.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1) + ".txt";

	XMStoreFloat4x4(&m_TransformMatrix, TransformMatrix);

	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	if (eType == TYPE_ANIM)
	{
		/*if (FAILED(Ready_Bones(m_pAIScene->mRootNode)))
			return E_FAIL;*/
	}

	/* 모델을 구성하는 메시들을 생성한다. */
	/* 모델 = 메시 + 메시 + ... */
	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	if (FAILED(Ready_Materials(strModelFilePath.c_str())))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint eTextureType)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	_uint		iMeshMaterialIndex = { m_Meshes[iMeshIndex]->Get_MaterialIndex() };

	if (iMeshMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	m_Materials[iMeshMaterialIndex].MaterialTextures[eTextureType]->Bind_ShaderResource(pShader, pConstantName);

	return S_OK;
}

HRESULT CModel::Play_Animation(_float fTimeDelta)
{
	//for (auto& pBone : m_Bones)
	//	pBone->Invalidate_CombinedTransformationMatrix(m_Bones);

	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	m_Meshes[iMeshIndex]->Bind_Buffers();
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CModel::Ready_Meshes()
{
	ifstream inputFile(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!inputFile.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}
	inputFile.read(reinterpret_cast<char*>(&m_iNumMeshes), sizeof(m_iNumMeshes));
	m_filePointerPos = inputFile.tellg();
	inputFile.close();

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eModelType, m_strDirectory, m_filePointerPos, m_Bones, XMLoadFloat4x4(&m_TransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
		m_filePointerPos = pMesh->Get_IncreasedFilePtPos(); // 파일포인터 위치 갱신
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath)
{
	ifstream inputFile(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!inputFile.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}
	inputFile.seekg(m_filePointerPos); // 여기서부터 읽기 시작
	inputFile.read(reinterpret_cast<char*>(&m_iNumMaterials), sizeof(m_iNumMaterials));

	while (true)
	{
		_uint iTemp;
		_uint jTemp;
		MATERIAL_TEXTURE			MeshMaterial{};
		_char			szFullPath[MAX_PATH] = { "" };
		_tchar			szPerfectPath[MAX_PATH] = { L"" };
		inputFile.read(reinterpret_cast<char*>(&iTemp), sizeof(iTemp));
		if (iTemp == m_iNumMaterials)
			break;
		inputFile.read(reinterpret_cast<char*>(&jTemp), sizeof(jTemp));
		inputFile.read(reinterpret_cast<char*>(&szFullPath), sizeof(szFullPath));
		
		MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szPerfectPath, MAX_PATH);
		MeshMaterial.MaterialTextures[jTemp] = CTexture::Create(m_pDevice, m_pContext, szPerfectPath);
		if (nullptr == MeshMaterial.MaterialTextures[jTemp])
			return E_FAIL;

		if(m_Materials.size() <= iTemp) // 새로운 배열 삽입
			m_Materials.push_back(MeshMaterial);
		else // 벡터 안에있는 배열의 원소 수정
			m_Materials[iTemp].MaterialTextures[jTemp] = MeshMaterial.MaterialTextures[jTemp];
	}
	m_filePointerPos = inputFile.tellg();
	inputFile.close();
	return S_OK;
}

//HRESULT CModel::Ready_Bones(aiNode* pAINode, _int iParentIndex)
//{
//	CBone* pBone = CBone::Create(pAINode, iParentIndex);
//	if (nullptr == pBone)
//		return E_FAIL;
//
//	m_Bones.push_back(pBone);
//
//	_int		iParent = m_Bones.size() - 1;
//
//	for (size_t i = 0; i < pAINode->mNumChildren; i++)
//	{
//		Ready_Bones(pAINode->mChildren[i], iParent);
//	}
//
//	return S_OK;
//}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, strModelFilePath, TransformMatrix)))
	{
		MSG_BOX(TEXT("Failed To Created : CModel"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CModel"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& Material : m_Materials)
	{
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_Release(Material.MaterialTextures[i]);
	}

	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();
}
