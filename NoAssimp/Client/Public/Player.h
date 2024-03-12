#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "Camera_Jack.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CPlayer final : public CGameObject
{
private:
	CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayer(const CPlayer& rhs);
	virtual ~CPlayer() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void OnKeyInput(_float fTimeDelta);
	void StateToAnimation();
	void Update_CamPos();
	_float Calculate_Angle();
	void Turn_Body(_float fAngle);

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCamera_Jack* m_pCamera = { nullptr };

	PLAYER_STATE m_eState = { PLAYER_END };
	WEAPON m_eWeapon = { WPN_SPEAR };
	_uint m_iLastAttack = { 0 };

	_bool m_bIsJumping = { false };

	_float m_fTempJumpTime = { 0.f };
	_float3	m_vPreTargetDir = {};
	_float3	m_vTargetDir = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CPlayer* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

