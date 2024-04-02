#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CKart : public CGameObject
{
	enum KART_COLLIDER { COLLIDER_PUSH, COLLIDER_WALL, COLLIDER_FRONT, COLLIDER_REAR, COLLIDER_END };

private:
	CKart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKart(const CKart& rhs);
	virtual ~CKart() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_vector Check_Direction(KART_COLLIDER eKartCollider);
	void Turn_Kart(_float fTimeDelta);

private:
	_bool m_bPush = { false };
	_bool m_bCollision = { false };

	_float m_fTime = {};

	_vector m_vFrontDir = {};
	_vector m_vRearDir = {};
	_vector m_vDirection = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom[COLLIDER_END] = {nullptr};

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CKart* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


