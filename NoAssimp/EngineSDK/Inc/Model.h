#pragma once

#include "Component.h"
#include "Animation.h"
#include <fstream>

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
	_bool isFinished() { return m_Animations[m_iCurrentAnimIndex]->isFinished(); }
	TYPE Get_ModelType() { return m_eModelType; }

public:
	void Set_Animation(_uint iAnimIndex, _bool isLoop) { m_iCurrentAnimIndex = iAnimIndex; m_isLoop = isLoop; }


public: // 바운딩박스
	_bool Check_BoundingBox(const class CTransform* pTransform) const;
	_float3 Check_Meshes(const class CTransform* pTransform) const;

public:
	virtual HRESULT Initialize_Prototype(TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_BoneMatrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
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
	_uint						m_iNumAnimations = { 0 };
	_uint						m_iCurrentAnimIndex = { 0 };
	_bool						m_isLoop = { false };
	vector<CAnimation*>			m_Animations;
	_float4x4					m_MeshBoneMatrices[512];

	// 파일입출력 변수
	string						m_strDirectory;
	streampos					m_filePointerPos;
	ifstream					m_InputFile;
	BoundingBox					m_BoundingBox;

private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const string& strModelFilePath, _fmatrix TransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END