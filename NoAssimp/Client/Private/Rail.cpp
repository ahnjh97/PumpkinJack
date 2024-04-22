#include "stdafx.h"
#include "Rail.h"

CRail::CRail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CRail::CRail(const CRail& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CRail::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CRail::Initialize(void* pArg)
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

	return S_OK;
}

void CRail::Tick(_float fTimeDelta)
{
	for(_int i = 0; i<7; i++)
		m_pColliderCom[i]->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CRail::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CRail::Render()
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
	for (_int i = 0; i < 7; i++) {
		if (m_pColliderCom[i] != nullptr)
			m_pGameInstance->Add_DebugComponents(m_pColliderCom[i]);
	}
#endif

	return S_OK;
}

_int CRail::Get_ColliderIndex(CCollider* pCollider)
{
	for (_int i = 0; i < 7; i++)
	{
		if (pCollider == m_pColliderCom[i])
			return i;
	}

	return -1;
}

_vector CRail::Compute_Direction(_int iColliderIndex)
{
	if (iColliderIndex >= 6)
		iColliderIndex = 5;
	if (iColliderIndex < 0)
		iColliderIndex = 0;

	_float3 vDstPos = m_pColliderCom[iColliderIndex]->Get_Center(CCollider::TYPE_OBB);
	_float3 vSrcPos = m_pColliderCom[iColliderIndex + 1]->Get_Center(CCollider::TYPE_OBB);

	_vector vDir = XMVector3Normalize(XMLoadFloat3(&vSrcPos) - XMLoadFloat3(&vDstPos));
	return vDir;
}

HRESULT CRail::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Lighted");
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(2, 1, 8.f);

	_float fDis = 5.f;
	_float fAngle = 0.f;
	for (_int i = 0; i < 7; i++)
	{
		if(_wstrModelTag == TEXT("Prototype_Component_Model_RailLeft60"))
			fAngle = (XM_PI / 40.f) * (i - 3);
		else if (_wstrModelTag == TEXT("Prototype_Component_Model_RailRight60"))
			fAngle = -(XM_PI / 40.f) * (i - 3);
		if (_wstrModelTag == TEXT("Prototype_Component_Model_RailLeft90"))
			fAngle = (XM_PI / 30.f) * (i - 3);
		else if (_wstrModelTag == TEXT("Prototype_Component_Model_RailRight90"))
			fAngle = -(XM_PI / 30.f) * (i - 3);

		ColliderDesc.vCenter = _float3((i - 3) * fDis * cosf(fAngle), ColliderDesc.vSize.y * 0.5f, (i - 3) * fDis * sinf(fAngle));
		wstring wstrColliderTag = L"Com_Collider" + to_wstring(i);

		if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
			wstrColliderTag, (CComponent**)&m_pColliderCom[i], &ColliderDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CRail::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCamFar", m_pGameInstance->Get_CamFarPtr(), sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CRail* CRail::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRail* pInstance = new CRail(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CRail"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CRail::Clone(void* pArg)
{
	CRail* pInstance = new CRail(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CRail"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CRail::Free()
{
	__super::Free();

	for(_int i =0; i<7; i++)
		Safe_Release(m_pColliderCom[i]);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
