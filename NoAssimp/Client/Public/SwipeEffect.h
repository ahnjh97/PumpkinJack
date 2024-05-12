#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CTexture;
class CShader;
END

BEGIN(Client)

class CSwipeEffect final : public CGameObject
{
	enum TEX_SWIPE { TEX_SLASH, TEX_NOISE, TEX_END };
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		_vector vSlimeBossRight;
	}SWIPEEFFECT_DESC;

private:
	CSwipeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSwipeEffect(const CSwipeEffect& rhs);
	virtual ~CSwipeEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Check_PlayerCol();

private:
	_bool	m_bEnabled = { false };
	_bool	m_bPlaySwipeSound = { true };

	_uint m_iSpriteIndex = {};
	_uint m_iWeaponIndex = {};

	_float	m_fTime = { };
	_float	m_fEndTime = { };
	_float	m_fWaitTime = {};
	_float	m_fPlayRatio = {};
	_float	m_fSpriteTime = {};

private:
	CModel* m_pModelCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr, nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CSwipeEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END



