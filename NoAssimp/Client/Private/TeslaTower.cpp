#include "stdafx.h"
#include "TeslaTower.h"
#include "ElecChain.h"
#include "ElecParts.h"

CTeslaTower::CTeslaTower(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTeslaTower::CTeslaTower(const CTeslaTower& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTeslaTower::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTeslaTower::Initialize(void* pArg)
{
	TESLATOWER_DESC tTeslaTowerDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		tTeslaTowerDesc = *(TESLATOWER_DESC*)pArg;

	tTeslaTowerDesc.fSpeedPerSec = 10.f;
	tTeslaTowerDesc.fRotationPerSec = XMConvertToRadians(90.0f);


	if (FAILED(__super::Initialize(&tTeslaTowerDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(tTeslaTowerDesc.iLevel))))
		return E_FAIL;

	m_iHp = 30;

	m_pSphere = Add_Sphere(LEVEL(tTeslaTowerDesc.iLevel), tTeslaTowerDesc.matWorld);
	m_pElecParts = Add_ElecParts(LEVEL(tTeslaTowerDesc.iLevel), tTeslaTowerDesc.matWorld);
	
	for (_uint i = 0; i < m_iNumElecChains; i++) 
		m_pElecChain[i] = Add_ElecChain(LEVEL(tTeslaTowerDesc.iLevel), tTeslaTowerDesc.matWorld, tTeslaTowerDesc.vReactorPos, i);

	return S_OK;
}

void CTeslaTower::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	m_fPlayRatio = m_fTime / m_fEndTime;
	if (m_fPlayRatio > 1.f)
		m_fPlayRatio = 1.f;

	if (m_iHp <= 0) {
		Add_DestroyEffect();
		Set_Dead();
		return;
	}
		
	if (m_pTransformCom->Get_Pos_Float4().y < 0)
		m_pTransformCom->Fall(-fTimeDelta * 2.5f);

	if (m_pSphere != nullptr)
		m_pSphere->Tick(fTimeDelta);

	if (m_pElecParts != nullptr)
		m_pElecParts->Tick(fTimeDelta);

	for (_uint i = 0; i < m_iNumElecChains; i++) {
		if (m_pElecChain[i] != nullptr)
			m_pElecChain[i]->Tick(fTimeDelta);
	}

	if (m_pColliderCom != nullptr)
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CTeslaTower::Late_Tick(_float fTimeDelta)
{
	if (m_bDead)
		return;

	if (m_pSphere != nullptr)
		m_pSphere->Late_Tick(fTimeDelta);

	if (m_pElecParts != nullptr)
		m_pElecParts->Late_Tick(fTimeDelta);

	for (_uint i = 0; i < m_iNumElecChains; i++) {
		if (m_pElecChain[i] != nullptr)
			m_pElecChain[i]->Late_Tick(fTimeDelta);
	}

	if (m_iPreHp != m_iHp) {
		if (m_fTime >= 0.3f)
			m_fTime = 0.f;
	}

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_iPreHp = m_iHp;
}

HRESULT CTeslaTower::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(11))) // Damagable
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif 

	return S_OK;
}

CGameObject* CTeslaTower::Add_Sphere(LEVEL eLevel, _float4x4 _matWorld)
{
	GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = eLevel;
	tDesc.matWorld = _matWorld;
	tDesc.matWorld._42 = 24.f;

	return m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Sphere"), &tDesc);
}

CGameObject* CTeslaTower::Add_ElecParts(LEVEL eLevel, _float4x4 _matWorld)
{
	CElecParts::ELECPARTS_DESC tElecPartsDesc{};
	tElecPartsDesc.iLevel = eLevel;
	tElecPartsDesc.matWorld = _matWorld;
	tElecPartsDesc.matWorld._42 = 24.f;
	tElecPartsDesc.eElecEffect = CElecParts::ELEC_TESLATOWER;

	return m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_ElecParts"), &tElecPartsDesc);
}

CGameObject* CTeslaTower::Add_ElecChain(LEVEL eLevel, _float4x4 _matWorld, _fvector vReactorPos, _uint iChainIndex)
{
	_vector vSpherePos = m_pTransformCom->Get_Pos_Vector();
	vSpherePos = XMVectorSetY(vSpherePos, 24);

	_float fDistance = XMVectorGetX(XMVector4Length(vReactorPos - vSpherePos));
	_vector vTempLook = XMVector4Normalize(vReactorPos - vSpherePos);

	_float4 vElecChainPos{};
	XMStoreFloat4(&vElecChainPos, vSpherePos + (vTempLook * fDistance / _float(m_iNumElecChains + 1) * (iChainIndex + 1)));

	CElecChain::ELEC_CHAIN_DESC tElecChainDesc{};
	tElecChainDesc.iLevel = eLevel;
	tElecChainDesc.matWorld._41 = vElecChainPos.x;
	tElecChainDesc.matWorld._42 = vElecChainPos.y;
	tElecChainDesc.matWorld._43 = vElecChainPos.z;
	tElecChainDesc.iElecChainIndex = iChainIndex;
	tElecChainDesc.vReactorPos = vReactorPos;

	return m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_ElecChain"), &tElecChainDesc);
}

void CTeslaTower::Add_DestroyEffect()
{
	CElecParts::ELECPARTS_DESC tElecPartsDesc{};
	tElecPartsDesc.iLevel = LEVEL_WIZARD;
	tElecPartsDesc.matWorld = m_pTransformCom->Get_WorldFloat4x4();
	tElecPartsDesc.eElecEffect = CElecParts::ELEC_DESTROY;
	tElecPartsDesc.fDuration = 1.2f;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_ElecParts"), TEXT("Prototype_GameObject_ElecParts"), &tElecPartsDesc)))
		return;
}

HRESULT CTeslaTower::Add_Components(LEVEL eLevel)
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_TeslaTower");
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.vCenter = _float3(0.f, 5.f, 0.f);
	ColliderDesc.fRadius = 8.f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTeslaTower::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CTeslaTower* CTeslaTower::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTeslaTower* pInstance = new CTeslaTower(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTeslaTower"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CTeslaTower::Clone(void* pArg)
{
	CTeslaTower* pInstance = new CTeslaTower(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTeslaTower"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTeslaTower::Free()
{
	__super::Free();

	for (_uint i = 0; i < m_iNumElecChains; i++)
		Safe_Release(m_pElecChain[i]);
	Safe_Release(m_pElecParts);
	Safe_Release(m_pSphere);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
