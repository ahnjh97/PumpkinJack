#pragma once
#include "GameObject.h"

BEGIN(Engine)
class ENGINE_DLL CPart abstract: public CGameObject
{
public:
	typedef struct : public GAMEOBJECT_DESC
	{
		const _float4x4* pParentMatrix;
	}PART_DESC;

protected:
	CPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPart(const CPart& rhs);
	virtual ~CPart() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	_float4x4				m_WorldMatrix = {};
	const _float4x4*		m_pParentMatrix;

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};
END