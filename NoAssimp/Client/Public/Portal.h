#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CBone;
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CPortal final : public CGameObject
{
public:
	typedef struct : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4* pParentMatrix;
		CBone* pPortalBone = { nullptr };
	}PORTAL_DESC;
private:
	CPortal(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPortal(const CPortal& rhs);
	virtual ~CPortal() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	void SlimeBossHeadSpawn(_float3 vPos);

private:
	_bool m_bSpawnSlimeBossRoomPillars = { false };
	_bool m_bInitializeColliderPos = { true };

	_float m_fScale = { 24.f };

	_vector m_vOriginalPos = {};
	_vector m_vLook = {};
	

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };
	CCollider* m_pColliderCom = { nullptr };
	class CPlayer* m_pPlayer = { nullptr };
	const _float4x4* m_pPlayerWorldMat = { nullptr };
	CCollider* m_pPlayerCollider = { nullptr };

	CBone* m_pSocket = { nullptr };
	_float4x4 m_WorldMatrix = {};
	const _float4x4* m_pParentMatrix;

private:
	HRESULT Add_Components(LEVEL eLevel);
	HRESULT Bind_ShaderResources();

public:
	static CPortal* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END