#include "stdafx.h"
#include "ArrowEffect.h"

CArrowEffect::CArrowEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPart{ pDevice, pContext }
{
}

CArrowEffect::CArrowEffect(const CArrowEffect& rhs)
    : CPart{ rhs }
{
}

HRESULT CArrowEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CArrowEffect::Initialize(void* pArg)
{
    PART_DESC* pPartObjectDesc = (PART_DESC*)pArg;

    pPartObjectDesc->fSpeedPerSec = 10.f;
    pPartObjectDesc->fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components(LEVEL(pPartObjectDesc->iLevel))))
        return E_FAIL;

    return S_OK;
}

void CArrowEffect::Tick(_float fTimeDelta)
{
    if (m_pCamera == nullptr)
    {
        m_pCamera = dynamic_cast<CCamera_Jack*>(m_pGameInstance->Get_GameObject(*m_pGameInstance->Get_CurrentLevelID(),
            TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Jack")));

        if (m_pCamera != nullptr)
            Safe_AddRef(m_pCamera);
    }

    CTransform* pCamTransform = { nullptr };
    if(m_pCamera != nullptr)
        pCamTransform = dynamic_cast<CTransform*>(m_pCamera->Get_Component(g_strTransformTag));

    _float4 vCamLook{};
    if (pCamTransform != nullptr)
        vCamLook = pCamTransform->Get_State_Float4(CTransform::STATE_LOOK);

    XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));

    m_WorldMatrix._41 = m_WorldMatrix._41 + vCamLook.x * 1.f;
    m_WorldMatrix._42 = m_WorldMatrix._42 + vCamLook.y * 1.f;
    m_WorldMatrix._43 = m_WorldMatrix._43 + vCamLook.z * 1.f;
}

void CArrowEffect::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CArrowEffect::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(2)))  // ArrowEffect
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Buffers()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CArrowEffect::Add_Components(LEVEL eLevel)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_SmokePuff"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
        return E_FAIL;

    /* For.Com_VIBuffer */
    if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_VIBuffer_Instance_Point_Arrow"),
        TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CArrowEffect::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
        return E_FAIL;
    if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture")))
        return E_FAIL;

    return S_OK;
}

CArrowEffect* CArrowEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CArrowEffect* pInstance = new CArrowEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CArrowEffect"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CArrowEffect::Clone(void* pArg)
{
    CArrowEffect* pInstance = new CArrowEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Create : CArrowEffect"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

void CArrowEffect::Free()
{
    __super::Free();

    Safe_Release(m_pCamera);
    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
}
