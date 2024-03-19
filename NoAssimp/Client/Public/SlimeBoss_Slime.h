#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CSlimeBoss_Slime final : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		const SLIMEBOSS* pState;
	}SLIMEBOSS_SLIME_DESC;
private:
	CSlimeBoss_Slime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlimeBoss_Slime(const CSlimeBoss_Slime& rhs);
	virtual ~CSlimeBoss_Slime() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	const SLIMEBOSS* m_pState = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSlimeBoss_Slime* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
