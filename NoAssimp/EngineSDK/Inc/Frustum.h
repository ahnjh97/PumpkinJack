#pragma once
#include "Base.h"

/* 투영공간상의 절두체의 여덟개 점을 정의한다. */
/* Tick함수를 통해서 매 프레임마다 한번씩만 월드로 역변환 시키는 작업을 해 두겠다. */
/* 절두체와 점의 비교를 수행하여 안에 있는지? 없는지? 판단한다. */

BEGIN(Engine)

class CFrustum final : public CBase
{
private:
	CFrustum();
	virtual ~CFrustum() = default;

public:
	HRESULT Initialize();
	void Tick();

	void Transform_LocalSpace(_fmatrix WorldMatrixInv);
	_bool isIn_WorldSpace(_fvector vWorldPos, _float fRange);
	_bool isIn_LocalSpace(_fvector vLocalPos, _float fRange);

private:
	_float3				m_vOriginalPoints[8] = {};
	_float3				m_vWorldPoints[8] = {};
	_float4				m_vWorldPlanes[6] = {};
	_float4				m_vLocalPlanes[6] = {};

private:
	class CGameInstance* m_pGameInstance = { nullptr };

private:
	void Make_Planes(const _float3* pPoints, _float4* pPlanes);

public:
	static CFrustum* Create();
	virtual void Free() override;
};

END