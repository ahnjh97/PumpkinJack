#pragma once
#include "Base.h"

BEGIN(Engine)

class ENGINE_DLL CCell final : public CBase
{
public:
	enum POINT { POINT_A, POINT_B, POINT_C, POINT_END };
	enum LINE { LINE_AB, LINE_BC, LINE_CA, LINE_END };
private:
	CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCell() = default;

public:
	_vector Get_Point(POINT ePoint) { return XMLoadFloat3(&m_vPoints[ePoint]); }

	_int Get_NeighborIndex(LINE eLine) { return m_iNeighborIndices[eLine]; }
	void Set_NeighborIndex(LINE eLine, _int iNeighborIndex) { m_iNeighborIndices[eLine] = iNeighborIndex; }
	void Reset_NeighborIndex() { m_iNeighborIndices[0] = -1;   m_iNeighborIndices[1] = -1;  m_iNeighborIndices[2] = -1;}

	void SetUp_Neighbor(LINE eLine, CCell* pNeighbor) {	m_iNeighborIndices[eLine] = pNeighbor->Get_Index(); }

	_int Get_Index() { return m_iIndex; }
	void Set_CellIndex(_uint iCellIndex) { m_iIndex = iCellIndex; }

	_bool Get_Alone();

	_int Get_TerrainIndex() { return m_iTerrainIndex; }
	void Set_TerrainIndex(_uint iTerrainIndex) { m_iTerrainIndex = iTerrainIndex; }

	_float3 Get_LineDir() { return m_vLineDir; }

public:
	HRESULT Initialize(_int iTerrainIndex, const _float3* pPoints, const _int* pNeighborIndices, _uint iIndex);
	_bool IsIn(_fvector vPosition, _fmatrix TerrainWorldMatrix, _int* pNeighborIndex);

	_bool Compare_Points(_fvector vSourPoint, _fvector vDestPoint);
	_bool Check_Redundancy(CCell* pCell);

#ifdef _DEBUG
public:
	virtual HRESULT Render();
#endif

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

private:
	_float3			m_vPoints[POINT_END];
	_uint			m_iIndex = { 0 };

	_int			m_iNeighborIndices[LINE_END] = { -1, -1, -1 };
	_uint			m_iTerrainIndex = {};
	_float3			m_vLineDir = {};

#ifdef _DEBUG

private:
	class CVIBuffer_Cell* m_pVIBuffer = { nullptr };

#endif

public:
	static CCell* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _int iTerrainIndex, const _float3* vPoints, const _int* pNeighborIndices, _uint iIndex);
	virtual void Free() override;
};

END