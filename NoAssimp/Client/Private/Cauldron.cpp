#include "stdafx.h"
#include "Cauldron.h"

CCauldron::CCauldron(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CCauldron::CCauldron(const CCauldron& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CCauldron::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCauldron::Initialize(void* pArg)
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

	m_iLevel = GameObjectDesc.iLevel;

	return S_OK;
}

void CCauldron::Tick(_float fTimeDelta)
{
	_float fHeight = {};
	if (*m_pGameInstance->Get_CurrentLevelID() == LEVEL_GAMEPLAY)
		fHeight = 9.5f;

	if (m_pTransformCom->Get_State_Float4(CTransform::STATE_POSITION).y <= fHeight)
		m_pTransformCom->Fall(-fTimeDelta * 2.5f);
}

void CCauldron::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CCauldron::Render()
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

	return S_OK;
}

HRESULT CCauldron::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel_Lighted");
	if (FAILED(__super::Add_Component(*m_pGameInstance->Get_CurrentLevelID(), wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(*m_pGameInstance->Get_CurrentLevelID(), wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CCauldron::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCamFar", m_pGameInstance->Get_CamFarPtr(), sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CCauldron* CCauldron::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCauldron* pInstance = new CCauldron(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CCauldron"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CCauldron::Clone(void* pArg)
{
	CCauldron* pInstance = new CCauldron(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CCauldron"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CCauldron::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
