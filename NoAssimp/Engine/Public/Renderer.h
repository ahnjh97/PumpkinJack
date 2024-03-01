#pragma once

#include "Base.h"

/* 1. 화면에 그려져야할 객체들만 그려지는 순서대로 보관하는 클래스이다.*/
/* 2. 보관하고 있는 순서대로 객체들의 Draw콜(렌더함수를호출한다.)을 수행한다.*/

BEGIN(Engine)

class CRenderer final : public CBase
{
public:
	enum RENDERGROUP { RENDER_PRIORITY, RENDER_NONBLEND, RENDER_BLEND, RENDER_UI, RENDER_END };
private:
	CRenderer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CRenderer() = default;
public:
	HRESULT Initialize();
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	HRESULT Render();
	
private:
	ID3D11Device*						m_pDevice = { nullptr };
	ID3D11DeviceContext*				m_pContext = { nullptr };
	list<class CGameObject*>	m_RenderObjects[RENDER_END];

private:
	HRESULT Render_Priority();
	HRESULT Render_NonBlend();
	HRESULT Render_Blend();
	HRESULT Render_UI();

public:
	static CRenderer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;


};

END