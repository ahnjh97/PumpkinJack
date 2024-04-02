#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CCrateBox : public CGameObject
{
private:
	CCrateBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCrateBox(const CCrateBox& rhs);
	virtual ~CCrateBox() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CCrateBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END


