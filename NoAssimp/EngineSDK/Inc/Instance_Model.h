#pragma once
#include "VIBuffer_Instance.h"
#include "Component.h"
#include <fstream>

BEGIN(Engine)

class ENGINE_DLL CInstance_Model final : public CComponent
{
private:
	CInstance_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CInstance_Model(const CInstance_Model& rhs);
	virtual ~CInstance_Model() = default;

public:
	_uint Get_NumInstanceMeshes() { return m_iNumInstanceMeshes; }

public:
	virtual HRESULT Initialize_Prototype(const string& strModelName, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint eTextureType);
	HRESULT Render(_uint iMeshIndex);

private:
	HRESULT Ready_InstanceMeshes(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	HRESULT Ready_Materials(const _char* pModelFilePath);
	
private:
	vector<class CVIBuffer_Instance_Mesh*>		m_vecInstanceMeshes;
	vector<MATERIAL_TEXTURE>	m_vecMaterials;
	_uint						m_iNumInstanceMeshes = { };
	_uint						m_iNumMaterials = { };

	ifstream	m_InputFile;
	string		m_strDirectory;

public:
	static CInstance_Model* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const string& strModelName, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END
