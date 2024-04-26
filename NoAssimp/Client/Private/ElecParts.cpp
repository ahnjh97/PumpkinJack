#include "stdafx.h"
#include "ElecParts.h"

CElecParts::CElecParts(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CElecParts::CElecParts(const CElecParts& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CElecParts::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElecParts::Initialize(void* pArg)
{
	ELECPARTS_DESC tElecPartsDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		tElecPartsDesc = *(ELECPARTS_DESC*)pArg;

	m_pTargetWorldMat = tElecPartsDesc.pTargetWorldMat;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)tElecPartsDesc.iLevel, tElecPartsDesc.eElecEffect)))
		return E_FAIL;


	if (tElecPartsDesc.eElecEffect == ELEC_HOLE) {
		_vector scale, rotation, translation;
		XMMatrixDecompose(&scale, &rotation, &translation, XMLoadFloat4x4(m_pTargetWorldMat));
		_matrix matRotation= XMMatrixRotationQuaternion(rotation);

		_float4x4 matWorld = {};
		::XMStoreFloat4x4(&matWorld, matRotation);
		m_pTransformCom->Set_WorldMatrix(matWorld);
	}

	if (m_pTargetWorldMat != nullptr)
		m_pTransformCom->Set_Pos(XMVectorSet(m_pTargetWorldMat->_41, m_pTargetWorldMat->_42, m_pTargetWorldMat->_43, 1));
	else
		m_pTransformCom->Set_Pos(_float4(tElecPartsDesc.matWorld._41, tElecPartsDesc.matWorld._42, tElecPartsDesc.matWorld._43, 1));

	return S_OK;
}

void CElecParts::Tick(_float fTimeDelta)
{
	if (m_pTargetWorldMat != nullptr) 
		m_pTransformCom->Set_Pos(XMVectorSet(m_pTargetWorldMat->_41, m_pTargetWorldMat->_42, m_pTargetWorldMat->_43, 1));

	m_pVIBufferCom->Check_LifeTimes(fTimeDelta);
}

void CElecParts::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}
HRESULT CElecParts::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(1))) // ElecParts
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CElecParts::Add_Components(LEVEL eLevel, ELEC_EFFECT eElecEffect)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_ElecParts"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_ELECPARTS])))
		return E_FAIL;

	/* For.Com_Texture_Noise */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Noise"),
		TEXT("Com_Texture_Noise"), (CComponent**)&m_pTextureCom[TEX_NOISE])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	wstring wstrPrototypeTag = TEXT("Prototype_Component_VIBuffer_Instance_Point") + to_wstring(eElecEffect);
	if (FAILED(__super::Add_Component(eLevel, wstrPrototypeTag,	TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CElecParts::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pTextureCom[TEX_ELECPARTS]->Bind_ShaderResource(m_pShaderCom, "g_Texture")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture")))
		return E_FAIL;

	return S_OK;
}

CElecParts* CElecParts::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElecParts* pInstance = new CElecParts(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CElecParts"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CElecParts::Clone(void* pArg)
{
	CElecParts* pInstance = new CElecParts(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Clone : CElecParts"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CElecParts::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	for(_int i = 0; i< TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pShaderCom);
}
