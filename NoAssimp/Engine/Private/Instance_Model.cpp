#include "VIBuffer_Instance_Mesh.h"
#include "Instance_Model.h"
#include "GameInstance.h"
#include "Texture.h"
#include "Shader.h"

CInstance_Model::CInstance_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{
}

CInstance_Model::CInstance_Model(const CInstance_Model& rhs)
	: CComponent{ rhs }
	, m_vecInstanceMeshes{ rhs.m_vecInstanceMeshes }
	, m_vecMaterials{ rhs.m_vecMaterials }
	, m_iNumInstanceMeshes{rhs.m_iNumInstanceMeshes }
	, m_iNumMaterials{ rhs.m_iNumMaterials }
{
	for (auto& pInstanceMesh : m_vecInstanceMeshes)
		Safe_AddRef(pInstanceMesh);

	for (auto& material : m_vecMaterials) {
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_AddRef(material.MaterialTextures[i]);
	}
}

HRESULT CInstance_Model::Initialize_Prototype(const string& strModelName, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	m_strDirectory = "../../../model_txt/NonAnim/" + strModelName + ".txt";
	m_InputFile.open(m_strDirectory.c_str(), ios::in | ios::binary);
	if (!m_InputFile.is_open())	{
		string tempstr = "Failed To Open : " + m_strDirectory;
		MB(tempstr);
		return E_FAIL;
	}

	if (FAILED(Ready_InstanceMeshes(InstanceDesc)))
		return E_FAIL;

	if (FAILED(Ready_Materials(strModelName.c_str())))
		return E_FAIL;

	m_InputFile.close();

	return S_OK;
}

HRESULT CInstance_Model::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CInstance_Model::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint eTextureType)
{
	if (iMeshIndex >= m_iNumInstanceMeshes)
		return E_FAIL;

	_uint		iMeshMaterialIndex = { m_vecInstanceMeshes[iMeshIndex]->Get_MaterialIndex() };

	if (iMeshMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	if (nullptr != m_vecMaterials[iMeshMaterialIndex].MaterialTextures[eTextureType])	{
		if (FAILED(m_vecMaterials[iMeshMaterialIndex].MaterialTextures[eTextureType]->Bind_ShaderResource(pShader, pConstantName)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CInstance_Model::Render(_uint iMeshIndex)
{
	m_vecInstanceMeshes[iMeshIndex]->Bind_Buffers();
	m_vecInstanceMeshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CInstance_Model::Ready_InstanceMeshes(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	m_InputFile.read(reinterpret_cast<char*>(&m_iNumInstanceMeshes), sizeof(m_iNumInstanceMeshes));

	for (size_t i = 0; i < m_iNumInstanceMeshes; i++)	{
		CVIBuffer_Instance_Mesh* pInstanceMesh = CVIBuffer_Instance_Mesh::Create(m_pDevice, m_pContext, m_InputFile, InstanceDesc);
		if (nullptr == pInstanceMesh)
			return E_FAIL;

		pInstanceMesh->Initialize(nullptr);

		m_vecInstanceMeshes.push_back(pInstanceMesh);
	}
	return S_OK;
}

HRESULT CInstance_Model::Ready_Materials(const _char* pModelFilePath)
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

 		if (m_vecMaterials.size() <= iTemp) // 새로운 배열 삽입
			m_vecMaterials.push_back(MeshMaterial);
		else // 벡터 안에있는 배열의 원소 수정
			m_vecMaterials[iTemp].MaterialTextures[jTemp] = MeshMaterial.MaterialTextures[jTemp];
	}

	return S_OK;
}

CInstance_Model* CInstance_Model::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const string& strModelName, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc)
{
	CInstance_Model* pInstance = new CInstance_Model(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype(strModelName, InstanceDesc))) {
		MSG_BOX(TEXT("Failed To Create : CInstance_Model"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CInstance_Model::Clone(void* pArg)
{
	CInstance_Model* pInstance = new CInstance_Model(*this);
	if (FAILED(pInstance->Initialize(pArg))) {
		MSG_BOX(TEXT("Failed To Cloned : CInstance_Model"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CInstance_Model::Free()
{
	__super::Free();

	for (auto& material : m_vecMaterials) {
		for (size_t i = 0; i < AI_TEXTURE_TYPE_MAX; i++)
			Safe_Release(material.MaterialTextures[i]);
	}
	m_vecMaterials.clear();

	for (auto& pInstanceMesh : m_vecInstanceMeshes)
		Safe_Release(pInstanceMesh);
	m_vecInstanceMeshes.clear();
}
