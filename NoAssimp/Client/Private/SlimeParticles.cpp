#include "stdafx.h"
#include "SlimeParticles.h"

CSlimeParticles::CSlimeParticles(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSlimeParticles::CSlimeParticles(const CSlimeParticles& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CSlimeParticles::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSlimeParticles::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {};
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	m_fDuration = 0.3f;

	if(GameObjectDesc.iLevel == LEVEL_EFFECTTOOL)
		m_pTransformCom->Set_Pos(XMVectorSet(m_pGameInstance->RandomNumber(-30, 0), 0, 0, 1));

	_vector vOffset = XMVectorSet(0, 3.5f, 0, 0) - m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 2.f;
	m_pTransformCom->Set_Pos(m_pTransformCom->Get_Pos_Vector() + vOffset);

	return S_OK;
}

void CSlimeParticles::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
	if (m_fTime > m_fDuration)
		Set_Dead();

	m_pVIBufferCom->Spread(fTimeDelta);
}

void CSlimeParticles::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CSlimeParticles::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(5))) // SlimeParticles
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeParticles::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Slime"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_SLIMETEX])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_Particle"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TEX_MASK])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	wstring wstrPrototypeTag = TEXT("Prototype_Component_VIBuffer_Instance_Point_SlimeParticles");
	if (FAILED(__super::Add_Component(eLevel, wstrPrototypeTag, TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CSlimeParticles::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition_Float4(), sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_SLIMETEX]->Bind_ShaderResource(m_pShaderCom, "g_Texture")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture")))
		return E_FAIL;

	return S_OK;
}

CSlimeParticles* CSlimeParticles::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSlimeParticles* pInstance = new CSlimeParticles(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeParticles"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSlimeParticles::Clone(void* pArg)
{
	CSlimeParticles* pInstance = new CSlimeParticles(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CSlimeParticles"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSlimeParticles::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	for(_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pShaderCom);
}
