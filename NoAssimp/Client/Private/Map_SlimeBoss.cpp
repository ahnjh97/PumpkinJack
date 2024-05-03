#include "stdafx.h"
#include "Map_SlimeBoss.h"
#include "Player.h"

CMap_SlimeBoss::CMap_SlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CMap_SlimeBoss::CMap_SlimeBoss(const CMap_SlimeBoss& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CMap_SlimeBoss::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMap_SlimeBoss::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_iLevel = LEVEL_SLIMEBOSS;

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();
	SetUp_MeshShaderPassIndices(iNumMeshes);

	return S_OK;
}

void CMap_SlimeBoss::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
}

void CMap_SlimeBoss::Late_Tick(_float fTimeDelta)
{
	if(true == Check_InSlimeBossRoom())
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this); // 알파값 있는 애들때문에 객체 분리해야될거같음
	else
		m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this); // 알파값 있는 애들때문에 객체 분리해야될거같음
}

HRESULT CMap_SlimeBoss::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_bool bInSlimeBossRoom = Check_InSlimeBossRoom();
	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom[m_vecMeshShaderTypes[i]], "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이터를 다 던져야한다. */
		if (FAILED(m_pShaderCom[m_vecMeshShaderTypes[i]]->Begin(m_vecMeshShaderPassIndices[i])))
			return E_FAIL;

		if (true == bInSlimeBossRoom) {
			if(m_vecMeshShaderTypes[i] == NO_LIGHT)
				m_pModelCom->Render(i);
		}
			
		else
			m_pModelCom->Render(i);
	}

	return S_OK;
}

void CMap_SlimeBoss::SetUp_MeshShaderPassIndices(_uint iNumMeshes)
{
	m_vecMeshShaderTypes.resize(iNumMeshes);
	m_vecMeshShaderPassIndices.resize(iNumMeshes);
	vector<_uint> vecSlimeBossRoom;

	for (_uint i = 0; i < iNumMeshes; i++)
	{
		string strTextureName = m_pModelCom->Get_DiffuseTextureName(i);
		if (strTextureName == "pine_Mat")
			m_vecMeshShaderTypes[i] = NO_LIGHT;
		else if (strTextureName == "slimeBossTex2_Mat")
			vecSlimeBossRoom.push_back(i);
	}

	for (auto& SBR : vecSlimeBossRoom)
		m_vecMeshShaderTypes[SBR] = NO_LIGHT;

	// 크기가 더 작은메쉬를 이펙트용으로 할당
	if (m_pModelCom->Get_MeshOffsetX(vecSlimeBossRoom.front()) < m_pModelCom->Get_MeshOffsetX(vecSlimeBossRoom.back()))
		m_vecMeshShaderPassIndices[vecSlimeBossRoom.front()] = 2; // SLIMEBOSSROOM
	else
		m_vecMeshShaderPassIndices[vecSlimeBossRoom.back()] = 2; // SLIMEBOSSROOM
}

_bool CMap_SlimeBoss::Check_InSlimeBossRoom()
{
	CPlayer* pPlayer = dynamic_cast<CPlayer*>(m_pGameInstance->Get_Player(LEVEL_SLIMEBOSS));
	if (nullptr == pPlayer)
		return false;

	Safe_AddRef(pPlayer);

	_bool bIsInSlimeBoss = pPlayer->Get_InSlimeBossRoom();

	Safe_Release(pPlayer);

	return bIsInSlimeBoss;
}

HRESULT CMap_SlimeBoss::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
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

	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Water"),
		TEXT("Com_Texture_Water"), (CComponent**)&m_pTextureCom[TEX_WATER])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_waterCausticText"),
		TEXT("Com_Texture_Caustic"), (CComponent**)&m_pTextureCom[TEX_CAUSTIC])))
		return E_FAIL;

	return S_OK;
}

HRESULT CMap_SlimeBoss::Bind_ShaderResources()
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

	if (FAILED(m_pTextureCom[TEX_WATER]->Bind_ShaderResource(m_pShaderCom[NO_LIGHT], "g_WaterTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TEX_CAUSTIC]->Bind_ShaderResource(m_pShaderCom[NO_LIGHT], "g_CausticTexture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom[NO_LIGHT]->Bind_RawValue("g_fTime", &m_fTime, sizeof(m_fTime))))
		return E_FAIL;

	return S_OK;
}

CMap_SlimeBoss* CMap_SlimeBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMap_SlimeBoss* pInstance = new CMap_SlimeBoss(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CMap_SlimeBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMap_SlimeBoss::Clone(void* pArg)
{
	CMap_SlimeBoss* pInstance = new CMap_SlimeBoss(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CMap_SlimeBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMap_SlimeBoss::Free()
{
	__super::Free();
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	for(_int j = 0; j< SHADER_END; j++)
		Safe_Release(m_pShaderCom[j]);
	Safe_Release(m_pModelCom);
}
