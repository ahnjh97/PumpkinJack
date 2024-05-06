#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CSoulEffect final : public CGameObject
{

private:
	CSoulEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSoulEffect(const CSoulEffect& rhs);
	virtual ~CSoulEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fTime = {};
	_float m_fScale = { 1.2f };
	_float m_fEndTime = { 1.5f };
	_float m_fPlayRatio = {};
	_float m_fAccSpeed = {};
	_float m_fAccScale = { 1.f };

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CSoulEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

