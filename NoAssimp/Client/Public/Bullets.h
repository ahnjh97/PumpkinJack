#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CBullets final : public CGameObject
{
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4* pPlayerWorldMat;
		_float4 vPlayerLook;
		_float4 vPlayerRight;
		_float fWaitTime;
		_float fDuration = { 0.6f };
		WEAPON eWeapon;
		STATE eState;
	}BULLETS_DESC;

private:
	CBullets(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBullets(const CBullets& rhs);
	virtual ~CBullets() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	_float3 m_vPivot = {};

	_float m_fTime = {};
	_float m_fDuration = { 0.3f };

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CBullets* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END




