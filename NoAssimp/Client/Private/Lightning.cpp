#include "stdafx.h"
#include "Lightning.h"

CLightning::CLightning(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLightning::CLightning(const CLightning& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CLightning::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLightning::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;


	_float fScale = 20.f;
	m_pTransformCom->Set_Scaled(fScale, fScale, fScale);

	m_pTransformCom->Turn(XMVectorSet(1, 0, 0, 0), 1);

	/*if (FAILED(Get_PlayerColliderPtr()))
		return E_FAIL;*/

	if(GameObjectDesc.iLevel == LEVEL_EFFECTTOOL)
		m_pTransformCom->Set_Pos(XMVectorSet(-rand() % 50, 0, -rand() % 50, 1));

	return S_OK;
}

void CLightning::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
	if (m_fTime > 2.5f)
		Set_Dead();

	if (m_fTime > 0.5f && true == m_bAddCollider) {
		Add_Collider();
		m_bAddCollider = false;
	}

	if (m_pColliderCom != nullptr)
	{
		Check_PlayerCol();
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
	}
}

void CLightning::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}
HRESULT CLightning::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(4)))  // Lightning
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif

	return S_OK;
}

HRESULT CLightning::Get_PlayerColliderPtr()
{
	m_pPlayer = m_pGameInstance->Get_Player(LEVEL_WIZARD);
	if (nullptr == m_pPlayer)
		return E_FAIL;

	Safe_AddRef(m_pPlayer);

	m_pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));
	if(nullptr == m_pPlayerCollider)
		return E_FAIL;

	Safe_AddRef(m_pPlayerCollider);

	return S_OK;
}

void CLightning::Check_PlayerCol()
{
	if (m_pPlayerCollider == nullptr)
		return;

	if (m_pColliderCom->Intersect(m_pPlayerCollider)) {
		if(false == m_pColliderCom->Check_AlreadyCollided(m_pPlayerCollider))
			m_pPlayer->Deduct_HP(20);
	}
}

void CLightning::Add_Collider()
{
	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	ColliderDesc.fRadius = 0.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return;
}

HRESULT CLightning::Add_Components(LEVEL eLevel)
{	
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Circle"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLightning::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_Circle")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(m_fTime))))
		return E_FAIL;

	return S_OK;
}

CLightning* CLightning::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLightning* pInstance = new CLightning(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLightning"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLightning::Clone(void* pArg)
{
	CLightning* pInstance = new CLightning(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLightning"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLightning::Free()
{
	__super::Free();

	for (_int i = 0; i < 2; i++)
		Safe_Release(m_pElecParts[i]);
	Safe_Release(m_pPlayerCollider);
	Safe_Release(m_pPlayer);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
