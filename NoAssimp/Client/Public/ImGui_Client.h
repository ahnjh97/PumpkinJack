#pragma once
#include "Client_Defines.h"
#include "MyImGui.h"

BEGIN(Client)
class CImGui_Client : public CMyImGui
{
private:
	CImGui_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CImGui_Client() = default;

public:
	HRESULT				Initialize();
	virtual void		Tick(_float fTimeDelta)		override;
	virtual HRESULT		Render()					override;
	void				Settings(_float fTimeDelta);
	void				OnKeyInput();
	void				ConvertToString();
	void				WizardDebug();

private:
	_bool				bCallNewFrame = { false };
	vector<string>		m_vecState = { "IDLE", "RUN", "JUMP", "ROLL", "ATTACK1", "ATTACK2", "ATTACK3", "AIR_ATTACK","AIR_ATTACK_END", "ROLL_ATTACK", "HIGHFALL", "PUSHING", "STATE_END" };
	vector<string>		m_vecWeapon = { "WPN_SPEAR", "WPN_RIFLE", "WPN_SCYTHE", "WPN_END" };
	vector<string>		m_vecWeaponName = { "Spear", "Rifle", "Scythe", "WPN_END" };
	_uint*				m_pCurLevelIndex = { nullptr };

	vector<string>		m_vecWizardState = { "WIZARD_ATTACK", "WIZARD_CHARGE", "WIZARD_AIR_IDLE", "WIZARD_DIE", "WIZARD_DIE2", "WIZARD_HIT", "WIZARD_IDLE", "WIZARD_WEAK", "WIZARD_END" };
public:
	static CImGui_Client*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void			Free();
};

END