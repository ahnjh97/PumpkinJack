#pragma once

#include "Base.h"

BEGIN(Engine)

class CAnimation final : public CBase
{
private:
	CAnimation();
	virtual ~CAnimation() = default;

public:
	HRESULT Initialize(const aiAnimation* pAIAnimation);

private:
	vector<class CChannel*>				m_Channels;

public:
	static CAnimation* Create(const aiAnimation* pAIAnimation);
	virtual void Free() override;
};

END