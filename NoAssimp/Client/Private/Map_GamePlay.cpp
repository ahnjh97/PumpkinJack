#include "stdafx.h"
#include "Map_GamePlay.h"

CMap_GamePlay::CMap_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMap_GamePlay::CMap_GamePlay(const CMap_GamePlay& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CMap_GamePlay::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMap_GamePlay::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_iLevel = LEVEL_GAMEPLAY;

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL_GAMEPLAY, GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	SetUp_MeshShaderPassIndices(iNumMeshes);

	return S_OK;
}

void CMap_GamePlay::Tick(_float fTimeDelta)
{
	//m_fOffset_Y -= 0.0015f;
}

void CMap_GamePlay::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this); // 알파값 있는 애들때문에 객체 분리해야될거같음
}

HRESULT CMap_GamePlay::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom[m_vecMeshShaderTypes[i]], "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이터를 다 던져야한다. */
		if (FAILED(m_pShaderCom[m_vecMeshShaderTypes[i]]->Begin(m_vecMeshShaderPassIndices[i])))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CMap_GamePlay::SetUp_MeshShaderPassIndices(_uint iNumMeshes)
{
	m_vecMeshShaderTypes.resize(iNumMeshes);
	m_vecMeshShaderPassIndices.resize(iNumMeshes);

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		string strTexturName = m_pModelCom->Get_DiffuseTextureName(i);
		if (strTexturName == "pine_Mat")
			m_vecMeshShaderTypes[i] = NO_LIGHT;
	}
}

HRESULT CMap_GamePlay::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Lighted");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader_Lighted"), (CComponent**)&m_pShaderCom[LIGHTED])))
		return E_FAIL;

	wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom[NO_LIGHT])))
		return E_FAIL;

	return S_OK;
}

HRESULT CMap_GamePlay::Bind_ShaderResources()
{
	for (_int i = 0; i < SHADER_END; i++)
	{
		if (nullptr == m_pShaderCom[i])
			return E_FAIL;
	}

	const LIGHT_DESC* pLightDesc = m_pGameInstance->Get_LightDesc(0);
	if (nullptr == pLightDesc)
		return E_FAIL;

	for (_int i = 0; i <= LIGHTED; i++)
	{
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;
		if (FAILED(m_pShaderCom[i]->Bind_RawValue("g_fCamFar", m_pGameInstance->Get_CamFarPtr(), sizeof(_float))))
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

CMap_GamePlay* CMap_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMap_GamePlay* pInstance = new CMap_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMap_GamePlay"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMap_GamePlay::Clone(void* pArg)
{
	CMap_GamePlay* pInstance = new CMap_GamePlay(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CMap_GamePlay"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMap_GamePlay::Free()
{
	__super::Free();
	for (_int j = 0; j < SHADER_END; j++)
		Safe_Release(m_pShaderCom[j]);
	Safe_Release(m_pModelCom);
}
