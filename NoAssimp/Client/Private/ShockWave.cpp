#include "stdafx.h"
#include "ShockWave.h"

CShockWave::CShockWave(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CShockWave::CShockWave(const CShockWave& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CShockWave::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CShockWave::Initialize(void* pArg)
{
	SHOCKWAVE_DESC GameObjectDesc = {};
	if (pArg)
		GameObjectDesc = *(SHOCKWAVE_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	m_iShockWaveIndex = GameObjectDesc.iShockWaveIndex;
	m_fDuration = 5.f;

	_float fAngle = 0.1f;
	m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), fAngle * GameObjectDesc.iRotation);

	//_vector vOffset = XMVectorSet(0, 3.5f, 0, 0) - m_pTransformCom->Get_State_Vector(CTransform::STATE_LOOK) * 2.f;
	//m_pTransformCom->Set_Pos(m_pTransformCom->Get_Pos_Vector() + vOffset);

	m_pGameInstance->StopSound(SKILL3);
	m_pGameInstance->Play_Sound(TEXT("Spell0.mp3"), SKILL3, 0.45f);

	return S_OK;
}

void CShockWave::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;
	if (m_fTime > m_fDuration)
		Set_Dead();

	m_pTransformCom->Go_Straight(fTimeDelta * 9.f);

	m_pVIBufferCom->Check_LifeTimes(fTimeDelta);

	if (m_pColliderCom != nullptr)
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CShockWave::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONLIGHT, this);
}

HRESULT CShockWave::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(6))) // ShockWave
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pGameInstance->Add_DebugComponents(m_pColliderCom);
#endif 

	return S_OK;
}

HRESULT CShockWave::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxInstance_Point"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_NewTex"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom[TEX_DEFAULT])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_SmokePuff"),
		TEXT("Com_Texture_Mask"), (CComponent**)&m_pTextureCom[TEX_MASK])))
		return E_FAIL;

	/* For.Com_VIBuffer */
	wstring wstrPrototypeTag = TEXT("Prototype_Component_VIBuffer_Instance_Point_ShockWave");
	if (FAILED(__super::Add_Component(eLevel, wstrPrototypeTag, TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	/* For.Com_Collider */
	CBounding_OBB::BOUNDING_OBB_DESC		ColliderDesc{};
	ColliderDesc.vSize = _float3(25, 11.f, 6.5f);
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CShockWave::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_DEFAULT]->Bind_ShaderResource(m_pShaderCom, "g_Texture")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture")))
		return E_FAIL;


	return S_OK;
}

CShockWave* CShockWave::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CShockWave* pInstance = new CShockWave(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CShockWave"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CShockWave::Clone(void* pArg)
{
	CShockWave* pInstance = new CShockWave(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CShockWave"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CShockWave::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pVIBufferCom);
	for (_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pShaderCom);
}
