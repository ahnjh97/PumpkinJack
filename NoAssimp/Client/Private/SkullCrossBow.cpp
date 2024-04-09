#include "stdafx.h"
#include "SkullCrossBow.h"
#include "Bone.h"

CSkullCrossBow::CSkullCrossBow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPart{ pDevice, pContext }
{
}

CSkullCrossBow::CSkullCrossBow(const CSkullCrossBow& rhs)
	: CPart{ rhs }
{

}

HRESULT CSkullCrossBow::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSkullCrossBow::Initialize(void* pArg)
{
	SKULL_CROSSBOW_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(SKULL_CROSSBOW_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	m_pSocket = GameObjectDesc.pSocket;
	m_pState = GameObjectDesc.pState;
	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	m_pModelCom->Set_Animation(0, true); // 999: 랜덤 애니메이션
	m_pModelCom->Set_AnimSpeed(2, 0.35f);

	return S_OK;
}

void CSkullCrossBow::Tick(_float fTimeDelta)
{
}

void CSkullCrossBow::Late_Tick(_float fTimeDelta)
{
	if (BALLISTA_SHOOT == *m_pState) {
		m_pModelCom->Change_Animation_ByIndex(0, true);
		m_pTransformCom->Set_Pos(XMVectorSet(0, 0, 0, 1));
		m_pTransformCom->Rotation(XMVectorSet(1, 0, 0, 0), 0);
	}

	else {
		m_pModelCom->Change_Animation_ByIndex(2, true);
		m_pTransformCom->Set_Pos(XMVectorSet(0, 5.5f, -2.5f, 1));
		m_pTransformCom->Rotation(XMVectorSet(1, 0, 0, 0), XM_PIDIV2);
	}

	_matrix matCombined = XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix());
	_vector vScale, vRot, vTrans;
	XMMatrixDecompose(&vScale, &vRot, &vTrans, matCombined);
	_vector vOffset = XMVectorSet(0.f, 1.f, -18.f, 0.f);
	vTrans += vOffset;
	_vector vNewScale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	_matrix matNew = XMMatrixAffineTransformation(vNewScale, XMVectorZero(), vRot, vTrans);
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * matNew * XMLoadFloat4x4(m_pParentMatrix));

	m_pModelCom->Play_Animation(fTimeDelta);
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CSkullCrossBow::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

HRESULT CSkullCrossBow::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSkullCrossBow::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
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

	return S_OK;
}

CSkullCrossBow* CSkullCrossBow::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSkullCrossBow* pInstance = new CSkullCrossBow(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSkullCrossBow"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CPart* CSkullCrossBow::Clone(void* pArg)
{
	CSkullCrossBow* pInstance = new CSkullCrossBow(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSkullCrossBow"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSkullCrossBow::Free()
{
	__super::Free();

	Safe_Release(m_pSocket);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
