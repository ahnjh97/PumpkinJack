#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CCollider final : public CComponent
{
public:
	enum TYPE { TYPE_AABB, TYPE_OBB, TYPE_SPHERE, TYPE_END };
private:
	CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCollider(const CCollider& rhs);
	virtual ~CCollider() = default;

public: 
	_float Get_HightestPoint();
	_float3 Get_Center();
	_bool Get_Disabled() { return m_bDisabled; }
	void Set_Disabled(_bool bDisabled) { m_bDisabled = bDisabled; }
	_float3 Get_ColliderLook();
	void Clear_CollidedColliders() { m_CollidedColliders.clear(); }
	_bool Check_AlreadyCollided(CCollider* pEnemyCollider);
	TYPE Get_ColliderType() { return m_eType; }

public:
	virtual HRESULT Initialize_Prototype(TYPE eType);
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_fmatrix WorldMatrix);
	_bool Intersect(CCollider* pTargetCollider);
	_bool Intersect_Ray(_fvector vRayOrigin, _fvector vRayDir, _float& fDis);

#ifdef _DEBUG
public:
	HRESULT Render();
#endif

private:
	TYPE				m_eType = { TYPE_END };
	class CBounding*	m_pBounding = { nullptr };
	_bool				m_bDisabled = { false };

	unordered_set<CCollider*> m_CollidedColliders; // 충돌한 콜라이더 저장

#ifdef _DEBUG
private:
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
	BasicEffect* m_pEffect = { nullptr };
	ID3D11InputLayout* m_pInputLayout = { nullptr };
#endif

public:
	static CCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

END