#pragma once
#include "VIBuffer_Instance.h"
#include <fstream>

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance_Mesh final : public CVIBuffer_Instance
{

private:
	CVIBuffer_Instance_Mesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileStream);
	CVIBuffer_Instance_Mesh(const CVIBuffer_Instance_Mesh& rhs);
	virtual ~CVIBuffer_Instance_Mesh() = default;

public:
	_uint Get_MaterialIndex() const { return m_iMaterialIndex; }

public:
	virtual HRESULT Initialize_Prototype(ifstream& fileStream, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual _float4 Compute_RandPosition() override;

public:
	void TurnAndSpread(_float fTimeDelta, _float fRotationSpeed);

private:
	_char		m_szName[MAX_PATH] = { "" };
	_uint		m_iMaterialIndex = { 0 };
	ifstream&	m_InputFile;
	_uint		m_iFaces = { };
	_float3*	m_pRotationAxes = { nullptr };

public:
	static CVIBuffer_Instance_Mesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, ifstream& fileStream, const CVIBuffer_Instance::INSTANCE_DESC& InstanceDesc);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END