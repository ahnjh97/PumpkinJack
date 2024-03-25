#pragma once
#include "Bounding.h"

BEGIN(Engine)

class CBounding_OBB final : public CBounding
{
public:
	typedef struct : CBounding::BOUNDING_DESC
	{
		_float3		vSize;
		_float3		vRotation;
	}BOUNDING_OBB_DESC;

	typedef struct
	{
		_float3		vCenter;
		_float3		vExtentDir[3];
		_float3		vAlignDir[3];
	}OBB_DESC;

private:
	CBounding_OBB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBounding_OBB() = default;

public:
	virtual void* Get_BoundingDesc() { return m_pBoundingDesc; }

public:
	virtual HRESULT Initialize(CBounding::BOUNDING_DESC* pBoundingDesc) override;
	virtual void Tick(_fmatrix WorldMatrix) override;
	virtual _bool Intersect(CCollider::TYPE eType, CBounding* pBoundingDesc) override;
	virtual _bool Intersect_Ray(_fvector vRayOrigin, _fvector vRayDir, _float& fDis) override;
	_bool Intersect(CBounding_OBB* pBoundingDesc);

#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
	BoundingOrientedBox* m_pBoundingDesc_Original = { nullptr };
	BoundingOrientedBox* m_pBoundingDesc = { nullptr };

private:
	OBB_DESC Compute_OBBDesc();

public:
	static CBounding_OBB* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBounding::BOUNDING_DESC* pBoundingDesc);
	virtual void Free() override;
};

END