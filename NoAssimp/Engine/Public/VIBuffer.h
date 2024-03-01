#pragma once

#include "Component.h"

/* 각 형태를 구성하기위한 정점 인덱스를 보유하는 개겣들의 부모다. */

BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer(const CVIBuffer& rhs);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render();

public:
	HRESULT Bind_Buffers();

protected:
	ID3D11Buffer*				m_pVB = { nullptr };
	ID3D11Buffer*				m_pIB = { nullptr };

protected:
	D3D11_BUFFER_DESC			m_BufferDesc;
	D3D11_SUBRESOURCE_DATA		m_InitialData;
	_uint						m_iVertexStride = { 0 };
	_uint						m_iIndexStride = { 0 };
	_uint						m_iNumVertices = { 0 };
	_uint						m_iNumIndices = { 0 };
	_uint						m_iNumVertexBuffers = { 0 };
	DXGI_FORMAT					m_eIndexFormat = {  };
	D3D11_PRIMITIVE_TOPOLOGY	m_ePrimitiveTopology = { };

protected:
	HRESULT Create_Buffer(_Out_ ID3D11Buffer** ppBuffer);

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END