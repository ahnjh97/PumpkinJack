#pragma once

#include "VIBuffer_Instance.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Point final : public CVIBuffer_Instance
{

private:
	CVIBuffer_Instance_Point(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instance_Point(const CVIBuffer_Instance_Point& rhs);
	virtual ~CVIBuffer_Instance_Point() = default;

public:
	virtual HRESULT Initialize_Prototype(const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual _float4 Compute_RandPosition() override;


public:
	static CVIBuffer_Instance_Point* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END