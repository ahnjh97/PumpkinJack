#pragma once

#include "Base.h"
#include <fstream>

/* 특정 애니메이션이 사용하고 있는 뼈들 중, 하나의 정보를 표현한다.. */
/* 이 뼈는 시간에 따라 어떤 상태(KeyFrame)를 취해야하는가? */

BEGIN(Engine)

class CChannel final : public CBase
{
private:
	CChannel();
	virtual ~CChannel() = default;

public:
	HRESULT Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	void Invalidate_TransformationMatrix(const vector<class CBone*>& Bones, _float fTrackPosition, _uint* pCurrentKeyFrameIndex);

	void Write_ChannelData(ofstream& fileStream);

private:
	_char				m_szName[MAX_PATH] = { "" };
	_int				m_iBoneIndex = { -1 };
	_uint				m_iNumKeyFrames = { 0 };
	vector<KEYFRAME>	m_KeyFrames;

public:
	static CChannel* Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones);
	virtual void Free() override;
};

END