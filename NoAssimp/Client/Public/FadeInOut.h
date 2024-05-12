#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CVIBuffer_Rect;
END

BEGIN(Client)
class CFadeInOut final : public CGameObject
{
private:
	CFadeInOut(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CFadeInOut(const CFadeInOut& rhs);
	virtual ~CFadeInOut() = default;

public:
	void Reset_Time() { m_fTime = 0.f; }

	_float Get_PlayRatio() { return m_fPlayRatio; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float		m_fX, m_fY, m_fZ, m_fSizeX, m_fSizeY;
	_float4x4	m_WorldMatrix, m_ViewMatrix, m_ProjMatrix;

	_bool		m_bFadeIn = { true };

	_float		m_fTime = { };
	_float		m_fEndTime = { };
	_float		m_fPlayRatio = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CFadeInOut* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

