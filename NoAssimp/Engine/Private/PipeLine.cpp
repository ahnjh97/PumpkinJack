#include "..\Public\PipeLine.h"



CPipeLine::CPipeLine()
{
}

HRESULT CPipeLine::Initialize()
{
	for (size_t i = 0; i < D3DTS_END; i++)
	{
		XMStoreFloat4x4(&m_TransformMatrices[i], XMMatrixIdentity());
		XMStoreFloat4x4(&m_TransformInverseMatrices[i], XMMatrixIdentity());
	}

	m_vCamPosition = _float4(0.f, 0.f, 0.f, 1.f);

	return S_OK;
}

void CPipeLine::Tick()
{
	for (size_t i = 0; i < D3DTS_END; i++)
	{
		XMStoreFloat4x4(&m_TransformInverseMatrices[i], XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_TransformMatrices[i])));
	}

	memcpy(&m_vCamPosition, &m_TransformInverseMatrices[D3DTS_VIEW].m[3][0], sizeof(_float4));
}

CPipeLine * CPipeLine::Create()
{
	CPipeLine*		pInstance = new CPipeLine();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CPipeLine"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPipeLine::Free()
{
}
