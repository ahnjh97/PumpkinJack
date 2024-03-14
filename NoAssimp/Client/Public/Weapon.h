#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CBone;
class CModel;
class CShader;
END

BEGIN(Client)

class CWeapon final : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		CBone* pSocket = { nullptr };
		const WEAPON* pWeapon;
	}WEAPON_DESC;

private:
	CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon(const CWeapon& rhs);
	virtual ~CWeapon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CBone* m_pSocket = { nullptr };
	const WEAPON* m_pWeapon = {};

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
