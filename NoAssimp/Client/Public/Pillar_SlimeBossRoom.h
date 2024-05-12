#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
END

BEGIN(Client)

class CPillar_SlimeBossRoom : public CGameObject
{
private:
	CPillar_SlimeBossRoom(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPillar_SlimeBossRoom(const CPillar_SlimeBossRoom& rhs);
	virtual ~CPillar_SlimeBossRoom() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_bool Check_HeadCol();

	void Add_RockParticles(_bool bIsPlayer = true);

private:
	_float m_fTime = { 100.f };
	_float m_fEndTime = { 0.35f };
	_float m_fPlayRatio = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CPillar_SlimeBossRoom* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
