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
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

	SLIMEBOSS m_eState = { SLIMEBOSS_END };

	map<const wstring, CPart*> m_Parts;

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
