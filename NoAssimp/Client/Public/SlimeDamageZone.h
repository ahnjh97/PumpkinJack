#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CNavigation;
END

BEGIN(Client)

class CSlimeDamageZone : public CGameObject
{
private:
	CSlimeDamageZone(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlimeDamageZone(const CSlimeDamageZone& rhs);
	virtual ~CSlimeDamageZone() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fTime = {};
	_float m_fDamageTime = { FLT_MAX };
	_float m_fAlpha = { 0.5f };

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSlimeDamageZone* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
