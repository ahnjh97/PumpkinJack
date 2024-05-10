#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CSoldierSlash final : public CPart
{
private:
	CSoldierSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSoldierSlash(const CSoldierSlash& rhs);
	virtual ~CSoldierSlash() = default;

public:
	void Reset_Time() { m_fTime = 0; m_fSpriteTime = 0.f; m_iSpriteIndex = 0; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool	m_bEnabled = { false };

	_uint	m_iSpriteIndex = { 5 };

	_float	m_fTime = { 100.f };
	_float	m_fEndTime = { 1.2f };
	_float	m_fPlayRatio = {};
	_float	m_fWaitTime = {};
	_float	m_fSpriteTime = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	const SOLDIER* m_pState = {};

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CSoldierSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END