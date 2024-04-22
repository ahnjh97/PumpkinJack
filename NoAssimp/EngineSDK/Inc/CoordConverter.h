#pragma once
#include "Base.h"

/* 내가 특정 Viewport상의 위치 xy를 던져주면 해당위치에 있는 픽셀의 월드위치를 계산해준다. */
/* z값을 얻어오기위해서는 깊이 렌더타겟으로 부터 얻어온다. */

BEGIN(Engine)

class CCoordConverter final : public CBase
{
private:
	CCoordConverter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCoordConverter() = default;

public:
	HRESULT Initialize();

public:
	_vector Compute_WorldPos(const _float2& vViewportPos, const wstring& strZRenderTargetTag, _uint iOffset);
	_vector Compute_WorldPos_Float4(const _float4& vPlayerPos, const wstring& strZRenderTargetTag, _uint iOffset);

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	_uint								m_iViewportWidth = { 0 };
	_uint								m_iViewportHeight = { 0 };

	/* 렌더타겟용으로 만들어진 텍스쳐로부터 직접 값을 얻어올 수 없기때문에 .*/
	/*내가 정보를 얻어오고싶은 렌더타겟의 정보를 복사받아놓기위한 텍스쳐이다. */
	ID3D11Texture2D* m_pTextureHub = { nullptr };

public:
	static CCoordConverter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END