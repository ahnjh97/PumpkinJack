#include "stdafx.h"
#include "SwipeEffect.h"

CSwipeEffect::CSwipeEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSwipeEffect::CSwipeEffect(const CSwipeEffect& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSwipeEffect::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSwipeEffect::Initialize(void* pArg)
{
	SWIPEEFFECT_DESC GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(SWIPEEFFECT_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	m_iLevel = GameObjectDesc.iLevel;

	_float fScale = 25.f;
	m_pTransformCom->Set_Scaled(fScale * 1.9f, fScale * 0.04f, fScale * 1.8f);

	m_pTransformCom->Turn(GameObjectDesc.vSlimeBossRight, 0.02f);
	m_pTransformCom->Set_Pos(m_pTransformCom->Get_Pos_Vector() + XMVectorSet(0, 4.8f, 0, 0) 
		+ m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 0.32f);

	m_fEndTime = 0.54f;
	m_fWaitTime = 1.25f;

	return S_OK;
}

void CSwipeEffect::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
	m_fSpriteTime += fTimeDelta;

	if (false == m_bEnabled) {
		if (m_fWaitTime < m_fTime) {
			m_bEnabled = true;
			m_fTime = 0.f;
			m_fSpriteTime = 0.f;
		}
	}

	if (true == m_bEnabled) {
		if (m_fSpriteTime >= m_fEndTime / 6.f) {
			m_iSpriteIndex--;
			m_fSpriteTime = 0.f;
		}

		if (m_iSpriteIndex >= 6)
			m_iSpriteIndex = 6;

		if (m_iSpriteIndex <= 0)
			m_iSpriteIndex = 0;

		m_fPlayRatio = m_fTime / m_fEndTime;
		if (m_fPlayRatio > 1.f)
			m_fPlayRatio = 1.f;

		if (m_fPlayRatio > 0.2f && m_fPlayRatio < 0.9f) {
			Check_PlayerCol();
			if (true == m_bPlaySwipeSound) {
				m_pGameInstance->StopSound(SKILL0);
				m_pGameInstance->Play_Sound(TEXT("Swipe.mp3"), SKILL0, 0.5f);
				m_bPlaySwipeSound = false;
			}
		}

		if (m_fTime > m_fEndTime)
			Set_Dead();
	}

	if (m_pColliderCom != nullptr)
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CSwipeEffect::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CSwipeEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;


		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(13))) // SwipeEffect
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif 

	return S_OK;
}

void CSwipeEffect::Check_PlayerCol()
{
	if (true == m_pColliderCom->Get_Disabled())
		return;

	CGameObject* pPlayer = m_pGameInstance->Get_Player(m_iLevel);
	if (nullptr == pPlayer)
		return;

	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
	if (pPlayerCollider == nullptr)
		return;

	if (m_pColliderCom->Intersect(pPlayerCollider)) {
		pPlayer->Deduct_HP(10);
		m_pColliderCom->Set_Disabled(true);
	}
}

HRESULT CSwipeEffect::Add_Components(LEVEL eLevel)
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_Slash");
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Slash"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_SLASH])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Noise"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TEX_NOISE])))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(6.f, 1.7f, 6.f);
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSwipeEffect::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iWeaponIndex", &m_iWeaponIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iSpriteIndex", &m_iSpriteIndex, sizeof(_uint))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_SLASH]->Bind_ShaderResources(m_pShaderCom, "g_AttackEffect")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResources(m_pShaderCom, "g_NoiseTexture")))
		return E_FAIL;

	return S_OK;
}

CSwipeEffect* CSwipeEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSwipeEffect* pInstance = new CSwipeEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSwipeEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwipeEffect::Clone(void* pArg)
{
	CSwipeEffect* pInstance = new CSwipeEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSwipeEffect"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwipeEffect::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	for(_int i = 0; i< TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pModelCom);
}
