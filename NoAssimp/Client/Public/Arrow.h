#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CPart;
END

BEGIN(Client)

class CArrow : public CGameObject
{
private:
	CArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CArrow(const CArrow& rhs);
	virtual ~CArrow() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void Check_CollisionCubeCol();
	void Check_PlayerCol();
	
private:
	_bool m_bObstacleCol = { false };

	_float m_fTime = {};
	_float m_fCollisionTime = {};
	_float m_fAlpha = { 1.f };

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };
	CPart* m_pArrowEffect = { nullptr };

private:
	HRESULT Add_Components(const wstring& _wstrModelTag);
	HRESULT Add_PartObjects(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CArrow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END

