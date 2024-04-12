#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CTongMap_Wizard final : public CGameObject
{
private:
	enum TONGMAP_WIZARD_TEX { TEX_NOISE, TEX_END };

private:
	CTongMap_Wizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTongMap_Wizard(const CTongMap_Wizard& rhs);
	virtual ~CTongMap_Wizard() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void SetUp_MeshShaderPassIndices(_uint iNumMeshes);

private:
	_float m_fOffset_Y = {};
	_float m_fAlpha = {0.5f};

	vector<_uint> m_vecMeshShaderPassIndices;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CTongMap_Wizard* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

