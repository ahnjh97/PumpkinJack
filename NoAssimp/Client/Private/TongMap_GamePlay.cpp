#include "stdafx.h"
#include "TongMap_GamePlay.h"

CTongMap_GamePlay::CTongMap_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTongMap_GamePlay::CTongMap_GamePlay(const CTongMap_GamePlay& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTongMap_GamePlay::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTongMap_GamePlay::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_iLevel = GameObjectDesc.iLevel;

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	return S_OK;
}

void CTongMap_GamePlay::Tick(_float fTimeDelta)
{
	//m_fOffset_Y -= 0.0015f;
}

void CTongMap_GamePlay::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this); // 알파값 있는 애들때문에 객체 분리해야될거같음
}

HRESULT CTongMap_GamePlay::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	vector<_uint> vecMeshShaderType = m_pModelCom->Get_MeshShaderType();
	for (size_t i = 0; i < iNumMeshes; i++)
	{
		SHADER eShader = (SHADER)vecMeshShaderType[i];
		string g_strTexture = "g_DiffuseTexture";
		if (eShader == NO_LIGHT)
			g_strTexture = "g_Texture";
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom[vecMeshShaderType[i]], g_strTexture.c_str(), i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이터를 다 던져야한다. */
		if (FAILED(m_pShaderCom[vecMeshShaderType[i]]->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CTongMap_GamePlay::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Lighted");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader_Lighted"), (CComponent**)&m_pShaderCom[LIGHTED])))
		return E_FAIL;

	wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_AlphaTex");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader_AlphaTex"), (CComponent**)&m_pShaderCom[ALPHA])))
		return E_FAIL;

	wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom[NO_LIGHT])))
		return E_FAIL;

	return S_OK;
}

HRESULT CTongMap_GamePlay::Bind_ShaderResources()
{
	for (_int i = 0; i < SHADER_END; i++)
	{
		if (nullptr == m_pShaderCom[i])
			return E_FAIL;
	}

	const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_LightDesc(0);
	if (nullptr == pLightDesc)
		return E_FAIL;

	for (_int i = 0; i <= ALPHA; i++)
	{
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_RawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_RawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
			return E_FAIL;
	}

	for (_int i = NO_LIGHT; i < SHADER_END; i++)
	{
		if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom[i], "g_WorldMatrix")))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;
	}

	return S_OK;
}

CTongMap_GamePlay* CTongMap_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTongMap_GamePlay* pInstance = new CTongMap_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTongMap_GamePlay"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CTongMap_GamePlay::Clone(void* pArg)
{
	CTongMap_GamePlay* pInstance = new CTongMap_GamePlay(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTongMap_GamePlay"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTongMap_GamePlay::Free()
{
	__super::Free();
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	for (_int j = 0; j < SHADER_END; j++)
		Safe_Release(m_pShaderCom[j]);
	Safe_Release(m_pModelCom);
}
