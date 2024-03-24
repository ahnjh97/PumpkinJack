#pragma once
#include "Bounding.h"

BEGIN(Engine)

class CBounding_AABB final : public CBounding
{
public:
	typedef struct : public CBounding::BOUNDING_DESC
	{
		_float3		vSize;
	}BOUNDING_AABB_DESC;
private:
	CBounding_AABB(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CBounding_AABB() = default;

public:
	virtual void* Get_BoundingDesc() { return m_pBoundingDesc; }

public:
	virtual HRESULT Initialize(CBounding::BOUNDING_DESC* pBoundingDesc) override;
	virtual void Tick(_fmatrix WorldMatrix) override;
	virtual _bool Intersect(CCollider::TYPE eType, CBounding* pBounding) override;
	virtual _bool Intersect(CBounding_AABB* pTargetBounding);
#ifdef _DEBUG
public:
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch) override;
#endif

private:
	BoundingBox* m_pBoundingDesc_Original = { nullptr };
	BoundingBox* m_pBoundingDesc = { nullptr };

private:
	_float3 Compute_Min() {
		_float3		vResult{};
		XMStoreFloat3(&vResult, XMLoadFloat3(&m_pBoundingDesc->Center) - XMLoadFloat3(&m_pBoundingDesc->Extents));
		return vResult;
	}
	_float3 Compute_Max() {
		_float3		vResult{};
		XMStoreFloat3(&vResult, XMLoadFloat3(&m_pBoundingDesc->Center) + XMLoadFloat3(&m_pBoundingDesc->Extents));
		return vResult;
	}

public:
	static CBounding_AABB* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CBounding::BOUNDING_DESC* pBoundingDesc);
	virtual void Free() override;
};

END