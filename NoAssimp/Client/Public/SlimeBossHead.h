#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CNavigation;
END

BEGIN(Client)

class CSlimeBossHead : public CGameObject
{
	enum HEADSTATE { HEAD_IDLE, HEAD_TURN, HEAD_DASH, HEAD_END };

private:
	CSlimeBossHead(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlimeBossHead(const CSlimeBossHead& rhs);
	virtual ~CSlimeBossHead() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void StateToAction(_float fTimeDelta);

private:
	HEADSTATE m_eState = {};
	HEADSTATE m_ePreState = {};

	_bool m_bPreInSlimeBossRoom = { false };
	_bool m_bInSlimeBossRoom = { false };
	_bool m_bCollision = { false };

	_float m_fTime = {};
	_float m_fDamageTime = { 1.f };
	_float m_fIdleTime = { 0.f };
	_float m_fShakeTime = { 0.f };
	_float m_fShakeDir = { -1.f };
	_float m_fDashTime = { 0.f };

	_float4 m_vDashDir = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };
	CNavigation* m_pNavigationCom = { nullptr };
	class CPlayer* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSlimeBossHead* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
