#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CTexture;
END

BEGIN(Client)

class CSlash final : public CGameObject
{
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4* pPlayerWorldMat;
		_float4 vPlayerLook;
		_float4 vPlayerRight;
		_float fWaitTime;
		_float fDuration = { 0.6f };
		WEAPON eWeapon;
		STATE eState;
		_bool bRollSecond = { false };
	}SLASH_DESC;

private:
	CSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlash(const CSlash& rhs);
	virtual ~CSlash() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_vector m_vOffset = { };

	_bool	m_bEnabled = { false };
	_bool	m_bTurn = { false };

	_uint	m_iWeaponIndex = {};
	_uint	m_iSpriteIndex = { 5 };

	_float	m_fTime = { };
	_float	m_fEndTime = { 0.6f };
	_float	m_fPlayRatio = {};
	_float	m_fWaitTime = {};
	_float	m_fSpriteTime = {};

private:
	CModel*		m_pModelCom = { nullptr };
	CShader*	m_pShaderCom = { nullptr };
	CTexture*	m_pTextureCom = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CSlash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END




