#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CWizard final : public CGameObject
{

private:
	CWizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWizard(const CWizard& rhs);
	virtual ~CWizard() = default;

public:
	INFO_WIZARD Get_WizardInfo();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Get_PlayerPtr();
	void Check_WizardHp();
	void Link_State(_float fTimeDelta);
	void StateToAnimation();
	void TurnTowards_Player();

	void Summon_LevitationTornado();

	void Cast_Spell();
	void Lightning();
	void FireBall();
	void Tornado();
	void HoleWall();
	void TeslaTower();
	void DestroyTeslaTowers();
	void DestroyElecArena();

	_bool AllTowersDestroyed();

	_matrix AttachColliderToBone();

private:
	WIZARD m_eState = { WIZARD_AIR_IDLE };
	WIZARD m_ePreState = { WIZARD_AIR_IDLE };
	SPELL m_eSpell = {};

	_bool m_bTowerSpawn = { false };
	_bool m_bAllTowersDead = { false };
	_bool m_bChangePhase = { false };
	_bool m_bCastSpell = { true };
	_bool m_bWait = { false };
	_bool m_bSummonLevitationTornado = { false };
	_bool m_bDestroyElecArena = { false };

	_int m_iWizardPhase = {};
	_int m_iAttackCount = {};
	_int m_iAttackFinishCount = { };
	_int m_iPreHp = {};

	_float m_fAlpha = { 1.f };
	_float m_fTime = {};
	_float m_fIdleTime = {};
	_float m_fChargeTime = {};
	_float m_fWaitTime = {};
	_float m_fPhaseChangeTime = { 100 };

	_float3 m_vTeslaTowerPos[6] = { _float3(483, 0, 483), _float3(695, 0, 535), _float3(515, 0, 695), 
		_float3(440, 0, 573), _float3(640, 0, 455), _float3(666, 0, 673) };

	vector <_int> m_vecSpellThresholds = { 6, 4, 4 };
	vector<_float> m_vecIdleDurations = { 1.f, 1.2f, 1.4f, 1.6f, 1.8f, 2.f };

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CGameObject* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMatrix = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	void	SetUp_WizardAnimSpeed();

public:
	static CWizard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
