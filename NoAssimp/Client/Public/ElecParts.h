#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CElecParts final : public CGameObject
{
	enum TEX_ELEC { TEX_ELECPARTS, TEX_NOISE, TEX_END };

public:
	enum ELEC_EFFECT { ELEC_TORNADO, ELEC_LIGHTNING1, ELEC_LIGHTNING2, ELEC_HOLE, ELEC_WALL, ELEC_CHARGE,
		ELEC_TESLATOWER, ELEC_DESTROY, ELEC_SCATTER, ELEC_EARTHQUAKE, ELEC_END };

	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		ELEC_EFFECT eElecEffect;
		const _float4x4* pTargetWorldMat = { nullptr };
		_float fDuration = { };
	}ELECPARTS_DESC;

public:
	ELEC_EFFECT Get_ElecEffect() { return m_eElecEffect; }

private:
	CElecParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CElecParts(const CElecParts& rhs);
	virtual ~CElecParts() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	ELEC_EFFECT m_eElecEffect = {};
	_bool m_bFadeIn = { true };
	_bool m_bShareTime = { false };

	_uint m_iSpriteIndex = {};

	_float m_fTime = {};
	_float m_fDuration = {};

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = {};
	CShader* m_pShaderCom = { nullptr };
	const _float4x4* m_pTargetWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel, ELEC_EFFECT eElecEffect);
	HRESULT Bind_ShaderResources();

public:
	static CElecParts* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END