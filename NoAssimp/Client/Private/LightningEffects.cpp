#include "stdafx.h"
#include "LightningEffects.h"
#include "Lightning.h"
#include "ElecParts.h"

CLightningEffects::CLightningEffects(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CLightningEffects::CLightningEffects(const CLightningEffects& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CLightningEffects::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLightningEffects::Initialize(void* pArg)
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

	m_iLevel = GameObjectDesc.iLevel;

	_float fScale = 20.f;
	m_pTransformCom->Set_Scaled(fScale, fScale, fScale);

	m_pTransformCom->Turn(XMVectorSet(1, 0, 0, 0), 1);

	if(GameObjectDesc.iLevel == LEVEL_EFFECTTOOL)
		m_pTransformCom->Set_Pos(XMVectorSet(-rand() % 50, 0, -rand() % 50, 1));
	else
	{
		if (FAILED(Get_PlayerColliderPtr()))
			return E_FAIL;
	}

	return S_OK;
}

void CLightningEffects::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
	if (m_fTime > 1.8f)
		Set_Dead();

	if (m_fTime > 1.45f) {
		if (m_pElecParts[0] != nullptr) {
			Safe_Release(m_pElecParts[0]);
			m_pColliderCom->Set_Disabled(true);
		}
	}
		
	if (m_fTime > 0.75f && true == m_bAddLightning) {
		Add_Lightning(2);
		m_bAddLightning = false;
	}

	if (m_fTime > 1.05f && true == m_bAddElecParts) {
		Add_ElecParts();
		Add_Collider();
		m_bAddElecParts = false;
	}

	for (_int i = 0; i < 2; i++) {
		if(m_pElecParts[i] != nullptr)
			m_pElecParts[i]->Tick(fTimeDelta);
	}

	if (m_pLightning != nullptr)
		m_pLightning->Tick(fTimeDelta);
	
	if (m_pColliderCom != nullptr)
	{
		Check_PlayerCol();
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
	}
}

void CLightningEffects::Late_Tick(_float fTimeDelta)
{
	for (_int i = 0; i < 2; i++) {
		if (m_pElecParts[i] != nullptr)
			m_pElecParts[i]->Late_Tick(fTimeDelta);
	}

	if (m_pLightning != nullptr)
		m_pLightning->Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}
HRESULT CLightningEffects::Render()
{
	//if (FAILED(Bind_ShaderResources()))
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Begin(4)))  // Circle
	//	return E_FAIL;

	//if (FAILED(m_pVIBufferCom->Bind_Buffers()))
	//	return E_FAIL;

	/*if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;*/

	if(m_pLightning != nullptr)
		m_pLightning->Render();

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif

	return S_OK;
}

HRESULT CLightningEffects::Get_PlayerColliderPtr()
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

void CLightningEffects::Check_PlayerCol()
{
	if(true == m_pColliderCom->Get_Disabled())
		return;

	if (m_pPlayerCollider == nullptr)
		return;
	
	if (m_pColliderCom->Intersect(m_pPlayerCollider)) {
		m_pPlayer->Deduct_HP(5);
		m_pColliderCom->Set_Disabled(true);
	}
}

void CLightningEffects::Add_Collider()
{
	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	ColliderDesc.fRadius = 0.5f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return;
}

void CLightningEffects::Add_ElecParts()
{
	_float4 vPos = m_pTransformCom->Get_Pos_Float4();

	CElecParts::ELECPARTS_DESC tElecPartsDesc{};
	tElecPartsDesc.iLevel = m_iLevel;
	tElecPartsDesc.eElecEffect = CElecParts::ELEC_LIGHTNING1;
	tElecPartsDesc.matWorld._41 = vPos.x;
	tElecPartsDesc.matWorld._42 = vPos.y;
	tElecPartsDesc.matWorld._43 = vPos.z;

	m_pElecParts[0] = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_ElecParts"), &tElecPartsDesc);
	if (m_pElecParts == nullptr)
		return;
}

void CLightningEffects::Add_Lightning(_uint iLightningIndex)
{
	_float4 vPos = m_pTransformCom->Get_Pos_Float4();

	CLightning::LIGHTNING_DESC tLightningDesc{};
	tLightningDesc.iLevel = m_iLevel;
	tLightningDesc.iLightingIndex = iLightningIndex;
	tLightningDesc.matWorld._41 = vPos.x;
	tLightningDesc.matWorld._42 = vPos.y;
	tLightningDesc.matWorld._43 = vPos.z;

	m_pLightning = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Lightning"), &tLightningDesc);
	if (m_pLightning == nullptr)
		return;
}

HRESULT CLightningEffects::Add_Components(LEVEL eLevel)
{	
	///* For.Com_Shader */
	//if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
	//	TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
	//	return E_FAIL;

	///* For.Com_Texture */
	//if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Circle"),
	//	TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
	//	return E_FAIL;

	///* For.Com_VIBuffer */
	//if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
	//	TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLightningEffects::Bind_ShaderResources()
{
	//if (nullptr == m_pShaderCom)
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
	//	return E_FAIL;
	//if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_Circle")))
	//	return E_FAIL;
	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(m_fTime))))
	//	return E_FAIL;
	/*if (FAILED(m_pGameInstance->Bind_RTShaderResource(m_pShaderCom, TEXT("Target_FieldDepth"), "g_FieldDepth")))
		return E_FAIL;*/

	return S_OK;
}

CLightningEffects* CLightningEffects::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLightningEffects* pInstance = new CLightningEffects(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CLightningEffects"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLightningEffects::Clone(void* pArg)
{
	CLightningEffects* pInstance = new CLightningEffects(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CLightningEffects"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLightningEffects::Free()
{
	__super::Free();

	Safe_Release(m_pLightning);
	for (_int i = 0; i < 2; i++)
		Safe_Release(m_pElecParts[i]);
	Safe_Release(m_pPlayerCollider);
	Safe_Release(m_pPlayer);
	Safe_Release(m_pColliderCom);

	//Safe_Release(m_pShaderCom);
	//Safe_Release(m_pTextureCom);
	//Safe_Release(m_pVIBufferCom);
}
