#pragma once
#include "Base.h"

BEGIN(Engine)

class CQuadTreeCell final : public CBase
{
public:
	enum CORNER { CORNER_RT, CORNER_RB, CORNER_LB, CORNER_LT, CORNER_END };

private:
	CQuadTreeCell();
	virtual ~CQuadTreeCell() = default;

public: _int Get_NumGroundCells() { return m_vecGroundCells.size(); }

public:
	HRESULT Initialize(_float2 fRT, _float2 fRB, _float2 fLB, _float2 fLT, vector<FACE> _vecFaces);

public:
	_bool Check_QuadTreeCell(_fvector vPlayerPos, _matrix matWorld, _Out_ _float& _fHeight);
	CORNER Check_Corner(_fvector vPos);

	void PartitionFaces(vector<FACE>& _vecRT, vector<FACE>& _vecRB, vector<FACE>& _vecLB, vector<FACE>& _vecLT, vector<FACE> _vecFaces);
	void PushIfNotAlreadyInserted(vector<CORNER>& _vecQuad, CORNER eQuad);

private:
	_float2		m_fCenter = {};
	_float2		m_fCorners[CORNER_END] = {};

	class CQuadTreeCell* m_pChildren[CORNER_END] = { nullptr, nullptr, nullptr, nullptr };
	vector<class CCell*> m_vecGroundCells;

public:
	static CQuadTreeCell* Create(_float2 fRT, _float2 fRB, _float2 fLB, _float2 fLT, vector<FACE> _vecFaces);
	virtual void Free() override;
};

END

