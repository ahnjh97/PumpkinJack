#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CTongMap_SlimeBoss final : public CGameObject
{
private:
	enum SHADER { LIGHTED, ALPHA, SLIMEBOSSROOM, NO_LIGHT, SHADER_END };
	enum TONGMAP_TEX { TEX_ALPHA, TEX_WATER, TEX_CAUSTIC, TEX_END };

private:
	CTongMap_SlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTongMap_SlimeBoss(const CTongMap_SlimeBoss& rhs);
	virtual ~CTongMap_SlimeBoss() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float m_fOffset_Y = {};

	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom[SHADER_END] = {};
	CTexture* m_pTextureCom[TEX_END] = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CTongMap_SlimeBoss* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END