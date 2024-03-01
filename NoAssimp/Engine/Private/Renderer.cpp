#include "..\Public\Renderer.h"
#include "GameObject.h"



CRenderer::CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CRenderer::Initialize()
{
	return S_OK;
}

HRESULT CRenderer::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject * pRenderObject)
{
	if (eRenderGroup >= RENDER_END)
		return E_FAIL;

	m_RenderObjects[eRenderGroup].push_back(pRenderObject);

	Safe_AddRef(pRenderObject);

	return S_OK;
}

HRESULT CRenderer::Render()
{
	if (FAILED(Render_Priority()))
		return E_FAIL;
	if (FAILED(Render_NonBlend()))
		return E_FAIL;
	if (FAILED(Render_Blend()))
		return E_FAIL;
	if (FAILED(Render_UI()))
		return E_FAIL;

	return S_OK;
}

HRESULT CRenderer::Render_Priority()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_PRIORITY])
	{
		if(nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_PRIORITY].clear();

	return S_OK;
}

HRESULT CRenderer::Render_NonBlend()
{
	for (auto& pRenderObject : m_RenderObjects[RENDER_NONBLEND])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_NONBLEND].clear();

	return S_OK;
}
//
//_bool Compare(CGameObject* pSour, CGameObject* pDest)
//{
//	return ((CBlendObject*)pSour)->Get_ViewZ() > ((CBlendObject*)pDest)->Get_ViewZ();
//}

HRESULT CRenderer::Render_Blend()
{
	//m_RenderObjects[RENDER_BLEND].sort([](CGameObject* pSour, CGameObject* pDest)->_bool
	//{
	//	return ((CBlendObject*)pSour)->Get_ViewZ() > ((CBlendObject*)pDest)->Get_ViewZ();
	//});

	for (auto& pRenderObject : m_RenderObjects[RENDER_BLEND])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_BLEND].clear();

	return S_OK;
}

HRESULT CRenderer::Render_UI()
{


	for (auto& pRenderObject : m_RenderObjects[RENDER_UI])
	{
		if (nullptr != pRenderObject)
			pRenderObject->Render();	
		Safe_Release(pRenderObject);
	}
	m_RenderObjects[RENDER_UI].clear();

	return S_OK;
}

CRenderer * CRenderer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRenderer*		pInstance = new CRenderer(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Created : CRenderer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CRenderer::Free()
{
	__super::Free();

	for (auto& RenderList : m_RenderObjects)
	{
		for (auto& pRenderObject : RenderList)		
			Safe_Release(pRenderObject);
		RenderList.clear();		
	}

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
