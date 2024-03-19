#include "stdafx.h"
#include "Monster.h"

CMonster::CMonster(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMonster::CMonster(const CMonster & rhs)
	: CGameObject{ rhs }
{

}

HRESULT CMonster::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMonster::Initialize(void * pArg)
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

	//m_pModelCom->Set_Animation(11, true); // 999: 랜덤 애니메이션
	if(!m_pModelCom->IsNonAnimVersion() && m_pModelCom->Get_ModelType() == TYPE_ANIM)
		m_pModelCom->Set_Animation(999, true); // 999: 랜덤 애니메이션
	
 	return S_OK;
}

void CMonster::Tick(_float fTimeDelta)
{
}

void CMonster::Late_Tick(_float fTimeDelta)
{
	_float3 vPreHipTrans = m_pModelCom->Get_RootTranslation();
	if ((vPreHipTrans.x < -5000.f) || (vPreHipTrans.y < -5000.f) || (vPreHipTrans.z < -5000.f))
		vPreHipTrans = _float3{ 0.f, 0.f, 0.f };

	m_pModelCom->Play_Animation(fTimeDelta);

	_float3 vHipTrans = m_pModelCom->Get_RootTranslation();

	_float3 vRelativeTranslation = {};
	XMStoreFloat3(&vRelativeTranslation,  XMLoadFloat3(&vHipTrans) - XMLoadFloat3(&vPreHipTrans));
	_float fNewScale = m_pModelCom->Get_ModelConfig().fNewScale * 0.9f;

	_matrix matTemp = XMMatrixTranslation(vRelativeTranslation.x * fNewScale, vRelativeTranslation.y * fNewScale, vRelativeTranslation.z * fNewScale);
	_matrix matRelative = XMMatrixMultiply(m_pTransformCom->Get_WorldMatrix(), matTemp);
	_float4x4 matWorld = {};
	XMStoreFloat4x4(&matWorld, matRelative);
	m_pTransformCom->Set_WorldMatrix(matWorld);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CMonster::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if(m_pModelCom->Get_ModelType() == TYPE_NONANIM)
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

HRESULT CMonster::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
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

HRESULT CMonster::Bind_ShaderResources()
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

CMonster * CMonster::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CMonster*		pInstance = new CMonster(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMonster"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CMonster::Clone(void * pArg)
{
	CMonster*		pInstance = new CMonster(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CMonster"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMonster::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);	
	Safe_Release(m_pModelCom);
}
