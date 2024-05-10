#include "stdafx.h"
#include "SoldierSlash.h"

CSoldierSlash::CSoldierSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPart{ pDevice, pContext }
{
}

CSoldierSlash::CSoldierSlash(const CSoldierSlash& rhs)
    : CPart{ rhs }
{
}

HRESULT CSoldierSlash::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSoldierSlash::Initialize(void* pArg)
{
    PART_DESC* pSoldierSlashDesc = (PART_DESC*)pArg;

    pSoldierSlashDesc->fSpeedPerSec = 10.f;
    pSoldierSlashDesc->fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components(LEVEL(pSoldierSlashDesc->iLevel))))
        return E_FAIL;

    m_iLevel = pSoldierSlashDesc->iLevel;

    _float fScale = 6.4f;
    m_pTransformCom->Set_Scaled(fScale, fScale * 0.6f, fScale * 1.05f);

    m_pTransformCom->Set_Pos(XMVectorSet(0, 5.3f, 0, 1));

    m_fEndTime = 1.8f;
     
    return S_OK;
}

void CSoldierSlash::Tick(_float fTimeDelta)
{
    m_fTime += fTimeDelta;
    m_fSpriteTime += fTimeDelta;

    if(m_fTime > m_fEndTime)
        return;

    if (false == m_bEnabled) {
        if (m_fWaitTime < m_fTime) {
            m_bEnabled = true;
            m_fTime = 0.f;
            m_fSpriteTime = 0.f;
        }
    }

    if (true == m_bEnabled) {
        m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta * 15.5f);

        if (m_fSpriteTime >= m_fEndTime / 6.f) {
            m_iSpriteIndex--;
            m_fSpriteTime = 0.f;
        }

        if (m_iSpriteIndex >= 6)
            m_iSpriteIndex = 6;

        if (m_iSpriteIndex <= 0)
            m_iSpriteIndex = 0;

        m_fPlayRatio = m_fTime / m_fEndTime;
        if (m_fPlayRatio > 1.f)
            m_fPlayRatio = 1.f;
    }
}

void CSoldierSlash::Late_Tick(_float fTimeDelta)
{
    if (true == m_bHide)
        return;

    XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CSoldierSlash::Render()
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
        if (FAILED(m_pShaderCom->Begin(10))) // Slash
            return E_FAIL;

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CSoldierSlash::Add_Components(LEVEL eLevel)
{
    /* For.Com_Model */
    wstring wstrModelTag = TEXT("Prototype_Component_Model_Slash");
    if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxModel"),
        TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Slash"),
        TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
        return E_FAIL;

    return S_OK;
}

HRESULT CSoldierSlash::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_AttackEffect")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iSpriteIndex", &m_iSpriteIndex, sizeof(_uint))))
        return E_FAIL;

    return S_OK;
}

CSoldierSlash* CSoldierSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSoldierSlash* pInstance = new CSoldierSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CSoldierSlash"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSoldierSlash::Clone(void* pArg)
{
    CSoldierSlash* pInstance = new CSoldierSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Create : CSoldierSlash"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSoldierSlash::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pModelCom);
}
