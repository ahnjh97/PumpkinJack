#include "stdafx.h"
#include "LifeBar.h"

CLifeBar::CLifeBar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{pDevice, pContext}
{
}

CLifeBar::CLifeBar(const CLifeBar& rhs)
{
}

HRESULT CLifeBar::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CLifeBar::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

void CLifeBar::Tick(_float fTimeDelta)
{
}

void CLifeBar::Late_Tick(_float fTimeDelta)
{
}

HRESULT CLifeBar::Render()
{
	return E_NOTIMPL;
}

HRESULT CLifeBar::Add_Components()
{
	return E_NOTIMPL;
}

HRESULT CLifeBar::Bind_ShaderResources()
{
	return E_NOTIMPL;
}



CGameObject* CLifeBar::Clone(void* pArg)
{
	return nullptr;
}

void CLifeBar::Free()
{
}
