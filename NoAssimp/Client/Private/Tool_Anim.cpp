#include "stdafx.h"
#include "Tool_Anim.h"

CTool_Anim::CTool_Anim(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTool_Anim::CTool_Anim(const CTool_Anim& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTool_Anim::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTool_Anim::Initialize(void* pArg)
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


	return S_OK;
}

void CTool_Anim::Tick(_float fTimeDelta)
{
}

void CTool_Anim::Late_Tick(_float fTimeDelta)
{
	_float3 vPreHipTrans = m_pModelCom->Get_HipTranslation();
	if ((vPreHipTrans.x < -5000.f) || (vPreHipTrans.y < -5000.f) || (vPreHipTrans.z < -5000.f))
		vPreHipTrans = _float3{ 0.f, 0.f, 0.f };

	m_pModelCom->Play_Animation(fTimeDelta);

	_float3 vHipTrans = m_pModelCom->Get_HipTranslation();

	_float3 vRelativeTranslation = {};
	XMStoreFloat3(&vRelativeTranslation, XMLoadFloat3(&vHipTrans) - XMLoadFloat3(&vPreHipTrans));
	_float fNewScale = m_pModelCom->Get_ModelConfig().fNewScale;
	/*fNewScale = 1.f;*/

	_matrix matTemp = XMMatrixTranslation(vRelativeTranslation.x * fNewScale, vRelativeTranslation.y * fNewScale, vRelativeTranslation.z * fNewScale);
	_matrix matRelative = XMMatrixMultiply(m_pTransformCom->Get_WorldMatrix(), matTemp);
	_float4x4 matWorld = {};
	XMStoreFloat4x4(&matWorld, matRelative);
	m_pTransformCom->Set_WorldMatrix(matWorld);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTool_Anim::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", i, aiTextureType_DIFFUSE);
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;
		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;
		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CTool_Anim::Add_Components(const wstring& _wstrModelTag)
{
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(LEVEL_MAPTOOL, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTool_Anim::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;



	return S_OK;
}

CTool_Anim* CTool_Anim::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTool_Anim* pInstance = new CTool_Anim(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTool_Anim"));
		Safe_Release(pInstance);
	}
	return pInstance;

}

CGameObject* CTool_Anim::Clone(void* pArg)
{
	CTool_Anim* pInstance = new CTool_Anim(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTool_Anim"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTool_Anim::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
