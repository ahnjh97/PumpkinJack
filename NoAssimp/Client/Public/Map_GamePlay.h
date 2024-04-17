#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CMap_GamePlay final : public CGameObject
{
private:
	enum SHADER { LIGHTED, NO_LIGHT, SHADER_END };

private:
	CMap_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMap_GamePlay(const CMap_GamePlay& rhs);
	virtual ~CMap_GamePlay() = default;

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

	vector<_uint> m_vecMeshShaderTypes;
	vector<_uint> m_vecMeshShaderPassIndices;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom[SHADER_END] = {};


private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CMap_GamePlay* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END