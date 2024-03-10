#pragma once

#include "Component.h"
#include "Animation.h"
#include <fstream>

BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
private:
	CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel(const CModel& rhs);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const { return m_iNumMeshes; }
	_bool isFinished() { return m_Animations[m_iCurrentAnimIndex]->isFinished(); }
	TYPE Get_ModelType() { return m_eModelType; }
	string Get_ModelName() { return m_strModelName; }
	_float3 Get_HipTranslation(){ return m_vHipTranslation; }
	_bool IsNonAnimVersion() { return m_bIsNonAnimVersion; }
	_float4x4 Get_ModelTransformMat() { return m_TransformMatrix; }

	void Set_Animation(_uint iAnimIndex, _bool isLoop) {
		if (m_bIsNonAnimVersion) return;
		m_iCurrentAnimIndex = iAnimIndex; m_isLoop = isLoop;
		if (m_iCurrentAnimIndex >= m_iNumAnimations) {
			m_iCurrentAnimIndex = rand() % m_iNumAnimations;}}

	void Set_Hide(_bool bHide) { m_bHide = bHide; }
	void Copy_Animations(vector<CAnimation*>& vecAnims) { vecAnims = m_Animations; }
	void Set_RealRoot(_uint iRealRoot) { m_tConfig.iRealRoot = iRealRoot; }


public: // 바운딩박스
	_bool Check_BoundingBox(const class CTransform* pTransform) const;
	_float3 Check_Meshes(const class CTransform* pTransform) const;

public:
	virtual HRESULT Initialize_Prototype(TYPE eType, const string& strModelName, _fmatrix TransformMatrix, MODEL_CONFIG tConfig, _bool bNonAnimVersion);
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
	vector<MATERIAL_TEXTURE>	m_Materials;

	_float4x4					m_TransformMatrix;
	_float3						m_vHipTranslation = {};
	_float4						m_vScale = {};
	_float4						m_vRotation = {};

	vector<class CBone*>		m_Bones;
	_uint						m_iNumAnimations = { 0 };
	_uint						m_iCurrentAnimIndex = { 0 };
	_bool						m_isLoop = { false };
	vector<CAnimation*>			m_Animations;
	_float4x4					m_MeshBoneMatrices[512];
	MODEL_CONFIG				m_tConfig = {};

	// 파일입출력 변수
	string						m_strDirectory;
	streampos					m_filePointerPos;
	ifstream					m_InputFile;
	BoundingBox					m_BoundingBox;

	string						m_strModelName;
	
	_bool						m_bIsNonAnimVersion = { false };
	_bool						m_bHide = { false };


private:
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Bones();
	HRESULT Ready_Animations();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const string& strModelName, _fmatrix TransformMatrix, MODEL_CONFIG tConfig, _bool bNonAnimVersion = false);
	
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END