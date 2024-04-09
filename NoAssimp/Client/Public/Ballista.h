#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPart;
END

BEGIN(Client)

class CBallista final : public CGameObject
{
private:
	CBallista(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBallista(const CBallista& rhs);
	virtual ~CBallista() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Add_Arrow();
	void Emit_AlarmLight();

private:
	BALLISTA m_eState = {};

	_uint m_iArrowCount = {};

	_float m_fIdleTime = {};

	_vector m_vOriginalPos = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	map<const wstring, CPart*> m_Parts;
	const _float4x4* m_pPlayerWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();
	HRESULT Add_PartObjects(LEVEL eLevel);

public:
	static CBallista* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
