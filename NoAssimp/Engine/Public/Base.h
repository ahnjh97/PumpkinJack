#pragma once

#include "Engine_Defines.h"

BEGIN(Engine)

class ENGINE_DLL CBase abstract
{
protected:
	CBase();
	virtual ~CBase() = default;

public:
	/* 레퍼런스 카운트를 증가시킨다. */
	/* 리턴 : 증가하고 난 이후의 RefCnt를 리턴 */
	unsigned int AddRef();

	/* 레퍼런스 카운트를 감소시킨다. or 삭제한다. */
	/* 리턴 : 감소시키기 이전의 RefCnt를 리턴 */
	unsigned int Release();
private:

	unsigned int		m_iRefCnt = { 0 };

public:
	virtual void Free();
};

END