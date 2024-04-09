#include "stdafx.h"
#include "TongMap_Wizard.h"

CTongMap_Wizard::CTongMap_Wizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTongMap_Wizard::CTongMap_Wizard(const CTongMap_Wizard& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTongMap_Wizard::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTongMap_Wizard::Initialize(void* pArg)
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

void CTongMap_Wizard::Tick(_float fTimeDelta)
{
	m_fOffset_Y -= 0.0015f;
}

void CTongMap_Wizard::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this); // 알파값 있는 애들때문에 객체 분리해야될거같음
}

HRESULT CTongMap_Wizard::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	SetUp_MeshShaderTypes(iNumMeshes);
	SetUp_MeshShaderPassIndices(iNumMeshes);

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		SHADER eShader = (SHADER)m_vecMeshShaderTypes[i];
		string g_strTexture = "g_DiffuseTexture";
		if (eShader == DEFAULT || eShader == ARENA_FIRE)
			g_strTexture = "g_Texture";

		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom[m_vecMeshShaderTypes[i]], g_strTexture.c_str(), i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이터를 다 던져야한다. */
		if (FAILED(m_pShaderCom[m_vecMeshShaderTypes[i]]->Begin(m_vecMeshShaderPassIndices[i])))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CTongMap_Wizard::SetUp_MeshShaderTypes(_uint iNumMeshes)
{
	m_vecMeshShaderTypes.resize(iNumMeshes);

}

void CTongMap_Wizard::SetUp_MeshShaderPassIndices(_uint iNumMeshes)
{
	m_vecMeshShaderPassIndices.resize(iNumMeshes);
	for (_int i = 0; i < iNumMeshes; i++)
	{
		string strDiffuseTexName = m_pModelCom->Get_DiffuseTextureName(i);
		if (strDiffuseTexName == "T_Earth_Mat")
			m_vecMeshShaderPassIndices[i] = 1;
	}
}

HRESULT CTongMap_Wizard::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom[DEFAULT])))
		return E_FAIL;

	wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_ArenaFire");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader_ArenaFire"), (CComponent**)&m_pShaderCom[ARENA_FIRE])))
		return E_FAIL;

	/*if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Water"),
		TEXT("Com_Texture_Water"), (CComponent**)&m_pTextureCom[TEX_WATER])))
		return E_FAIL;*/

	return S_OK;
}

HRESULT CTongMap_Wizard::Bind_ShaderResources()
{
	for (_int i = 0; i < SHADER_END; i++) {
		if (nullptr == m_pShaderCom[i])
			return E_FAIL;
	}

	for (_int i = 0; i < SHADER_END; i++)
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

CTongMap_Wizard* CTongMap_Wizard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTongMap_Wizard* pInstance = new CTongMap_Wizard(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTongMap_Wizard"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CTongMap_Wizard::Clone(void* pArg)
{
	CTongMap_Wizard* pInstance = new CTongMap_Wizard(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTongMap_Wizard"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTongMap_Wizard::Free()
{
	__super::Free();
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	for (_int j = 0; j < SHADER_END; j++)
		Safe_Release(m_pShaderCom[j]);
	Safe_Release(m_pModelCom);
}
