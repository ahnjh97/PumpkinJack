#include "Model.h"
#include "Bone.h"
#include "Mesh.h"

#include "Shader.h"
#include "Texture.h"
#include <fstream>
#include "GameInstance.h"

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
	, m_eModelType{ rhs.m_eModelType }
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

_bool CModel::Check_BoundingBox(const CTransform* pTransform) const
{
	_float3 vRayDir{}, vRayPos{};
	_float fDis;
	m_pGameInstance->Transform_PickingToLocalSpace(pTransform, &vRayDir, &vRayPos);
	_vector vecRayDir = ::XMVector3Normalize(::XMLoadFloat3(&vRayDir));
	_vector vecRayPos = ::XMLoadFloat3(&vRayPos);
	return m_BoundingBox.Intersects(vecRayPos, vecRayDir, fDis);
}

_float3 CModel::Check_Meshes(const CTransform* pTransform) const
{
	vector<_float3> vecPickPos;
	for (auto iter : m_Meshes)
	{
		_float3 vTemp = iter->Get_PickPos(pTransform);
		_vector vPickPos = ::XMLoadFloat3(&vTemp);
		if (!XMVector3Equal(vPickPos, XMVectorSet(0.f, 0.f, 0.f, 0.f)))
			vecPickPos.push_back(vTemp);
	}

	if(vecPickPos.empty())
		return _float3();

	// 가장 가까운놈 계산 필요
	return vecPickPos[0];
}

HRESULT CModel::Initialize_Prototype(TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix)
{
	m_eModelType = eType;
	size_t lastSlashPos = strModelFilePath.find_last_of("/");
	size_t secondLastSlashPos = strModelFilePath.find_last_of("/", lastSlashPos - 1);

	string strTxt = ".txt";
	if (eType == TYPE_ANIM)
		strTxt = "_Anim" + strTxt;

	if (secondLastSlashPos != string::npos) 
		m_strDirectory = "../../../model_txt/" +  strModelFilePath.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1) + strTxt;

	m_InputFile.open(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!m_InputFile.is_open())
	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}

	::XMStoreFloat4x4(&m_TransformMatrix, TransformMatrix);

	/* 읽은 정보를 바탕으로해서 내가 사용하기 좋게 정리한다.  */
	if (eType == TYPE_ANIM)
	{
		if (FAILED(Ready_Bones()))
			return E_FAIL;
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

	m_InputFile.close();
	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel::Bind_BoneMatrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	if (m_eModelType == TYPE_ANIM)
	{
		ZeroMemory(m_MeshBoneMatrices, sizeof(_float4x4) * 512);
		m_Meshes[iMeshIndex]->Stock_Matrices(m_Bones, m_MeshBoneMatrices);

		return pShader->Bind_Matrices(pConstantName, m_MeshBoneMatrices, 512);
	}
	return S_OK;
}

HRESULT CModel::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint eTextureType)
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
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumMeshes), sizeof(m_iNumMeshes));
	_float3 vMin{}, vMax{};

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eModelType, m_strDirectory, m_InputFile, m_filePointerPos, m_Bones, XMLoadFloat4x4(&m_TransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
		m_filePointerPos = pMesh->Get_IncreasedFilePtPos(); // 파일포인터 위치 갱신

		pair<_float3, _float3> pairMinMax = pMesh->Get_MinMax();

		vMin = _float3(min(pairMinMax.first.x, vMin.x), min(pairMinMax.first.y, vMin.y), min(pairMinMax.first.z, vMin.z));
		vMax = _float3(max(pairMinMax.second.x, vMax.x), max(pairMinMax.second.y, vMax.y), max(pairMinMax.second.z, vMax.z));
	}

	BoundingBox tempBox(vMin, vMax);
	m_BoundingBox = tempBox;
	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath)
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumMaterials), sizeof(m_iNumMaterials));

	while (true)
	{
		_uint iTemp;
		_uint jTemp;
		MATERIAL_TEXTURE			MeshMaterial{};
		_char			szFullPath[MAX_PATH] = { "" };
		_tchar			szPerfectPath[MAX_PATH] = { L"" };
		m_InputFile.read(reinterpret_cast<char*>(&iTemp), sizeof(iTemp));
		if (iTemp == m_iNumMaterials)
			break;
		m_InputFile.read(reinterpret_cast<char*>(&jTemp), sizeof(jTemp));
		m_InputFile.read(reinterpret_cast<char*>(&szFullPath), sizeof(szFullPath));
		
		MultiByteToWideChar(CP_ACP, 0, szFullPath, strlen(szFullPath), szPerfectPath, MAX_PATH);
		MeshMaterial.MaterialTextures[jTemp] = CTexture::Create(m_pDevice, m_pContext, szPerfectPath);
		if (nullptr == MeshMaterial.MaterialTextures[jTemp])
			return E_FAIL;

		if(m_Materials.size() <= iTemp) // 새로운 배열 삽입
			m_Materials.push_back(MeshMaterial);
		else // 벡터 안에있는 배열의 원소 수정
			m_Materials[iTemp].MaterialTextures[jTemp] = MeshMaterial.MaterialTextures[jTemp];
	}


	return S_OK;
}

HRESULT CModel::Ready_Bones()
{
	_uint iNumBones = {};

	m_InputFile.read(reinterpret_cast<char*>(&iNumBones), sizeof(iNumBones));
	m_Bones.reserve(iNumBones);

	for (_uint i = 0; i < iNumBones; i++)
	{
		CBone* pBone = CBone::Create(m_InputFile);
		if (nullptr == pBone)
			return E_FAIL;

		m_Bones.push_back(pBone);
	}

	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumAnimations), sizeof(m_iNumAnimations));
	m_Animations.reserve(m_iNumAnimations);

	for (size_t i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::Create(m_Bones, m_InputFile);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}

	return S_OK;
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
