#pragma once

#include "Base.h"

/* 렌더링파이프라인에 필요한 ViewMatrix, ProjMatrix보존하낟. */
/* 기타 클라이언트 로직에 필요한 데이터들을 만들고 저장ㅎ나다. (View, Proj Inv, CamPosition */

BEGIN(Engine)

class CPipeLine final : public CBase
{
public:
	enum TRANSFORMSTATE { D3DTS_VIEW, D3DTS_PROJ, D3DTS_END };
private:
	CPipeLine();
	virtual ~CPipeLine() = default;

public:
	void Set_Transform(TRANSFORMSTATE eState, _fmatrix TransformMatrix) {
		XMStoreFloat4x4(&m_TransformMatrices[eState], TransformMatrix);
	}
public:
	_matrix Get_Transform_Matrix(TRANSFORMSTATE eState) const {
		return XMLoadFloat4x4(&m_TransformMatrices[eState]);
	}
	_float4x4 Get_Transform_Float4x4(TRANSFORMSTATE eState) const {
		return m_TransformMatrices[eState];
	}
	_matrix Get_Transform_Matrix_Inverse(TRANSFORMSTATE eState) const {
		return XMLoadFloat4x4(&m_TransformInverseMatrices[eState]);
	}
	_float4x4 Get_Transform_Float4x4_Inverse(TRANSFORMSTATE eState) const {
		return m_TransformInverseMatrices[eState];
	}

	_vector Get_CamPosition_Vector() const {
		return XMLoadFloat4(&m_vCamPosition);
	}

	_float4 Get_CamPosition_Float4() const {
		return m_vCamPosition;
	}

public:
	HRESULT Initialize();
	void Tick();

private:
	_float4x4			m_TransformMatrices[D3DTS_END];
	_float4x4			m_TransformInverseMatrices[D3DTS_END];
	_float4				m_vCamPosition;

public:
	static CPipeLine* Create();
	virtual void Free() override;	
};

END