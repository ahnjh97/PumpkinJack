#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CBone;
class CModel;
class CShader;
END

BEGIN(Client)

class CSkullCrossBow final : public CPart
{
public:
	typedef struct : public CPart::PART_DESC
	{
		CBone* pSocket = { nullptr };
		const BALLISTA* pState;
	}SKULL_CROSSBOW_DESC;

private:
	CSkullCrossBow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSkullCrossBow(const CSkullCrossBow& rhs);
	virtual ~CSkullCrossBow() = default;

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
	const BALLISTA* m_pState = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CSkullCrossBow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CPart* Clone(void* pArg) override;
	virtual void Free() override;
};

END