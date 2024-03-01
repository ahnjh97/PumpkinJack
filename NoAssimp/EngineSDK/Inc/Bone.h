#pragma once

#include "Base.h"

/* 뼈의 정보를 표현하기위한 타입 세가지 제공하낟. */
/* aiNode, aiBone, aiAnimNode */

BEGIN(Engine)

class CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	HRESULT Initialize( _int iParentIndex);
	void Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones);
	_bool Compare_Name(const _char* pBoneName) {
		return !strcmp(m_szName, pBoneName);
	}

private:
	_char				m_szName[MAX_PATH] = { "" };

	/* 이 뼈 만의 상태행렬 */
	_float4x4			m_TransformationMatrix;

	/* 최종 상태행렬. */
	_float4x4			m_CombinedTransformationMatrix;

	_int				m_iParentBoneIndex = { -1 };

public:
	static CBone* Create(_int  iParentIndex);
	virtual void Free() override;
};

END