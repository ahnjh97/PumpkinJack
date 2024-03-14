#include "stdafx.h"
#include "Body.h"

CBody::CBody(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPart{ pDevice, pContext }
{
}

CBody::CBody(const CBody& rhs)
	: CPart{ rhs }
{
}

HRESULT CBody::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBody::Initialize(void* pArg)
{
	BODY_DESC BodyDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		BodyDesc = *(BODY_DESC*)pArg;

	BodyDesc.fSpeedPerSec = 25.f;
	BodyDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	m_pState = BodyDesc.pState;
	m_pWeapon = BodyDesc.pWeapon;
	m_pLastAttack = BodyDesc.pLastAttack;
	if (FAILED(__super::Initialize(&BodyDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(BodyDesc.iLevel), BodyDesc.wstrModelTag)))
		return E_FAIL;

	if (!m_pModelCom->IsNonAnimVersion() && m_pModelCom->Get_ModelType() == TYPE_ANIM)
		m_pModelCom->Set_Animation(999, true); // 999: 랜덤 애니메이션

	return S_OK;
}

void CBody::Tick(_float fTimeDelta)
{
	if (m_pModelCom == nullptr)
		return;
	StateToAnimation();
}

void CBody::Late_Tick(_float fTimeDelta)
{
	_float3 vPreHipTrans = m_pModelCom->Get_HipTranslation();
	if ((vPreHipTrans.x < -5000.f) || (vPreHipTrans.y < -5000.f) || (vPreHipTrans.z < -5000.f))
		vPreHipTrans = _float3{ 0.f, 0.f, 0.f };

	m_pModelCom->Play_Animation(fTimeDelta);

	_float3 vHipTrans = m_pModelCom->Get_HipTranslation();

	_float3 vRelativeTranslation = {};
	XMStoreFloat3(&vRelativeTranslation, XMLoadFloat3(&vHipTrans) - XMLoadFloat3(&vPreHipTrans));
	_float fNewScale = m_pModelCom->Get_ModelConfig().fNewScale * 0.9f;

	_matrix matTemp = ::XMMatrixTranslation(vRelativeTranslation.x * fNewScale, vRelativeTranslation.y * fNewScale, vRelativeTranslation.z * fNewScale);
	_matrix matRelative = ::XMMatrixMultiply(m_pTransformCom->Get_WorldMatrix(), matTemp);
	_float4x4 matWorld = {};
	XMStoreFloat4x4(&matWorld, matRelative);
	m_pTransformCom->Set_WorldMatrix(matWorld);
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pModelCom->Reset_Animation();
}

HRESULT CBody::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CBody::StateToAnimation()
{
	string Jack_ = "Jack_";
	string _Anim = "_Anim";
	string _Jack = "_Jack";

	if (*m_pWeapon == WPN_SPEAR)
	{
		if (*m_pState == IDLE)
			m_pModelCom->Find_Animation(Jack_ + "Spear_Idle" + _Anim + _Jack, true);
		if (*m_pState == RUN)
			m_pModelCom->Find_Animation(Jack_ + "Spear_Run2" + _Anim + _Jack, true);
		if (*m_pState == JUMP)
			m_pModelCom->Find_Animation(Jack_ + "Spear_Jump" + _Anim + _Jack, false);
		if (*m_pState == ATTACK1)
			m_pModelCom->Find_Animation(Jack_ + "Spear_Atk2" + _Anim + _Jack, false);
		if (*m_pState == ATTACK2)
			m_pModelCom->Find_Animation(Jack_ + "Spear_Atk5" + _Anim + _Jack, false);
		if (*m_pState == ATTACK3)
			m_pModelCom->Find_Animation(Jack_ + "Spear_Atk3" + _Anim + _Jack, false);
		if (*m_pState == AIR_ATTACK)
			m_pModelCom->Find_Animation(Jack_ + "Spear_AirAttack" + _Anim + _Jack, false);
		if (*m_pState == AIR_ATTACK_END)
			m_pModelCom->Find_Animation(Jack_ + "Spear_AirAttack_End" + _Anim, false);
	}
	if (*m_pWeapon == WPN_RIFLE)
	{
		if (*m_pState == IDLE)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_Idle" + _Anim + _Jack, true);
		if (*m_pState == RUN)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_Run" + _Anim + _Jack, true);
		if (*m_pState == JUMP)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_Jump" + _Anim + _Jack, false);
		if (*m_pState == ATTACK1)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_Shoot_1" + _Anim + _Jack, false);
		if (*m_pState == ATTACK2)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_Shoot_2" + _Anim + _Jack, false);
		if (*m_pState == ATTACK3)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_Shoot_3" + _Anim + _Jack, false);
		if (*m_pState == AIR_ATTACK_END)
			m_pModelCom->Find_Animation(Jack_ + "Rifle_AirAttackShort" + _Anim, false);
	}
	
}

HRESULT CBody::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = {};
	if (m_pModelCom->Get_ModelType() == TYPE_ANIM)
		wstrShaderTag = TEXT("Prototype_Component_Shader_VtxAnimModel");
	else
		wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");

	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CBody::Bind_ShaderResources()
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

CBody* CBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBody* pInstance = new CBody(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CBody"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBody::Clone(void* pArg)
{
	CBody* pInstance = new CBody(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CBody"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBody::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
