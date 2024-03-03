#include "..\Public\VIBuffer.h"

CVIBuffer::CVIBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent{ pDevice, pContext }
{
}

CVIBuffer::CVIBuffer(const CVIBuffer & rhs) //헤헤호호
	: CComponent{ rhs }
	, m_pVB{ rhs.m_pVB }
	, m_pIB{ rhs.m_pIB }
	, m_BufferDesc{ rhs.m_BufferDesc }
	, m_InitialData{ rhs.m_InitialData }
	, m_iVertexStride{ rhs.m_iVertexStride }
	, m_iIndexStride { rhs.m_iIndexStride}
	, m_iNumVertices { rhs.m_iNumVertices }
	, m_iNumIndices { rhs.m_iNumIndices }
	, m_iNumVertexBuffers { rhs.m_iNumVertexBuffers }
	, m_eIndexFormat {rhs.m_eIndexFormat}
	, m_ePrimitiveTopology {rhs.m_ePrimitiveTopology}
{
	Safe_AddRef(m_pVB);
	Safe_AddRef(m_pIB);
}

HRESULT CVIBuffer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CVIBuffer::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CVIBuffer::Render()
{
	if (nullptr == m_pContext)
		return E_FAIL;

	/* 인덱스버퍼의 값을 이용하여 정점 버퍼의 정점들을 그려낸다. */
	m_pContext->DrawIndexed(m_iNumIndices, 0, 0);

	return S_OK;
}

HRESULT CVIBuffer::Bind_Buffers()
{
	ID3D11Buffer*		pVertexBuffers[] = {
		m_pVB, 
	
	};

	_uint				iVertexStrides[] = {
		m_iVertexStride
	};

	_uint				iOffsets[] = {
		0
	};

	/* 정점버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBuffers, iVertexStrides, iOffsets);

	/* 인덱스버퍼들을 장치에 바인딩한다. */
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);

	/* 그릴때 어떤 형태로 정점들을 이어 그릴건지. */
	m_pContext->IASetPrimitiveTopology(m_ePrimitiveTopology);



	return S_OK;
}

_bool CVIBuffer::bRayTriangleIntersect(_float3 _vRayPos, _float3 _vRayDir, _float3 _v0, _float3 _v1, _float3 _v2, _float& _fDis) const
{
	return false;
	//XMVECTOR o = XMLoadFloat3(&_vRayPos);
	//XMVECTOR d = XMVector3Normalize(XMLoadFloat3(&_vRayDir));
	//XMVECTOR v0 = XMLoadFloat3(&_v0);
	//XMVECTOR v1 = XMLoadFloat3(&_v1);
	//XMVECTOR v2 = XMLoadFloat3(&_v2);

	//XMVECTOR e1 = v1 - v0;
	//XMVECTOR e2 = v2 - v0;
	//XMVECTOR h = XMVector3Cross(d, e2);
	//float a = XMVectorGetX(XMVector3Dot(e1, h));

	//if (a > -FLT_EPSILON && a < FLT_EPSILON)
	//	return false;

	//float f = 1.0f / a;
	//XMVECTOR s = o - v0;
	//float u = f * XMVectorGetX(XMVector3Dot(s, h));

	//if (u < 0.0f || u > 1.0f)
	//	return false;

	//XMVECTOR q = XMVector3Cross(s, e1);
	//float v = f * XMVectorGetX(XMVector3Dot(d, q));

	//if (v < 0.0f || u + v > 1.0f)
	//	return false;

	//_fDis = f * XMVectorGetX(XMVector3Dot(e2, q));

	//return _fDis > FLT_EPSILON;
}



HRESULT CVIBuffer::Create_Buffer(ID3D11Buffer ** ppBuffer)
{
	return m_pDevice->CreateBuffer(&m_BufferDesc, &m_InitialData, ppBuffer);
}

void CVIBuffer::Free()
{
	__super::Free();

	Safe_Release(m_pVB);
	Safe_Release(m_pIB);
}
