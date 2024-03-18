#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CBackGround final : public CGameObject
{
private:
	CBackGround(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _bool bLoadingScreen);
	CBackGround(const CBackGround& rhs);
	virtual ~CBackGround() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*				m_pShaderCom = { nullptr };
	CTexture*				m_pTextureCom = { nullptr };
	CVIBuffer_Rect*			m_pVIBufferCom = { nullptr };

private:
	_float					m_fX, m_fY, m_fZ, m_fSizeX, m_fSizeY;
	_float4x4				m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;
	_bool					m_bLoadingScreen = { false };
	_uint					m_iTextureIndex = {};
	_float					m_fTime = {};

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CBackGround* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _bool bLoadingScreen = false);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END