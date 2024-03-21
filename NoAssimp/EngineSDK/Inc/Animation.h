#pragma once

#include "Base.h"
#include <fstream>

/* 하나의 애니메이션이 몇개의 뼈를 컨트롤해야하는지. 그 뼈들은 무엇인지.  */
/* 이 애니메이션을 재생하는데 걸리는 총 거리.. */
/* 이 애니메이션을 구동하는 속도. */

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	CAnimation(const CAnimation& rhs);
	virtual ~CAnimation() = default;

public:
	_bool isFinished() const { return m_isFinished; }
	string Get_Name() { return string(m_szName); }
	_float Get_TickPerSecond() { return m_fTickPerSecond; }
	vector<class CChannel*> Get_Channels() { return m_Channels; }
	_uint Get_PlayCount() { return m_iPlayCount; }
	_float Get_PlayRatio() { return m_fTrackPosition / m_fDuration; }

	void Set_Coefficient(_float fCoefficient) { m_fCoefficient = fCoefficient; }
	void Set_IsFinished(_bool bIsFinished) { m_isFinished = bIsFinished; }
	void Reset_PlayCount() { m_iPlayCount = 0; }

public:
	HRESULT Initialize(const vector<class CBone*>& Bones, ifstream& fileStream);
	void Update_TransformationMatrix(_float fTimeDelta, const vector<class CBone*>& Bones, _bool isLoop);
	void Reset(const vector<class CBone*>& Bones);
	void Lerp_Animation(const vector<class CBone*>& Bones, vector<KEYFRAME>& vecKeyFrame, _float fRatio);
	void Play_FinalFrame() { m_fTrackPosition = m_fDuration; }

public:
	void Read_AnimationData(ifstream& fileStream);

private:
	_char						m_szName[MAX_PATH] = { "" };

	_bool						m_isFinished = { false };

	_uint						m_iNumChannels = { 0 };
	_uint						m_iPlayCount = { 0 };

	_float						m_fDuration = { 0.0f };		 /* 전체 재생 길이. */
	_float						m_fTickPerSecond = { 0.0f }; /* 초당 얼마나 재생을 해야하는가 (속도) */
	_float						m_fTrackPosition = { 0.0f }; /* 현재 애니메이션이 어디까지 재생되었는지"? */
	_float						m_fCoefficient = { 1.f };




	vector<class CChannel*>		m_Channels;
	vector<_uint>				m_CurrentKeyFrameIndices;



public:
	static CAnimation* Create(const vector<class CBone*>& Bones, ifstream& fileStream);
	CAnimation* Clone();
	virtual void Free() override;
};

END