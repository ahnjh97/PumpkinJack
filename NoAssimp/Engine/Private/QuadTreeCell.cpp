#include "QuadTreeCell.h"
#include "Cell.h"

CQuadTreeCell::CQuadTreeCell()
{

}

HRESULT CQuadTreeCell::Initialize(_float2 fRT, _float2 fRB, _float2 fLB, _float2 fLT, const vector<FACE>& _vecFaces)
{
	m_fCorners[CORNER_RT] = fRT;
	m_fCorners[CORNER_RB] = fRB;
	m_fCorners[CORNER_LB] = fLB;
	m_fCorners[CORNER_LT] = fLT;
	m_fCenter = _float2((m_fCorners[CORNER_LT].x + m_fCorners[CORNER_RB].x) * 0.5f, (m_fCorners[CORNER_LT].y + m_fCorners[CORNER_RB].y) * 0.5f);

	if (_vecFaces.empty())
		return S_OK;

	if (_vecFaces.size() <= 8) {	// 삼각형 개수가 8개 이하일 경우 더 이상 자식을 생성하지 않음
		for (auto& face : _vecFaces) {
			CCell* pCell = CCell::Create(face.vA, face.vB, face.vC);
			m_vecGroundCells.emplace_back(pCell);
		}
		return S_OK;
	}

	if (abs(fRT.x - fLT.x) <= 8.f || abs(fRT.y - fRB.y) <= 8.f) {
		for (auto& face : _vecFaces) {
			CCell* pCell = CCell::Create(face.vA, face.vB, face.vC);
			m_vecGroundCells.emplace_back(pCell);
		}
		return S_OK;
	}

	_float2 fRC{}, fBC{}, fLC{}, fTC{};
	fRC = _float2(m_fCorners[CORNER_RT].x, (m_fCorners[CORNER_RT].y + m_fCorners[CORNER_RB].y) * 0.5f);
	fBC = _float2((m_fCorners[CORNER_RB].x + m_fCorners[CORNER_LB].x) *	0.5f, m_fCorners[CORNER_RB].y);
	fLC = _float2(m_fCorners[CORNER_LB].x, (m_fCorners[CORNER_LB].y + m_fCorners[CORNER_LT].y) * 0.5f);
	fTC = _float2((m_fCorners[CORNER_LT].x + m_fCorners[CORNER_RT].x) * 0.5f, m_fCorners[CORNER_RT].y);

	vector<FACE> vecRT, vecRB, vecLB, vecLT;

	PartitionFaces(vecRT, vecRB, vecLB, vecLT, _vecFaces);

	m_pChildren[CORNER_RT] = CQuadTreeCell::Create(m_fCorners[CORNER_RT], fRC, m_fCenter, fTC, vecRT);
	m_pChildren[CORNER_RB] = CQuadTreeCell::Create(fRC, m_fCorners[CORNER_RB], fBC, m_fCenter, vecRB);
	m_pChildren[CORNER_LB] = CQuadTreeCell::Create(m_fCenter, fBC, m_fCorners[CORNER_LB], fLC, vecLB);
	m_pChildren[CORNER_LT] = CQuadTreeCell::Create(fTC, m_fCenter, fLC, m_fCorners[CORNER_LT],  vecLT);

	return S_OK;
}

_bool CQuadTreeCell::Check_QuadTreeCell(_fvector vPlayerPos, _matrix matWorld, _Out_ _float& _fHeight)
{
	if (m_vecGroundCells.empty()) {
		if (nullptr == m_pChildren[CORNER_RT]) 
			return false;
	}
	
	_bool bFoundGround = false;
	if (!m_vecGroundCells.empty()) {	// 내가 가진 셀들을 순회하며 검사

		for (auto& groundCell : m_vecGroundCells) {
			if (true == groundCell->IsIn(vPlayerPos)) {
				_float fTempHeight = XMVectorGetY(XMVector3TransformCoord(groundCell->Get_Point(CCell::POINT_A), matWorld));
				_fHeight = max(_fHeight, fTempHeight);
				bFoundGround = true;
			}
		}

		if (bFoundGround) {

			if (nullptr == m_pChildren[CORNER_RT])
				return true;
			else
			{
				CORNER eCorner = Check_Corner(vPlayerPos);
				m_pChildren[eCorner]->Check_QuadTreeCell(vPlayerPos, matWorld, _fHeight);

				return true;
			}
		}

		if (!bFoundGround && nullptr == m_pChildren[CORNER_RT])
			return false;
	}

	CORNER eCorner = Check_Corner(vPlayerPos);
	if (true == m_pChildren[eCorner]->Check_QuadTreeCell(vPlayerPos, matWorld, _fHeight))
		return true;
	
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

void CQuadTreeCell::PartitionFaces(vector<FACE>& _vecRT, vector<FACE>& _vecRB, vector<FACE>& _vecLB, vector<FACE>& _vecLT, const vector<FACE>& _vecFaces)
{
	if (_vecFaces.empty())
		return;

	for (auto& face : _vecFaces)
	{
		const CORNER eCornerA = Check_Corner(XMLoadFloat3(&face.vA));
		const CORNER eCornerB = Check_Corner(XMLoadFloat3(&face.vB));
		const CORNER eCornerC = Check_Corner(XMLoadFloat3(&face.vC));

		if (eCornerA != eCornerB || eCornerA != eCornerC)
		{ 
			CCell* pCell = CCell::Create(face.vA, face.vB, face.vC);
			m_vecGroundCells.emplace_back(pCell);
		}
		else // 삼각형들의 세 점이 속한 사분면이 전부 동일할때
		{
			switch (eCornerA)
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

void CQuadTreeCell::PushIfNotAlreadyInserted(vector<CORNER>& _vecQuad, CORNER eQuad)
{
	if (_vecQuad.empty())
		_vecQuad.emplace_back(eQuad);
	else
	{
		_bool bAlreadyInserted = { false };

		for (auto& quad : _vecQuad) {
			if (quad == eQuad)
				bAlreadyInserted = true;
		}

		if (false == bAlreadyInserted)
			_vecQuad.emplace_back(eQuad);
	}
}

CQuadTreeCell* CQuadTreeCell::Create(_float2 fRT, _float2 fRB, _float2 fLB, _float2 fLT, const vector<FACE>& _vecFaces)
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

	if (m_pChildren[CORNER_RT] != nullptr) {
		for (_int i = 0; i < CORNER_END; i++)
			Safe_Release(m_pChildren[i]);
	}
}

