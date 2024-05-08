#pragma once
#include "Client_Defines.h"
#include "Camera_Jack.h"
#include "Part.h"

BEGIN(Engine)
class CVIBuffer_Instance_Point;
class CTexture;
class CShader;
END

BEGIN(Client)

class CArrowEffect final : public CPart
{
	enum TEX_ARROWEFFECT { TEX_SMOKE, TEX_NOISE, TEX_END };
private:
	CArrowEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArrowEffect(const CArrowEffect& rhs);
	virtual ~CArrowEffect() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;


private:
	_float m_fTime = {};

private:
	CVIBuffer_Instance_Point* m_pVIBufferCom = { nullptr };
	CTexture* m_pTextureCom[TEX_END] = { nullptr, nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCamera_Jack* m_pCamera = { nullptr };

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CArrowEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
