#pragma once

#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
public:
	enum TYPE { TYPE_NONANIM, TYPE_ANIM, TYPE_END };

private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const { return m_iNumMeshes; }

public:
	virtual HRESULT Initialize_Prototype(TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, _uint eTextureType);
	HRESULT Play_Animation(_float fTimeDelta);
	HRESULT Render(_uint iMeshIndex);

private:
	TYPE						m_eModelType = { TYPE_END };

private:
	_uint						m_iNumMeshes = { 0 };
	vector<class CMesh*>		m_Meshes;

	_uint						m_iNumMaterials = { 0 };
	vector<MATERIAL_TEXTURE>		m_Materials;

	_float4x4					m_TransformMatrix;

	vector<class CBone*>		m_Bones;

	string						m_strDirectory;
	streampos					m_filePointerPos;

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	//HRESULT Ready_Bones(aiNode* pAINode, _int iParentIndex = -1);



public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END