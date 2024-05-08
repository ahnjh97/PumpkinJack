#include "stdafx.h"
#include "JumpEffect.h"

CJumpEffect::CJumpEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CJumpEffect::CJumpEffect(const CJumpEffect& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CJumpEffect::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CJumpEffect::Initialize(void* pArg)
{
    GAMEOBJECT_DESC* pGameObjectDesc = (GAMEOBJECT_DESC*)pArg;

    pGameObjectDesc->fSpeedPerSec = 10.f;
    pGameObjectDesc->fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components(LEVEL(pGameObjectDesc->iLevel))))
        return E_FAIL;

    return S_OK;
}

void CJumpEffect::Tick(_float fTimeDelta)
{
    m_fTime += fTimeDelta;

    if (m_fTime > m_fDuration)
        Set_Dead();

    m_pVIBufferCom->Check_LifeTimes(fTimeDelta);
}

void CJumpEffect::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CJumpEffect::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(4)))  // JumpEffect
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Buffers()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CJumpEffect::Add_Components(LEVEL eLevel)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_SmokePuff"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_SMOKE])))
        return E_FAIL;

    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Cloud"),
        TEXT("Com_Texture_Noise"), (CComponent**)&m_pTextureCom[TEX_NOISE])))
        return E_FAIL;

    /* For.Com_VIBuffer */
    if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_VIBuffer_Instance_Point_Jump"),
        TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CJumpEffect::Bind_ShaderResources()
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
    if (FAILED(m_pTextureCom[TEX_SMOKE]->Bind_ShaderResource(m_pShaderCom, "g_Texture")))
        return E_FAIL;
    if (FAILED(m_pTextureCom[TEX_NOISE]->Bind_ShaderResource(m_pShaderCom, "g_NoiseTexture")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
        return E_FAIL;

    return S_OK;
}

CJumpEffect* CJumpEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CJumpEffect* pInstance = new CJumpEffect(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CJumpEffect"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CJumpEffect::Clone(void* pArg)
{
    CJumpEffect* pInstance = new CJumpEffect(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Create : CJumpEffect"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

void CJumpEffect::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    for (_int i = 0; i < TEX_END; i++)
        Safe_Release(m_pTextureCom[i]);
    Safe_Release(m_pShaderCom);
}
