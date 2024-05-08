#include "stdafx.h"
#include "Bullets.h"

CBullets::CBullets(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CBullets::CBullets(const CBullets& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CBullets::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CBullets::Initialize(void* pArg)
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

void CBullets::Tick(_float fTimeDelta)
{
    m_fTime += fTimeDelta;

    if (m_fTime > m_fDuration)
        Set_Dead();

    m_pVIBufferCom->Spread(fTimeDelta);
}

void CBullets::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CBullets::Render()
{
    if (FAILED(Bind_ShaderResources()))
        return E_FAIL;

    if (FAILED(m_pShaderCom->Begin(5)))  // Bullets
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Bind_Buffers()))
        return E_FAIL;

    if (FAILED(m_pVIBufferCom->Render()))
        return E_FAIL;

    return S_OK;
}

HRESULT CBullets::Add_Components(LEVEL eLevel)
{
    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Bullet"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
        return E_FAIL;

    /* For.Com_VIBuffer */
    if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_VIBuffer_Instance_Point_Bullets"),
        TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CBullets::Bind_ShaderResources()
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
    if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_AttackEffect")))
        return E_FAIL;

    return S_OK;
}

CBullets* CBullets::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CBullets* pInstance = new CBullets(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CBullets"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CBullets::Clone(void* pArg)
{
    CBullets* pInstance = new CBullets(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Create : CBullets"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

void CBullets::Free()
{
    __super::Free();

    Safe_Release(m_pVIBufferCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pShaderCom);
}
