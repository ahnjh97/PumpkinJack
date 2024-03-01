#pragma once

/* 디자이너분들이 저장해준 정점과 인덱스의 정보를 바탕으로해서 정점, 인덱스버퍼를 새엏나다.  */
#include "VIBuffer.h"
#include "Model.h"
#include <fstream>

BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& rhs);
	virtual ~CMesh() = default;

public:
	_uint Get_MaterialIndex() const { return m_iMaterialIndex; }
	streampos Get_IncreasedFilePtPos() { return m_filePointerPos; }

public:
	virtual HRESULT Initialize_Prototype(CModel::TYPE eModelType, string strDirectory, streampos filePtPos, const vector<CBone*>& Bones, _fmatrix TransformMatrix);
	virtual HRESULT Initialize(void* pArg) override;

private:
	_char					m_szName[MAX_PATH] = { "" };

	/* 이 메시는 모델에서 로드해놓은 머테리얼들 중 몇번째 머테리얼을 이용하는가? */
	_uint					m_iMaterialIndex = { 0 };

	_uint					m_iNumBones = { 0 };
	vector<_uint>			m_Bones;

	_uint					m_iFaces = {0};
	string					m_strDirectory;
	streampos				m_filePointerPos;
	ifstream				m_InputFile;

private:
	HRESULT Ready_Vertices_For_NonAnimModel(_fmatrix TransformationMatrix);
	HRESULT Ready_Vertices_For_AnimModel(const vector<CBone*>& Bones);

public:
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CModel::TYPE eModelType, string strDirectory, streampos filePtPos, const vector<class CBone*>& Bones, _fmatrix TransformMatrix);
	virtual CMesh* Clone(void* pArg);
	virtual void Free() override;
};

END