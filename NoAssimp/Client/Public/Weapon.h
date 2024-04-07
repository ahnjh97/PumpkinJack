#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CBone;
class CModel;
class CShader;
END

BEGIN(Client)

class CWeapon final : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		CBone* pSocket = { nullptr };
		const STATE* pState;
		const WEAPON* pWeapon;
		const _uint* pDamage;
	}WEAPON_DESC;

private:
	CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon(const CWeapon& rhs);
	virtual ~CWeapon() = default;

public:
	_float4 Get_Pos() { return _float4(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, 1); }
	void Reset_Damagable();
	void Activate_WeaponCollider() { m_bActivated = true; }
	void DeActivate_WeaponCollider() { m_bActivated = false; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool Check_CorrectWeapon();

	void Check_SlimeCol();
	void Check_SlimeBossRoomPillarCol();
	void Check_SoldierCol();

private:
	_bool m_bCorrectWeapon = { false };
	_bool m_bActivated = { false };

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CBone* m_pSocket = { nullptr };
	const WEAPON* m_pWeapon = {};
	const STATE* m_pState = {};
	const _uint* m_pDamage = {};
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
