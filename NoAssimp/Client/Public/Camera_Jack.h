#pragma once
#include "Client_Defines.h"
#include "Camera.h"
class CCamera_Jack : public CCamera
{
public:
	typedef struct : public CCamera::CAMERA_DESC
	{
		_float fDistance = { 0.f };
		_float fHeight = { 0.f };
	}CAMERA_JACK_DESC;
private:
	CCamera_Jack(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Jack(const CCamera_Jack& rhs);
	virtual ~CCamera_Jack() = default;

public:
	void Copy_PlayerPos(_fvector vPlayerPos);

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void OnMouseInput(_float fTimeDelta);
	void RevolveAroundPlayer();

	_float3 m_vPlayerPos = {};
	_float m_fDistance = {};
	_float m_fHeight = {};

public:
	static CCamera_Jack* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

