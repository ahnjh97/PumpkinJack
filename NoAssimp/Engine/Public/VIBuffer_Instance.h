#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance abstract : public CVIBuffer
{
public:
	enum INSTANCE_SHAPE { SHAPE_DEFAULT, SHAPE_HEMISPHERE, SHAPE_RING, SHAPE_CYLINDER, SHAPE_SPHERE, SHAPE_RING_ROTATED, SHAPE_ARROW, SHAPE_END };
	//enum INSTANCE_RESPAWN_TYPE

	typedef struct
	{
		_float3		vPivot;
		_float3		vCenter;
		_float3		vRange;
		_float3		vMinScale, vMaxScale;
		_float2		vLifeTime;
		_bool		isLoop;
		_float2		vSpeed;
		_uint		iNumInstance;
		_uint		iSpriteIndexOffset = {};
		INSTANCE_SHAPE	eShape = { };
		_float		fDistanceLimit = {};
		_float		fDuration = {};
		_bool		bUpFixed = { false };
	}INSTANCE_DESC;

protected:
	CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instance(const CVIBuffer_Instance& rhs);
	virtual ~CVIBuffer_Instance() = default;

public:
	_float3 Get_Pivot() { return m_InstanceDesc.vPivot; }

public:
	virtual HRESULT Initialize_Prototype(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

public:
	virtual _float4 Compute_RandPosition(INSTANCE_SHAPE eShape, _bool bRespawnAtMinHeight = false);

	virtual void Drop(_float fTimeDelta);
	virtual void Spread(_float fTimeDelta);
	virtual void Spread_XZ(_float fTimeDelta);
	virtual void Check_LifeTimes(_float fTimeDelta); // 위치이동 X, LifeTime만 확인
	virtual void MoveToPivot(_float fTimeDelta);
	virtual void TurnAndSpread(_float fTimeDelta, _float fRotationSpeed);

	void	Compute_LifeTime(VTXMATRIX* pVertices, _uint iInstanceIndex, _float fTimeDelta, _bool bRespawnAtMinHeight = false);


protected:
	ID3D11Buffer*			m_pVBInstance = { nullptr };
	_uint					m_iInstanceStride = { 0 };
	_uint					m_iNumInstance = { 0 };
	_uint					m_iIndexCountPerInstance = { 0 };
	VTXMATRIX*				m_pInstanceVertices = { nullptr };
	INSTANCE_DESC			m_InstanceDesc = {};
	_float					m_fTime = {};

protected:
	D3D11_BUFFER_DESC		m_InstanceBufferDesc{};
	D3D11_SUBRESOURCE_DATA	m_InstanceSubResourceData{};

protected:
	random_device	m_RandomDevice;
	mt19937_64		m_RandomNumber;
	_float2*		m_pLifeTimes = { nullptr };
	_float*			m_pSpeeds = { nullptr };
	_float3*		m_pRotationAxes = { nullptr };

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END