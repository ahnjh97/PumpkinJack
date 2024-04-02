#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CRail : public CGameObject
{
private:
	CRail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CRail(const CRail& rhs);
	virtual ~CRail() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	_int Get_ColliderIndex(CCollider* pCollider);
	_vector Compute_Direction(_int iColliderIndex);

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = {nullptr};
	CCollider* m_pColliderCom[7] = {nullptr};

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CRail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


