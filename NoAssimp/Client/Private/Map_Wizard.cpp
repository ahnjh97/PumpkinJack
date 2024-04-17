#include "stdafx.h"
#include "Map_Wizard.h"

CMap_Wizard::CMap_Wizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMap_Wizard::CMap_Wizard(const CMap_Wizard& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CMap_Wizard::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMap_Wizard::Initialize(void* pArg)
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

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	SetUp_MeshShaderPassIndices(iNumMeshes);

	return S_OK;
}

void CMap_Wizard::Tick(_float fTimeDelta)
{
	m_fOffset_Y += 0.004f;
}

void CMap_Wizard::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this); // 알파값 있는 애들때문에 객체 분리해야될거같음
}

HRESULT CMap_Wizard::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(m_vecMeshShaderPassIndices[i])))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CMap_Wizard::SetUp_MeshShaderPassIndices(_uint iNumMeshes)
{
	m_vecMeshShaderPassIndices.resize(iNumMeshes);
	for (_uint i = 0; i < iNumMeshes; i++)
	{
		string strDiffuseTexName = m_pModelCom->Get_DiffuseTextureName(i);
		if (strDiffuseTexName == "T_Earth_Mat")
			m_vecMeshShaderPassIndices[i] = 0; // NoCull
		else if (strDiffuseTexName == "ArenaFireMat")
			m_vecMeshShaderPassIndices[i] = 4; // ArenaFire
	}
}

HRESULT CMap_Wizard::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	wstring wstrTextureTag = TEXT("Prototype_Component_Texture_VoronoiNoise");
	if (FAILED(__super::Add_Component(eLevel, wstrTextureTag, TEXT("Com_Texture_Noise"), (CComponent**)&m_pTextureCom[TEX_NOISE])))
		return E_FAIL;

	return S_OK;
}

HRESULT CMap_Wizard::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fOffset_Y", &m_fOffset_Y, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture")))
		return E_FAIL;
	return S_OK;
}

CMap_Wizard* CMap_Wizard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMap_Wizard* pInstance = new CMap_Wizard(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMap_Wizard"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CMap_Wizard::Clone(void* pArg)
{
	CMap_Wizard* pInstance = new CMap_Wizard(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CMap_Wizard"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMap_Wizard::Free()
{
	__super::Free();
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
