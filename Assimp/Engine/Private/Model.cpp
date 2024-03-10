#include "Model.h"
#include "Bone.h"
#include "Mesh.h"
#include "GameInstance.h"
#include "Shader.h"
#include "Texture.h"
#include <fstream>
#include <cstdio>
#include <locale>
#include <codecvt>

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
	, m_iNumAnimations{ rhs.m_iNumAnimations }
	, m_eModelType{rhs.m_eModelType}
{
	for (auto& pPrototypeAnimation : rhs.m_Animations)
		m_Animations.push_back(pPrototypeAnimation->Clone());

	for (auto& pPrototypeBone : rhs.m_Bones)
		m_Bones.push_back(pPrototypeBone->Clone());

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

	_uint		iOption = { aiProcessPreset_TargetRealtime_Fast | aiProcess_ConvertToLeftHanded };

	iOption = eType == TYPE_NONANIM ? iOption | aiProcess_PreTransformVertices : iOption;

	/* 파일ㄹ의 정보를 읽어서 aiScene안에 모든 데이터를 담아주게된다. */
	m_pAIScene = m_Importer.ReadFile(strModelFilePath.c_str(), iOption);
	if (nullptr == m_pAIScene)
		return E_FAIL;

	XMStoreFloat4x4(&m_TransformMatrix, TransformMatrix);

	// 파일입출력
	string tempFileName = "tempTXT.txt";
	remove(tempFileName.c_str()); // 이전에 남아있던거에 덮어쓰지 말고 새로운 파일에 하도록
	m_OutputFile.open(tempFileName, ios::out | ios::binary | ios::app);

	if (!m_OutputFile.is_open()) // 임시파일 열렸는지 확인
	{
		MSG_BOX(TEXT("Failed to open tempTXT.txt"));
		return E_FAIL;
	}

	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	if (eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Bones(m_pAIScene->mRootNode)))
			return E_FAIL;

		_uint iNumBones = m_Bones.size();
		m_OutputFile.write(reinterpret_cast<const char*>(&iNumBones), sizeof(iNumBones));

		for (auto boneIter : m_Bones)
		{
			boneIter->Write_BoneData(m_OutputFile);
		}
	}

	/* 모델을 구성하는 메시들을 생성한다. */
	/* 모델 = 메시 + 메시 + ... */
	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	if (FAILED(Ready_Materials(strModelFilePath.c_str())))
		return E_FAIL;

	if (eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Animations()))
			return E_FAIL;
	}

	if(FAILED(RenameBinaryFile())) // 파일 닫고, 임시파일-> 최종파일 이름
		return E_FAIL;

	if (eType == TYPE_ANIM)
		Create_NonAnimVersion(strModelFilePath , TransformMatrix);

	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	ZeroMemory(m_MeshBoneMatrices, sizeof(_float4x4) * 512);
	m_Meshes[iMeshIndex]->Stock_Matrices(m_Bones, m_MeshBoneMatrices);

	return pShader->Bind_Matrices(pConstantName, m_MeshBoneMatrices, 512);
}

HRESULT CModel::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eTextureType)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	_uint		iMeshMaterialIndex = { m_Meshes[iMeshIndex]->Get_MaterialIndex() };

	if (iMeshMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	if (nullptr != m_Materials[iMeshMaterialIndex].MaterialTextures[eTextureType])
		m_Materials[iMeshMaterialIndex].MaterialTextures[eTextureType]->Bind_ShaderResource(pShader, pConstantName);

	return S_OK;
}

