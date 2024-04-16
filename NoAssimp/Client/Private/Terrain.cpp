#include "stdafx.h"
#include "Terrain.h"
#include "Player.h"

CTerrain::CTerrain(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTerrain::CTerrain(const CTerrain & rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTerrain::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTerrain::Initialize(void * pArg)
{
	TERRAIN_DESC		GameObjectDesc{};
	if (pArg != nullptr)
		GameObjectDesc = *(TERRAIN_DESC*)pArg;
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_iGridIndex = GameObjectDesc.iGridIndex;
	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL; 

	if (FAILED(Add_Components((LEVEL)GameObjectDesc.iLevel)))
		return E_FAIL;	
	
	return S_OK;
}

void CTerrain::Tick(_float fTimeDelta)
{
	_int iCurLevel = *m_pGameInstance->Get_CurrentLevelID();
	if (iCurLevel == LEVEL_SLIMEBOSS)
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
		if (nullptr == pPlayer)
			return;
		if (true == pPlayer->Get_InSlimeBossRoom())
		{
			m_bHide = true;
			return;
		}
		else
			m_bHide = false;
			
	}

	if (m_pNavigationCom != nullptr)
		m_pNavigationCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CTerrain::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTerrain::Render()
{
	if (m_bHide)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(m_iPassIndex)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_pNavigationCom != nullptr && m_pGameInstance->Get_NavigationMode() == false)
		m_pNavigationCom->Render();
#endif

	return S_OK;
}

HRESULT CTerrain::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (LEVEL_SLIMEBOSS == eLevel) {
		if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex_LevelSlimeBoss"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
			return E_FAIL;
	}
	else {
		if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex"),
			TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
			return E_FAIL;
	}

	/* For.Com_Texture_Diffuse */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Ground"),
		TEXT("Com_Texture_Diffuse"), (CComponent**)&m_pTextureCom[TYPE_DIFFUSE])))
		return E_FAIL;

	/* For.Com_Texture_Mask */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Mask"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TYPE_MASK])))
		return E_FAIL;

	/* For.Com_Texture_Water */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Water"),
		TEXT("Com_Texture_Water"), (CComponent**)&m_pTextureCom[TYPE_WATER])))
		return E_FAIL;
	
	wstring wstrVIBufferTerrain = TEXT("Prototype_Component_VIBuffer_Terrain");
	if (eLevel == LEVEL_GAMEPLAY)
		wstrVIBufferTerrain += TEXT("_GamePlay");
	else if (eLevel == LEVEL_SLIMEBOSS)
		wstrVIBufferTerrain += TEXT("_SlimeBoss");
	else if(eLevel == LEVEL_WIZARD)
		wstrVIBufferTerrain += TEXT("_Wizard");

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(eLevel, wstrVIBufferTerrain, TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	if (eLevel == LEVEL_GAMEPLAY || eLevel == LEVEL_SLIMEBOSS) {
		/* For.Com_Navigation */
		if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Navigation" + to_wstring(eLevel)),
			TEXT("Com_Navigation"), (CComponent**)&m_pNavigationCom)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CTerrain::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;	

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	
	if (FAILED(m_pTextureCom[TYPE_DIFFUSE]->Bind_ShaderResources(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TYPE_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TYPE_WATER]->Bind_ShaderResource(m_pShaderCom, "g_WaterTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;

	return S_OK;
}

CTerrain * CTerrain::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTerrain*		pInstance = new CTerrain(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTerrain"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject * CTerrain::Clone(void * pArg)
{
	CTerrain*		pInstance = new CTerrain(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTerrain"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTerrain::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);
	Safe_Release(m_pShaderCom);
	for (size_t j = 0; j < TYPE_END; j++)
		Safe_Release(m_pTextureCom[j]);
	Safe_Release(m_pVIBufferCom);
}
