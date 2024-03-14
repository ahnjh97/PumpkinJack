#include "Part.h"

CPart::CPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject{pDevice, pContext}
{
}

CPart::CPart(const CPart& rhs)
	:CGameObject{ rhs }
{
}

HRESULT CPart::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPart::Initialize(void* pArg)
{
	if (nullptr != pArg)
	{
		PART_DESC* pPartDesc = (PART_DESC*)pArg;

		m_pParentMatrix = pPartDesc->pParentMatrix;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CPart::Tick(_float fTimeDelta)
{
}

void CPart::Late_Tick(_float fTimeDelta)
{
}

HRESULT CPart::Render()
{
	return S_OK;
}

void CPart::Free()
{
	__super::Free();
}
