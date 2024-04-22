#include "stdafx.h"
#include "Body.h"
#include "Bone.h"

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
	m_pIsJumping = BodyDesc.pIsJumping;
	if (FAILED(__super::Initialize(&BodyDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(BodyDesc.iLevel), BodyDesc.wstrModelTag)))
		return E_FAIL;

	if(m_pModelCom != nullptr)
		Initialize_AnimSpeed();
	
	m_pModelCom->Set_Animation(m_pModelCom->Get_AnimIndex("Spear_Idle"), true);

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
	m_pModelCom->Play_Animation(fTimeDelta);
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
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
	m_pModelCom->Set_LerpTime(0.08f);

	string strWeapon, strState;

	switch (*m_pWeapon)
	{
	case WPN_SPEAR:
		strWeapon = "Spear_";
		break;
	case WPN_RIFLE:
		strWeapon = "Rifle_";
		break;
	case WPN_SCYTHE:
		strWeapon = "Scythe_";
		break;
	}
	
	switch (*m_pState)
	{
	case IDLE:
		strState = "Idle";
		break;
	case RUN:
		strState = "Run";
		break;
	case JUMP:
		strState = "Jump";
		break;
	case ROLL:
		strWeapon = "";
		strState = "Roll";
		break;
	case ATTACK1:
		strState = "Atk1";
		break;
	case ATTACK2:
		strState = "Atk2";
		break;
	case ATTACK3:
		strState = "Atk3";
		break;
	case AIR_ATTACK:
		strState = "AirAttack";
		if (*m_pWeapon == WPN_SCYTHE || *m_pWeapon == WPN_SPEAR)
			strState += "_Start";
		break;
	case AIR_ATTACK_END:
		strState = "AirAttack_End";
		if(*m_pWeapon == WPN_SCYTHE)
			m_pModelCom->Set_LerpTime(0.105f);
		break;
	case ROLL_ATTACK:
		strState = "Roll";
		if (*m_pWeapon == WPN_SPEAR)
			strState = "Atk3";
		break;
	case HIGHFALL:
		strWeapon = "";
		strState = "HighFall";
		break;
	case PUSHING:
		strWeapon = "";
		strState = "Pushing";
		break;
	}

	_bool bLoop;
	if ((*m_pState == IDLE) || (*m_pState == RUN) || (*m_pState == HIGHFALL) || (*m_pState == PUSHING))
		bLoop = true;
	else
		bLoop = false;

	if (*m_pState == IDLE && *m_pIsJumping == true)
		m_pModelCom->Play_FinalFrame(strWeapon + "Jump");
	else
		m_pModelCom->Change_Animation(strWeapon + strState, bLoop);
}

void CBody::Initialize_AnimSpeed()
{
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Spear_AirAttack_Start"), 4.4f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Rifle_AirAttack"), 3.2f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Scythe_AirAttack_Start"), 3.8f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Scythe_AirAttack_End"), 3.3f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Spear_Roll"), 1.8f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Rifle_Roll"), 3.45f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Scythe_Roll"), 2.5f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Roll"), 3.45f);
	
	for(int i = 1; i< 4; i++)
		m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Spear_Atk" + to_string(i)), 2.34f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Spear_Atk3"), 2.5f);


	for (int i = 1; i < 4; i++)
		m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Rifle_Atk" + to_string(i)), 2.7f);

	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Scythe_Atk1"), 5.4f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Scythe_Atk2"), 4.6f);
	m_pModelCom->Set_AnimSpeed(m_pModelCom->Get_AnimIndex("Scythe_Atk3"), 3.6f);
}

HRESULT CBody::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fCamFar", m_pGameInstance->Get_CamFarPtr(), sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CBody* CBody::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBody* pInstance = new CBody(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CBody"));
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBody::Clone(void* pArg)
{
	CBody* pInstance = new CBody(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CBody"));
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
