#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CKnife : public CGameObject
{
private:
	CKnife(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKnife(const CKnife& rhs);
	virtual ~CKnife() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	_float m_fTime = {};
	_float m_fAlpha = {1.f};

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CKnife* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
