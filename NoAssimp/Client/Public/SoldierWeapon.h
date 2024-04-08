#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CBone;
END

BEGIN(Client)

class CSoldierWeapon final : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		CBone* pSocket = { nullptr };
		const SOLDIER* pState;
		const _uint* pDamage;
	}SOLDIER_WEAPON_DESC;

private:
	CSoldierWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSoldierWeapon(const CSoldierWeapon& rhs);
	virtual ~CSoldierWeapon() = default;

public:
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
	void Check_PlayerCol();

private:
	_bool m_bActivated = { false };

private:
	CBone* m_pSocket = { nullptr };
	const SOLDIER* m_pState = {};
	const _uint* m_pDamage = {};
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components();

public:
	static CSoldierWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


