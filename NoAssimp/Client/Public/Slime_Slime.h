#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CModel;
class CTexture;
class CShader;
END

BEGIN(Client)

class CSlime_Slime final : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		const SLIME* pState;
	}SLIME_SLIME_DESC;
private:
	CSlime_Slime(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlime_Slime(const CSlime_Slime& rhs);
	virtual ~CSlime_Slime() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void StateToAnimation();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr }; // Emit ÅØ½ºÃÄ
	const SLIME* m_pState = { };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSlime_Slime* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
