#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CSoldier final : public CGameObject
{
private:
	CSoldier(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSoldier(const CSoldier& rhs);
	virtual ~CSoldier() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Get_PlayerWorldMatPtr();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSoldier* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

