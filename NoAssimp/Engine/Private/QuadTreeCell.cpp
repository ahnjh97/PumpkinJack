#include "QuadTreeCell.h"
#include "GameInstance.h"
#include "Cell.h"

CQuadTreeCell::CQuadTreeCell()
{

}

HRESULT CQuadTreeCell::Initialize(_float2 fRT, _float2 fRB, _float2 fLB, _float2 fLT, vector<FACE> _vecFaces)
{
	if (_vecFaces.size() <= 8) {	// 삼각형 개수가 8개 이하일 경우 더 이상 자식을 생성하지 않음
		for (auto& face : _vecFaces) {
			CCell* pCell = CCell::Create(face.vA, face.vB, face.vC);
			m_vecGroundCells.emplace_back(pCell);
		}
		return S_OK;
	}

	m_fCorners[CORNER_RT] = fRT;
	m_fCorners[CORNER_RB] = fRB;
	m_fCorners[CORNER_LB] = fLB;
	m_fCorners[CORNER_LT] = fLT;
	m_fCenter = _float2((m_fCorners[CORNER_LT].x + m_fCorners[CORNER_RB].x) * 0.5f, (m_fCorners[CORNER_LT].y + m_fCorners[CORNER_RB].y) * 0.5f);

	_float2 fRC{}, fBC{}, fLC{}, fTC{};
	fRC = _float2((m_fCorners[CORNER_RT].x + m_fCorners[CORNER_RB].x) * 0.5f, (m_fCorners[CORNER_RT].y + m_fCorners[CORNER_RB].y) * 0.5f);
	fBC = _float2((m_fCorners[CORNER_RB].x + m_fCorners[CORNER_LB].x) *	0.5f, (m_fCorners[CORNER_RB].y + m_fCorners[CORNER_LB].y) * 0.5f);
	fLC = _float2((m_fCorners[CORNER_LB].x + m_fCorners[CORNER_LT].x) * 0.5f, (m_fCorners[CORNER_LB].y + m_fCorners[CORNER_LT].y) * 0.5f);
	fTC = _float2((m_fCorners[CORNER_LT].x + m_fCorners[CORNER_RT].x) * 0.5f, (m_fCorners[CORNER_LT].y + m_fCorners[CORNER_RT].y) * 0.5f);

	vector<FACE> vecRT, vecRB, vecLB, vecLT;

	PartitionFaces(vecRT, vecRB, vecLB, vecLT, _vecFaces);

	m_pChildren[CORNER_RT] = CQuadTreeCell::Create(fTC, m_fCorners[CORNER_RT], fRC, m_fCenter, vecRT);
	m_pChildren[CORNER_RB] = CQuadTreeCell::Create(m_fCenter, fRC, m_fCorners[CORNER_RB], fBC, vecRB);
	m_pChildren[CORNER_LB] = CQuadTreeCell::Create(fLC, m_fCenter, fBC, m_fCorners[CORNER_LB], vecLB);
	m_pChildren[CORNER_LT] = CQuadTreeCell::Create(m_fCorners[CORNER_LT], fTC, m_fCenter, fLC, vecLT);

	return S_OK;
}

_bool CQuadTreeCell::Check_QuadTreeCell(_fvector vPlayerPos, _matrix matWorld, _Out_ _float& _fHeight)
{
	if (m_vecGroundCells.empty()) {
		CORNER eCorner = Check_Corner(vPlayerPos);
		m_pChildren[eCorner]->Check_QuadTreeCell(vPlayerPos, matWorld, _fHeight);
	}
	else
	{
		for (auto& groundCell : m_vecGroundCells) {
			if (true == groundCell->IsIn(vPlayerPos)) {
				_fHeight = XMVectorGetY(XMVector3TransformCoord(groundCell->Get_Point(CCell::POINT_A), matWorld));
				return true;
			}
		}
	}

	return false;
}

CQuadTreeCell::CORNER CQuadTreeCell::Check_Corner(_fvector vPos)
{
	if (m_fCenter.x <= XMVectorGetX(vPos)) {
		if (m_fCenter.y <= XMVectorGetZ(vPos))
			return CORNER_RT;
		else
			return CORNER_RB;
	}
	else
	{
		if (m_fCenter.y >= XMVectorGetZ(vPos))
			return CORNER_LB;
		else
			return CORNER_LT;
	}
}

void CQuadTreeCell::PartitionFaces(vector<FACE>& _vecRT, vector<FACE>& _vecRB, vector<FACE>& _vecLB, vector<FACE>& _vecLT, vector<FACE> _vecFaces)
{
	if (_vecFaces.empty())
		return;

	for (auto& face : _vecFaces)
	{
		unordered_set<CORNER> quadSet; // 몇 사분면에 추가할건지
		CORNER eCornerA = Check_Corner(XMLoadFloat3(&face.vA));
		quadSet.insert(eCornerA);
		CORNER eCornerB = Check_Corner(XMLoadFloat3(&face.vB));
		quadSet.insert(eCornerB);
		CORNER eCornerC = Check_Corner(XMLoadFloat3(&face.vC));
		quadSet.insert(eCornerC);

		for (auto& quad : quadSet) {
			switch (quad)
			{
			case CORNER_RT:
				_vecRT.emplace_back(face);
				break;
			case CORNER_RB:
				_vecRB.emplace_back(face);
				break;
			case CORNER_LB:
				_vecLB.emplace_back(face);
				break;
			case CORNER_LT:
				_vecLT.emplace_back(face);
				break;
			}
		}
	}
}

CQuadTreeCell* CQuadTreeCell::Create(_float2 fRT, _float2 fRB, _float2 fLB, _float2 fLT, vector<FACE> _vecFaces)
{
	CQuadTreeCell* pInstance = new CQuadTreeCell();

	if (FAILED(pInstance->Initialize(fRT, fRB, fLB, fLT, _vecFaces)))
	{
		MSG_BOX(TEXT("Failed to Create : CQuadTreeCell"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQuadTreeCell::Free()
{
	__super::Free();

	for (auto& groundCell : m_vecGroundCells)
		Safe_Release(groundCell);
}