HRESULT CModel::Play_Animation(_float fTimeDelta)
{
	/* 현재 애니메이션에 맞는 뼈의 상태(m_TransformationMatrix)를 갱신해준다. */
	m_Animations[m_iCurrentAnimIndex]->Update_TransformationMatrix(fTimeDelta, m_Bones, m_isLoop);

	for (auto& pBone : m_Bones)
		pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_TransformMatrix));

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
	m_iNumMeshes = m_pAIScene->mNumMeshes;

	m_OutputFile.write(reinterpret_cast<const char*>(&m_iNumMeshes), sizeof(m_iNumMeshes));

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eModelType, m_pAIScene->mMeshes[i], m_OutputFile, m_Bones, XMLoadFloat4x4(&m_TransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath)
{
	m_iNumMaterials = m_pAIScene->mNumMaterials;

	m_OutputFile.write(reinterpret_cast<const char*>(&m_iNumMaterials), sizeof(m_iNumMaterials));

	for (_uint i = 0; i < m_iNumMaterials; i++)
	{
		aiMaterial* pAIMaterial = m_pAIScene->mMaterials[i];

		MATERIAL_TEXTURE			MeshMaterial{};

		for (_uint j = aiTextureType_DIFFUSE; j < AI_TEXTURE_TYPE_MAX; j++)
		{
			aiString		strTextureFilePath;

			if (FAILED(pAIMaterial->GetTexture(aiTextureType(j), 0, &strTextureFilePath)))
				continue;

			_char			szDrive[MAX_PATH] = { "" };
			_char			szDirectory[MAX_PATH] = { "" };
			_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDirectory, MAX_PATH, nullptr, 0, nullptr, 0);


			_char			szFileName[MAX_PATH] = { "" };
			_char			szEXT[MAX_PATH] = { "" };

			/* ..\Bin\Resources\Models\Fiona\ */
			_splitpath_s(strTextureFilePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szEXT, MAX_PATH);

			_char			szFullPath[MAX_PATH] = { "" };
			strcpy_s(szFullPath, szDrive);
			strcat_s(szFullPath, szDirectory);
			strcat_s(szFullPath, szFileName);
			strcat_s(szFullPath, szEXT);

			_tchar			szPerfectPath[MAX_PATH] = { L"" };

			MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szPerfectPath, MAX_PATH);


			MeshMaterial.MaterialTextures[j] = CTexture::Create(m_pDevice, m_pContext, szPerfectPath);
			if (nullptr == MeshMaterial.MaterialTextures[j])
				return E_FAIL;

			m_OutputFile.write(reinterpret_cast<const char*>(&i), sizeof(_uint));
			m_OutputFile.write(reinterpret_cast<const char*>(&j), sizeof(_uint));
			m_OutputFile.write(reinterpret_cast<const char*>(&szFullPath), sizeof(szFullPath));
			m_strDirectory = szDirectory;
		}
		
		m_Materials.push_back(MeshMaterial);

	}
	m_OutputFile.write(reinterpret_cast<const char*>(&m_iNumMaterials), sizeof(_uint)); // loop멈추기용
	
	return S_OK;
}

HRESULT CModel::Ready_Bones(aiNode* pAINode, _int iParentIndex)
{
	CBone* pBone = CBone::Create(pAINode, iParentIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	_int		iParent = m_Bones.size() - 1;

	for (size_t i = 0; i < pAINode->mNumChildren; i++)
	{
		Ready_Bones(pAINode->mChildren[i], iParent);
	}

	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	m_iNumAnimations = m_pAIScene->mNumAnimations;
	m_OutputFile.write(reinterpret_cast<const char*>(&m_iNumAnimations), sizeof(m_iNumAnimations));

	for (size_t i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}

	for (auto vecIter : m_Animations)
	{
		vecIter->Write_AnimationData(m_OutputFile);
	}

	return S_OK;
}

HRESULT CModel::RenameBinaryFile()
{
	m_OutputFile.close();

	string tempFileName = "tempTXT.txt";
	if (!m_OutputFile)
	{
		MSG_BOX(TEXT("Failed to write data to tempTXT.txt"));
		remove(tempFileName.c_str()); // 임시파일 삭제
		return E_FAIL;
	}
	// 원본파일 삭제
	size_t lastSlashPos = m_strDirectory.find_last_of("/");
	size_t secondLastSlashPos = m_strDirectory.find_last_of("/", lastSlashPos - 1);

	if (secondLastSlashPos != string::npos) {
		string modelName = m_strDirectory.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1);
		m_strDirectory = modelName;
	}

	string strFolderName = "Anim/";
	if (m_eModelType == TYPE_NONANIM)
		strFolderName = "Non" + strFolderName;

	m_strDirectory = "../../../model_txt/" + strFolderName + m_strDirectory + ".txt";
	remove(m_strDirectory.c_str());

	// 임시파일의 파일이름을 원본파일의 이름으로 변경
	if (rename(tempFileName.c_str(), m_strDirectory.c_str()) != 0)
	{
		MSG_BOX(TEXT("Failed to rename : tempTXT.txt"));
		remove(tempFileName.c_str()); // 임시파일 삭제
		return E_FAIL;
	}

	return S_OK;
}

void CModel::Create_NonAnimVersion(const string& strModelFilePath, _fmatrix TransformMatrix)
{
	string strModelName_NonAnim = m_strDirectory + "_NonAnim";
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring wstrModelName_NonAnim = converter.from_bytes(strModelName_NonAnim);
	wstring wstrPrototypeTag = L"Prototype_Component_Model_" + wstrModelName_NonAnim;

	m_pGameInstance->Add_Prototype(Level_MapTool, wstrPrototypeTag, Create(m_pDevice, m_pContext, TYPE_NONANIM, strModelFilePath, TransformMatrix));
}

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

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);

	m_Animations.clear();

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);

	m_Bones.clear();

	for (auto& Material : m_Materials)
	{
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_Release(Material.MaterialTextures[i]);
	}

	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);

	m_Meshes.clear();

	m_Importer.FreeScene();
}
