#include "..\Public\Texture.h"
#include "Shader.h"

CTexture::CTexture(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{

}

CTexture::CTexture(const CTexture & rhs)
	: CComponent{ rhs }
	, m_Textures{ rhs.m_Textures }
	, m_iNumTextures{ rhs.m_iNumTextures }
{
	for (auto& pSRV : m_Textures)
		Safe_AddRef(pSRV);
}

HRESULT CTexture::Initialize_Prototype(const wstring & strTextureFilePath, _uint iNumTextures)
{
	_tchar		szEXT[MAX_PATH] = TEXT("");

	/* 1번째 인자에 해당하는 문자열을 종류에 분해해준다. */

	/* ..\Bin\정의훈\141\Framework\Client\Bin\Resources\Textures\Test.jpg */
	_wsplitpath_s(strTextureFilePath.c_str(), nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

	HRESULT hr = { 0 };

	_tchar		szFileName[MAX_PATH] = { TEXT("") };	

	ID3D11ShaderResourceView*			pSRV = { nullptr };
	
	/* 뜯어낸 확장자가 dds였다면. */
	if (false == lstrcmp(szEXT, TEXT(".dds")))
	{
		for (size_t i = 0; i < iNumTextures; i++)
		{
			wsprintf(szFileName, strTextureFilePath.c_str(), i);
			
			hr = CreateDDSTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures.push_back(pSRV);
		}

	}
	else if (false == lstrcmp(szEXT, TEXT(".tga")))
	{
		MSG_BOX(TEXT("TGA안됨ㅋ"));
	}
	else
	{
		for (size_t i = 0; i < iNumTextures; i++)
		{
			wsprintf(szFileName, strTextureFilePath.c_str(), i);

			hr = CreateWICTextureFromFile(m_pDevice, szFileName, nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Textures.push_back(pSRV);
		}
	}

	m_iNumTextures = iNumTextures;

	return S_OK;
}

HRESULT CTexture::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShaderResource(CShader * pShader, const _char * pConstantName, _uint iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_Texture(pConstantName, m_Textures[iTextureIndex]);	
}

CTexture * CTexture::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strTextureFilePath, _uint iNumTexture)
{
	CTexture*		pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strTextureFilePath, iNumTexture)))
	{
		MSG_BOX(TEXT("Failed To Created : CTexture"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CTexture::Clone(void * pArg)
{
	CTexture*		pInstance = new CTexture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTexture"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTexture::Free()
{
	__super::Free();

	for (auto& pSRV : m_Textures)
		Safe_Release(pSRV);

	m_Textures.clear();
}
