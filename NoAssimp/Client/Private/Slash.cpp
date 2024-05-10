#include "stdafx.h"
#include "Slash.h"

CSlash::CSlash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CSlash::CSlash(const CSlash& rhs)
    : CGameObject{ rhs }
{
}

HRESULT CSlash::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CSlash::Initialize(void* pArg)
{
    SLASH_DESC* pGameObjectDesc = (SLASH_DESC*)pArg;

    pGameObjectDesc->fSpeedPerSec = 10.f;
    pGameObjectDesc->fRotationPerSec = XMConvertToRadians(90.0f);

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Add_Components(LEVEL(pGameObjectDesc->iLevel))))
        return E_FAIL;

    m_iWeaponIndex = pGameObjectDesc->eWeapon;
    m_fWaitTime = pGameObjectDesc->fWaitTime;
    m_fEndTime = pGameObjectDesc->fDuration;

    _float fScale = 5.f;
    m_pTransformCom->Set_Scaled(fScale, fScale * 0.6f, fScale);

    _vector vPos = m_pTransformCom->Get_Pos_Vector();

    if (WPN_SPEAR == pGameObjectDesc->eWeapon)
    {
        if (ATTACK1 == pGameObjectDesc->eState) {
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), -0.45f);
            m_pTransformCom->Set_Pos(vPos + XMVectorSet(0, 7.5f, 0, 0) + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.f);
        }
        else if (ATTACK2 == pGameObjectDesc->eState)
        {
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 2);
            m_pTransformCom->Set_Pos(vPos + XMVectorSet(0, 7.f, 0, 0) + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.f);
        }
        else if (AIR_ATTACK == pGameObjectDesc->eState)
        {
            m_pPlayerWorldMat = pGameObjectDesc->pPlayerWorldMat;
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 1);
            m_vOffset = XMVectorSet(0, 7.f, 0, 0) + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.5f;
            m_pTransformCom->Set_Pos(vPos + m_vOffset);
             m_fWaitTime = 0.2f;
        }
    }
    else if (WPN_SCYTHE == pGameObjectDesc->eWeapon)
    {
        if (ATTACK1 == pGameObjectDesc->eState) {
            m_pTransformCom->Set_Scaled(fScale * 1.25f, fScale * 0.6f * 1.25f, fScale * 1.35f);
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), -0.25f);
            m_pTransformCom->Set_Pos(vPos + XMVectorSet(0, 8.f, 0, 0) + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.5f);
            m_fWaitTime = 0.1f;
            m_fEndTime = 0.7f;
        }
        else if (ATTACK2 == pGameObjectDesc->eState)
        {
            m_pTransformCom->Set_Scaled(fScale * 1.25f, fScale * 0.6f * 1.25f, fScale * 1.35f);
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 2.25f);
            m_pTransformCom->Set_Pos(vPos + XMVectorSet(0, 7.5f, 0, 0) + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.5f);
            m_fWaitTime = 0.2f;
            m_fEndTime = 0.45f;
        }
        else if (ATTACK3 == pGameObjectDesc->eState)
        {
            m_pTransformCom->Set_Scaled(fScale * 1.25f, fScale * 0.6f * 1.25f, fScale * 1.35f);
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 1.35f);
            m_pTransformCom->Set_Pos(vPos + XMVectorSet(0, 8.f, 0, 0) + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.5f);
            m_fWaitTime = 0.15f;
            m_fEndTime = 0.45f;
        }
        else if (AIR_ATTACK == pGameObjectDesc->eState)
        {
            m_pTransformCom->Set_Scaled(fScale * 1.25f, fScale * 0.6f * 1.25f, fScale * 1.35f);
            m_pPlayerWorldMat = pGameObjectDesc->pPlayerWorldMat;
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK), 0.7f);
            //m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), .f);
            m_vOffset = XMVectorSet(0, 7.f, 0, 0) - XMLoadFloat4(&pGameObjectDesc->vPlayerRight) * 1.5f 
                + XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 4.f;
            m_pTransformCom->Set_Pos(vPos + m_vOffset);
            m_fWaitTime = 0.45f;
            m_fEndTime = 0.5f;
        }
        else if (ROLL_ATTACK == pGameObjectDesc->eState)
        {
            m_pTransformCom->Set_Scaled(fScale * 1.55f, fScale * 0.6f * 1.55f, fScale * 1.65f);

            m_bTurn = true;
            //m_vOffset = XMLoadFloat4(&pGameObjectDesc->vPlayerLook) * 5.f;
            m_pTransformCom->Set_Pos(vPos + XMVectorSet(0, 4.f, 0, 0));
            m_fWaitTime = 0.35f;
            m_fEndTime = 1.2f;
        }
    }

    return S_OK;
}

void CSlash::Tick(_float fTimeDelta)
{
    m_fTime += fTimeDelta;
    m_fSpriteTime += fTimeDelta;

    if (false == m_bEnabled) {
        if (m_fWaitTime < m_fTime) {
            m_bEnabled = true;
            m_fTime = 0.f;
            m_fSpriteTime = 0.f;
        }
    }
        
    if(true == m_bEnabled) {

        if (m_pPlayerWorldMat != nullptr) 
            m_pTransformCom->Set_Pos(XMVectorSet(m_pPlayerWorldMat->_41, m_pPlayerWorldMat->_42, m_pPlayerWorldMat->_43, 1) + m_vOffset);
        
        if (true == m_bTurn) 
            m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), fTimeDelta * 15.5f);
    
        if (m_fSpriteTime >= m_fEndTime / 6.f) {
            m_iSpriteIndex--;
            m_fSpriteTime = 0.f;
        }
        
        if (m_iSpriteIndex >= 6)
            m_iSpriteIndex = 6;

        if(m_iSpriteIndex <= 0)
            m_iSpriteIndex = 0;

        m_fPlayRatio = m_fTime / m_fEndTime;
        if (m_fPlayRatio > 1.f)
            m_fPlayRatio = 1.f;

        if (m_fTime > m_fEndTime)
            Set_Dead();
    }
}

void CSlash::Late_Tick(_float fTimeDelta)
{
    m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CSlash::Render()
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
        if (FAILED(m_pShaderCom->Begin(9))) // Slash
            return E_FAIL;

        m_pModelCom->Render(i);
    }

    return S_OK;
}

HRESULT CSlash::Add_Components(LEVEL eLevel)
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

HRESULT CSlash::Bind_ShaderResources()
{
    if (nullptr == m_pShaderCom)
        return E_FAIL;

    if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
        return E_FAIL;
    if (FAILED(m_pTextureCom->Bind_ShaderResources(m_pShaderCom, "g_AttackEffect")))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_fPlayRatio", &m_fPlayRatio, sizeof(_float))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iWeaponIndex", &m_iWeaponIndex, sizeof(_uint))))
        return E_FAIL;
    if (FAILED(m_pShaderCom->Bind_RawValue("g_iSpriteIndex", &m_iSpriteIndex, sizeof(_uint))))
        return E_FAIL;

    return S_OK;
}

CSlash* CSlash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CSlash* pInstance = new CSlash(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX(TEXT("Failed To Create : CSlash"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CSlash::Clone(void* pArg)
{
    CSlash* pInstance = new CSlash(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX(TEXT("Failed To Create : CSlash"));

        Safe_Release(pInstance);
    }

    return pInstance;
}

void CSlash::Free()
{
    __super::Free();

    Safe_Release(m_pShaderCom);
    Safe_Release(m_pTextureCom);
    Safe_Release(m_pModelCom);
}
