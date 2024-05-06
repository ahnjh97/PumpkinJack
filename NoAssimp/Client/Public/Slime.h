#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPart;
END

BEGIN(Client)

class CSlime : public CGameObject
{
	enum COLLIDER { DEFAULT, DAMAGE_RANGE, HIT, COLLIDER_END };
private:
	CSlime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlime(const CSlime& rhs);
	virtual ~CSlime() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void StateToAnimation();
	_bool Check_DeathTrigger();
	void Check_SlimeCol(_float fTimeDelta);
	void Check_PillarCol(_float fTimeDelta);
	void Check_SlimeBossCol(_float fTimeDelta);

	void DeathEffect();
	void AttackEffect();

private:
	SLIME m_eState = { SLIME_MOVE };

	_bool m_bAttackTrigger = { false };
	_bool m_bDamagedPlayer = { false };

	_float m_fTime = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	map<const wstring, CPart*> m_Parts;
	CCollider* m_pColliderCom[COLLIDER_END] = {nullptr};
	class CPlayer* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };
	CCollider* m_pPlayerCollider = { nullptr };
	CCollider* m_pSlimeBossCollider = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_PartObjects(LEVEL eLevel);

public:
	static CSlime* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
