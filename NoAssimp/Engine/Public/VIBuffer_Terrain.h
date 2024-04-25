#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Terrain final : public CVIBuffer
{
private:
	CVIBuffer_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Terrain(const CVIBuffer_Terrain& rhs);
	virtual ~CVIBuffer_Terrain() = default;

public:
	_int Get_NumVerticesX() { return m_iNumVerticesX; }
	_int Get_NumVerticesZ() { return m_iNumVerticesZ; }

public:
	virtual HRESULT Initialize_Prototype(_uint iX, _uint iZ);
	virtual HRESULT Initialize_Prototype(const wstring& strHeightMapFilePath, _bool bQuadTree);
	virtual HRESULT Initialize(void* pArg);

public:
	void Culling(_fmatrix WorldMatrixInv);

	_float3 Get_PickPos(const class CTransform* pTransform) const; 
	_int Get_TriangleIndex(const class CTransform* pTransform, _float3& vA, _float3& vB, _float3& vC) const;

private:
	_uint				m_iNumVerticesX = { 0 };
	_uint				m_iNumVerticesZ = { 0 };

	_float3*			m_pVerticesPos = { nullptr };  

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	class CQuadTree_Terrain*	m_pQuadTree_Terrain = { nullptr };

public:
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strHeightMapFilePath, _bool bQuadTree = false);
	static CVIBuffer_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iX, _uint iZ);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};

END
