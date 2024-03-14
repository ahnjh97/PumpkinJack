#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CModel;
class CShader;
END

class CBody : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		const STATE* pState;
		const WEAPON* pWeapon;
		const _uint* pLastAttack;
	}BODY_DESC;

private:
	CBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBody(const CBody& rhs);
	virtual ~CBody() = default;

public:
	_bool Get_IsFinished() { return m_pModelCom->isFinished(); }

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
	const STATE* m_pState;
	const WEAPON* m_pWeapon;
	const _uint* m_pLastAttack;

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CBody* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

