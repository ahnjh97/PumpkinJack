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
	void PillarSpawn();
	void PortalSpawn();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	map<const wstring, CPart*> m_Parts;
	class CPlayer* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMatrix = { nullptr };

	SLIMEBOSS m_eState = { SLIMEBOSS_SPAWN };
	SLIMEBOSS m_ePreState = { SLIMEBOSS_SPAWN };
	_bool m_bPillarSpawn = { false };
	_bool m_bPortalSpawn = { false };
	_int m_iHp = { 100 };
	_int m_iCurPattern = { -1};
	_int m_iLightningCount = {};
	_float m_fPreAngle = {};
	_float4 m_vLightningPos = {};

	vector<SLIMEBOSS> m_vecSlimeBossPattern = { SLIMEBOSS_FORK, SLIMEBOSS_INVOKE_BEGIN, SLIMEBOSS_INVOKE
		, SLIMEBOSS_FORK, SLIMEBOSS_LIGHTNING, SLIMEBOSS_FORK, SLIMEBOSS_PILLAR_SPAWN, SLIMEBOSS_IDLE, SLIMEBOSS_MEGASPAWN };

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
