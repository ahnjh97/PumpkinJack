#include "stdafx.h"
#include "SlimeBoss.h"
#include "SlimeBoss_Slime.h"

CSlimeBoss::CSlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSlimeBoss::CSlimeBoss(const CSlimeBoss& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CSlimeBoss::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlimeBoss::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	if (FAILED(Add_PartObjects((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;

	m_pModelCom->Set_Animation(0, true);

	return S_OK;
}

void CSlimeBoss::Tick(_float fTimeDelta)
{
	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
}

void CSlimeBoss::Late_Tick(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
}

HRESULT CSlimeBoss::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (m_pModelCom->Get_ModelType() == TYPE_NONANIM)
			m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", i, aiTextureType_DIFFUSE);
		if (m_pModelCom->Get_ModelType() == TYPE_ANIM)
			m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CSlimeBoss::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = {};
	if (m_pModelCom->Get_ModelType() == TYPE_ANIM)
		wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	else
		wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");

	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeBoss::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;


	if (m_pModelCom->Get_ModelType() == TYPE_ANIM)
	{
		if (nullptr == m_pShaderCom)
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;

		const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_LightDesc(0);
		if (nullptr == pLightDesc)
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CSlimeBoss::Add_PartObjects(LEVEL eLevel)
{
	/* For.Part_SlimeBoss */
	CPart* pSlimeBoss_Slime = { nullptr };
	CSlimeBoss_Slime::SLIMEBOSS_SLIME_DESC	SlimeBossDesc{};

	SlimeBossDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	SlimeBossDesc.pState = &m_eState;
	SlimeBossDesc.iLevel = eLevel;
	SlimeBossDesc.wstrModelTag = TEXT("Prototype_Component_Model_Part_SlimeBoss");
	pSlimeBoss_Slime = dynamic_cast<CPart*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Part_SlimeBoss_Slime"), &SlimeBossDesc));
	if (nullptr == pSlimeBoss_Slime)
		return E_FAIL;
	m_Parts.emplace(TEXT("Part_SlimeBoss_Slime"), pSlimeBoss_Slime);
	return S_OK;
}

CSlimeBoss* CSlimeBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlimeBoss* pInstance = new CSlimeBoss(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CSlimeBoss::Clone(void* pArg)
{
	CSlimeBoss* pInstance = new CSlimeBoss(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSlimeBoss::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
