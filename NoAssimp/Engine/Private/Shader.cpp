#include "..\Public\Shader.h"

CShader::CShader(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent{ pDevice, pContext }
{
}

CShader::CShader(const CShader & rhs)
	: CComponent{ rhs }
	, m_pEffect { rhs.m_pEffect }
	, m_InputLayouts { rhs.m_InputLayouts }
{
	Safe_AddRef(m_pEffect);

	for (auto& pInputLayout : m_InputLayouts)
		Safe_AddRef(pInputLayout);
}

HRESULT CShader::Initialize_Prototype(const wstring & strShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	_uint		iHlslFlag = { 0 };
#ifdef _DEBUG
	iHlslFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else	
	iHlslFlag = D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif // 
	if (FAILED(D3DX11CompileEffectFromFile(strShaderFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pDevice, &m_pEffect, nullptr)))
		return E_FAIL;

	ID3DX11EffectTechnique*		pTechnique = m_pEffect->GetTechniqueByIndex(0);
	if (nullptr == pTechnique)
		return E_FAIL;

	D3DX11_TECHNIQUE_DESC		TechniqueDesc{};
	pTechnique->GetDesc(&TechniqueDesc);

	for (_uint i = 0; i < TechniqueDesc.Passes; ++i)
	{
		ID3DX11EffectPass*		pPass = pTechnique->GetPassByIndex(i);
		
		D3DX11_PASS_DESC		PassDesc{};
		pPass->GetDesc(&PassDesc);

		ID3D11InputLayout*		pInputLayout = { nullptr };
		
		///* D3D11_INPUT_ELEMENT_DESC	: 정점의 멤버변수 하나의 정보를 표현한다. */
		

		if (FAILED(m_pDevice->CreateInputLayout(/* 내 정점의 구성정보*/pElements,
			iNumElements, 
			PassDesc.pIAInputSignature,
			PassDesc.IAInputSignatureSize,
			&pInputLayout)))
			return E_FAIL;

		m_InputLayouts.push_back(pInputLayout);
	}
	return S_OK;
}

HRESULT CShader::Initialize(void * pArg)
{
	return S_OK;
}

HRESULT CShader::Begin(_uint iPassIndex)
{
	if (iPassIndex >= m_InputLayouts.size())
		return E_FAIL;

	ID3DX11EffectTechnique*		pTechnique = m_pEffect->GetTechniqueByIndex(0);
	if (nullptr == pTechnique)
		return E_FAIL;

	ID3DX11EffectPass*			pPass = pTechnique->GetPassByIndex(iPassIndex);
	if (nullptr == pPass)
		return E_FAIL;	

	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex]);

	pPass->Apply(0, m_pContext);

	return S_OK;
}

HRESULT CShader::Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix)
{
	/* 타입의 구분없이 pConstantName이름을 가진 전역변수에 기능을 할 수 있는 컴객체를 얻어온다. */
	ID3DX11EffectVariable*	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	/* 실제 얻어온 전역변수 객체의 타입을 명시하여 변환한다. */
	ID3DX11EffectMatrixVariable*	pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrix)
		return E_FAIL;

	return pMatrixVariable->SetMatrix((_float*)pMatrix);	
}

HRESULT CShader::Bind_Texture(const _char* pConstantName, ID3D11ShaderResourceView* pSRV)
{
	/* 타입의 구분없이 pConstantName이름을 가진 전역변수에 기능을 할 수 있는 컴객체를 얻어온다. */
	ID3DX11EffectVariable*	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	/* 실제 얻어온 전역변수 객체의 타입을 명시하여 변환한다. */
	ID3DX11EffectShaderResourceVariable*	pSRVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVariable)
		return E_FAIL;

	return pSRVariable->SetResource(pSRV);
}


CShader * CShader::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, _uint iNumElements)
{
	CShader*		pInstance = new CShader(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(strShaderFilePath, pElements, iNumElements)))
	{
		MSG_BOX(TEXT("Failed To Created : CShader"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CShader::Clone(void * pArg)
{
	CShader*		pInstance = new CShader(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CShader"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShader::Free()
{
	__super::Free();

	Safe_Release(m_pEffect);

	for (auto& pInputLayout : m_InputLayouts)
		Safe_Release(pInputLayout);

	m_InputLayouts.clear();

}
