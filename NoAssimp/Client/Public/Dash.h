#pragma once
#include "Client_Defines.h"
#include "Part.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CDash final : public CPart
{
private:
	CDash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDash(const CDash& rhs);
	virtual ~CDash() = default;

public:
	void Reset_Time() { m_fTime = 0; m_iSpriteIndex = 0; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	_float4x4 LookAtCam_UpFixed();

private:
	_uint m_iSpriteIndex = { 6 };

	_float m_fTime = {};
	_float m_fScale = {};

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	const _float4x4* m_pTargetWorldMat = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CDash* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END



