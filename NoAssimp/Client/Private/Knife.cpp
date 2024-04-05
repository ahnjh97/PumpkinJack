#include "stdafx.h"
#include "Knife.h"
#include <Player.h>

CKnife::CKnife(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CKnife::CKnife(const CKnife& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CKnife::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CKnife::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	_float fAngle = m_pGameInstance->RandomNumber(0, 359) / 90.f;
	m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), fAngle);
	m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), 0.15f);
	return S_OK;
}

void CKnife::Tick(_float fTimeDelta)
{
	if (m_fAlpha <= 0.01f)
		Set_Dead();

	m_fTime += fTimeDelta * 1.6f;

	if (m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y >= -3)
		m_pTransformCom->Fall(m_fTime);
	else
	{
		m_fAlpha -= 0.005f;
		return;
	}

	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
	if (pPlayer)
	{
		CCollider* pPlayerCollider = dynamic_cast<CCollider*>(pPlayer->Get_Component(TEXT("Com_Collider")));
		if (m_pColliderCom->Intersect(pPlayerCollider))
		{
			if(m_bDamaged == false)
				pPlayer->Change_HP(20);
			m_bDamaged = true;
		}
	}

	m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CKnife::Late_Tick(_float fTimeDelta)
{
	if (m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y >= -3)
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	else
	{
		m_pColliderCom->Set_Disabled(true);
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
	}
}

HRESULT CKnife::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);
		
			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pColliderCom->Render();
#endif

	return S_OK;
}

HRESULT CKnife::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Knife");
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_SLIMEBOSS, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(2, 24, 4.5f);
	ColliderDesc.vCenter = _float3(0.f, ColliderDesc.vSize.y * 0.5f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CKnife::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if(FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
		return E_FAIL;

	return S_OK;
}

CKnife* CKnife::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CKnife* pInstance = new CKnife(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CKnife"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CKnife::Clone(void* pArg)
{
	CKnife* pInstance = new CKnife(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CKnife"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CKnife::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
