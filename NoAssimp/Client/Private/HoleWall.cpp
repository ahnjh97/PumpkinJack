#include "stdafx.h"
#include "ElecParts.h"
#include "HoleWall.h"

CHoleWall::CHoleWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CHoleWall::CHoleWall(const CHoleWall& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CHoleWall::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CHoleWall::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	m_pPlayer = m_pGameInstance->Get_Player(LEVEL_WIZARD);
	Safe_AddRef(m_pPlayer);
	m_fFallSpeed = m_pGameInstance->RandomNumber(5, 15) * 0.1f;

	CElecParts::ELECPARTS_DESC tElecPartsDesc{};
	tElecPartsDesc.iLevel = GameObjectDesc.iLevel;
	tElecPartsDesc.eElecEffect = CElecParts::ELEC_HOLE;
	tElecPartsDesc.pTargetWorldMat = m_pTransformCom->Get_WorldFloat4x4_Ptr();

	m_pElecParts = m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_ElecParts"), &tElecPartsDesc);
	if (m_pElecParts == nullptr)
		return E_FAIL;

	return S_OK;
}

void CHoleWall::Tick(_float fTimeDelta)
{
	if (m_fTime > 9.f)
		Set_Dead();

	m_fTime += fTimeDelta;
	m_fDamageTime += fTimeDelta;
	m_pTransformCom->Move(fTimeDelta * 3.f, m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK));
	m_pTransformCom->Fall(fTimeDelta * m_fFallSpeed);

	Check_PlayerCol();

	m_pElecParts->Tick(fTimeDelta);

	for (_int i = 0; i < 2; i++)
		m_pColliderCom[i]->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CHoleWall::Late_Tick(_float fTimeDelta)
{
	m_pElecParts->Late_Tick(fTimeDelta);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this, true);
}

HRESULT CHoleWall::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(7))) // HoleWall
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	for (_int i = 0; i < 2; i++) {
		if (m_pColliderCom[i] != nullptr)
			m_pGameInstance->Add_DebugComponents(m_pColliderCom[i]);
	}
#endif

	return S_OK;
}

void CHoleWall::Check_PlayerCol()
{
	if (m_pPlayer == nullptr)
		return;

	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));
	if (m_pColliderCom[0]->Intersect(pPlayerCollider)) { 
		if (!m_pColliderCom[1]->Intersect(pPlayerCollider)) { // 구멍
			if (m_fDamageTime >= 3.f) {
				m_pPlayer->Deduct_HP(30);
				m_fDamageTime = 0.f;
			}
		}
	}
}

HRESULT CHoleWall::Add_Components(LEVEL eLevel)
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_HoleWall");
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_FireNoise"),
		TEXT("Com_Texture_FireNoise"), (CComponent**)&m_pTextureCom[TEX_FIRENOISE])))
		return E_FAIL;
	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_VoronoiBlur"),
		TEXT("Com_Texture_VoronoiBlur"), (CComponent**)&m_pTextureCom[TEX_VORONOIBLUR])))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC	tOBBDesc{};
	tOBBDesc.vCenter = _float3();
	tOBBDesc.vSize = _float3(1000, 1000, 10);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom[0], &tOBBDesc)))
		return E_FAIL;

	CBounding_Sphere::BOUNDING_SPHERE_DESC	ColliderDesc{};
	ColliderDesc.vCenter = _float3();
	ColliderDesc.fRadius = 20.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider_Sub"), (CComponent**)&m_pColliderCom[1], &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CHoleWall::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_FIRENOISE]->Bind_ShaderResources(m_pShaderCom, "g_FireNoise")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_VORONOIBLUR]->Bind_ShaderResources(m_pShaderCom, "g_VoronoiBlur")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CHoleWall* CHoleWall::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CHoleWall* pInstance = new CHoleWall(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CHoleWall"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CHoleWall::Clone(void* pArg)
{
	CHoleWall* pInstance = new CHoleWall(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CHoleWall"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CHoleWall::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);

	for (_int i = 0; i < 2; i++)
		Safe_Release(m_pColliderCom[i]);
	for(_int j = 0; j < TEX_END; j++)
		Safe_Release(m_pTextureCom[j]);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pElecParts);
}
