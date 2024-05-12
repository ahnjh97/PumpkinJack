#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPart;
END

BEGIN(Client)

class CSlimeBoss final : public CGameObject
{
private:
	CSlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlimeBoss(const CSlimeBoss& rhs);
	virtual ~CSlimeBoss() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Link_Animation(_float fTimeDelta);
	void StateToAnimation();
	void TurnTowards_Player(_float fTimeDelta);
	void KnifeSpawn();
	void PillarSpawn();
	void PortalSpawn();
	void SlimeSpawn();
	void SlimeDamageZoneSpawn(_uint iNum);
	void DeathEffect();
	void Add_SwipeEffect();
	void Add_ShockWave(_int iShockWaveCount);
	void Add_ChargeEffect();
	void DestroyChargeEffect();
	void Add_Cauldron();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	map<const wstring, CPart*> m_Parts;
	class CPlayer* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMatrix = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

	SLIMEBOSS m_eState = { SLIMEBOSS_SPAWN };
	SLIMEBOSS m_ePreState = { SLIMEBOSS_SPAWN };

	_bool m_bPillarSpawn = { false };
	_bool m_bPortalSpawn = { false };
	_bool m_bSlimeSpawn = { false };
	_bool m_bSlimeDamageZoneSpawn = { false };
	_bool m_bTurnToPlayer = { false };
	_bool m_bAddSwipeEffect = { false };
	_bool m_bAddShockWave = { false };
	_bool m_bPlaySpawnSound = { true };
	_bool m_bPlaySpinKnifeSound = { true };
	_bool m_bDestroyCharge = { true };

	_int m_iCurPattern = { -1};
	_int m_iLightningCount = {};

	_float m_fTime = {};

	_float4 m_vLightningPos = {};

	vector<SLIMEBOSS> m_vecSlimeBossPattern = { SLIMEBOSS_FORK, SLIMEBOSS_INVOKE_BEGIN, SLIMEBOSS_INVOKE
		, SLIMEBOSS_FORK, SLIMEBOSS_LIGHTNING, SLIMEBOSS_FORK, SLIMEBOSS_PILLAR_SPAWN, SLIMEBOSS_MEGASPAWN };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_PartObjects(LEVEL eLevel);

public:
	static CSlimeBoss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
