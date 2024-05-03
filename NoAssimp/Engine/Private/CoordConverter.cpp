#include "CoordConverter.h"
#include "GameInstance.h"

CCoordConverter::CCoordConverter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CCoordConverter::Initialize()
{
	_uint				iNumViewports = { 1 };
	D3D11_VIEWPORT		ViewportDesc{};

	m_pContext->RSGetViewports(&iNumViewports, &ViewportDesc);

	m_iViewportWidth = ViewportDesc.Width;
	m_iViewportHeight = ViewportDesc.Height;

	D3D11_TEXTURE2D_DESC	TextureDesc;
	ZeroMemory(&TextureDesc, sizeof(D3D11_TEXTURE2D_DESC));

	TextureDesc.Width = ViewportDesc.Width;
	TextureDesc.Height = ViewportDesc.Height;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	TextureDesc.SampleDesc.Quality = 0;
	TextureDesc.SampleDesc.Count = 1;

	TextureDesc.Usage = D3D11_USAGE_STAGING;
	TextureDesc.BindFlags = 0;
	TextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	TextureDesc.MiscFlags = 0;

	if (FAILED(m_pDevice->CreateTexture2D(&TextureDesc, nullptr, &m_pTextureHub)))
		return E_FAIL;

	return S_OK;
}

_vector CCoordConverter::Compute_WorldPos(const _float2& vViewportPos, const wstring& strZRenderTargetTag, _uint iOffset)
{
	if (FAILED(m_pGameInstance->Copy_Resource(strZRenderTargetTag, &m_pTextureHub)))
		return XMVectorZero();

	D3D11_MAPPED_SUBRESOURCE		SubResource{};

	m_pContext->Map(m_pTextureHub, 0, D3D11_MAP_READ, 0, &SubResource);

	_uint		iPixelIndex = vViewportPos.y * m_iViewportWidth + vViewportPos.x;

	_ubyte*		pAddress = ((_ubyte*)(((_float4*)SubResource.pData) + iPixelIndex));

	_float		fZ = *((_float*)(pAddress + iOffset));


	m_pContext->Unmap(m_pTextureHub, 0);

	_float3		vProjPos;

	vProjPos.x = vViewportPos.x / (m_iViewportWidth * 0.5f) - 1.f;
	vProjPos.y = vViewportPos.y / -(m_iViewportHeight * 0.5f) + 1.f;
	vProjPos.z = fZ;

	_matrix		ProjMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);
	_matrix		ViewMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	_vector		vWorldPos = XMVector3TransformCoord(XMLoadFloat3(&vProjPos), ProjMatrixInv);
	vWorldPos = XMVector3TransformCoord(vWorldPos, ViewMatrixInv);

	return vWorldPos;
}

_vector CCoordConverter::Compute_WorldPos_Float4(const _float4& vPlayerPos, const wstring& strZRenderTargetTag, _uint iOffset)
{
	if (FAILED(m_pGameInstance->Copy_Resource(strZRenderTargetTag, &m_pTextureHub)))
		return XMVectorZero();

	D3D11_MAPPED_SUBRESOURCE SubResource{};

	if (FAILED(m_pContext->Map(m_pTextureHub, 0, D3D11_MAP_READ, 0, &SubResource)))
		return _vector();

	_matrix ViewMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_VIEW);
	_matrix ProjMatrix = m_pGameInstance->Get_Transform_Matrix(CPipeLine::D3DTS_PROJ);
	_vector vPlayerViewPos = XMVector3TransformCoord(XMLoadFloat4(&vPlayerPos), ViewMatrix);
	vPlayerViewPos = XMVector3TransformCoord(vPlayerViewPos, ProjMatrix);


	_float2 vViewportPos{};
	vViewportPos.x = (XMVectorGetX(vPlayerViewPos) + 1.0f) * 0.5f * m_iViewportWidth;
	vViewportPos.y = (1.0f - XMVectorGetY(vPlayerViewPos)) * 0.5f * m_iViewportHeight;

	_uint iPixelIndex = static_cast<_uint>(vViewportPos.y) * m_iViewportWidth + static_cast<_uint>(vViewportPos.x);

	if (iPixelIndex > m_iViewportWidth * m_iViewportHeight)
		return XMLoadFloat4(&vPlayerPos);

	_ubyte* pAddress = ((_ubyte*)(((_float4*)SubResource.pData) + iPixelIndex));

	_float fZ = *((_float*)(pAddress + iOffset));

	m_pContext->Unmap(m_pTextureHub, 0);

	_float3 vProjPos;
	vProjPos.x = vViewportPos.x / (m_iViewportWidth * 0.5f) - 1.f;
	vProjPos.y = vViewportPos.y / -(m_iViewportHeight * 0.5f) + 1.f;
	vProjPos.z = fZ;

	_matrix ProjMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);
	_matrix ViewMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	_vector vWorldPos = XMVector3TransformCoord(XMLoadFloat3(&vProjPos), ProjMatrixInv);
	vWorldPos = XMVector3TransformCoord(vWorldPos, ViewMatrixInv);

	return vWorldPos;
}

CCoordConverter* CCoordConverter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCoordConverter* pInstance = new CCoordConverter(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed to Created : CCoordConverter"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCoordConverter::Free()
{
	Safe_Release(m_pTextureHub);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
