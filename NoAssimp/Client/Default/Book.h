#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CInstance_Model;
class CTexture;
class CShader;
END

BEGIN(Client)

class CBook final : public CGameObject
{
private:
	CBook(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBook(const CBook& rhs);
	virtual ~CBook() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CInstance_Model* m_pInstanceModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components();
	HRESULT Bind_ShaderResources();

public:
	static CBook* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END