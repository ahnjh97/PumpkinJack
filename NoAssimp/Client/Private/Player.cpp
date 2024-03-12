#include "stdafx.h"
#include "Player.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CPlayer::CPlayer(const CPlayer& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CPlayer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPlayer::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 20.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	//m_pModelCom->Set_Animation(11, true); // 999: 랜덤 애니메이션
	if (!m_pModelCom->IsNonAnimVersion() && m_pModelCom->Get_ModelType() == TYPE_ANIM)
		m_pModelCom->Set_Animation(999, true); // 999: 랜덤 애니메이션


	return S_OK;
}

void CPlayer::Tick(_float fTimeDelta)
{
	if (m_bIsJumping)
		m_fTempJumpTime += fTimeDelta;
	if (m_fTempJumpTime > 1.2f)
	{
		m_bIsJumping = false;
		m_fTempJumpTime = 0.f;
	}
		
	if (m_pCamera == nullptr)
	{
		m_pCamera = dynamic_cast<CCamera_Jack*>(m_pGameInstance->Get_GameObject(m_pGameInstance->Get_CurrentLevelID(), TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Jack")));
		Safe_AddRef(m_pCamera);
	}
		
	OnKeyInput(fTimeDelta);
	StateToAnimation();
	Update_CamPos();
	_float fAngle = Calculate_Angle();
	Turn_Body(fAngle);
}

void CPlayer::Late_Tick(_float fTimeDelta)
{
	_float3 vPreHipTrans = m_pModelCom->Get_HipTranslation();
	if ((vPreHipTrans.x < -5000.f) || (vPreHipTrans.y < -5000.f) || (vPreHipTrans.z < -5000.f))
		vPreHipTrans = _float3{ 0.f, 0.f, 0.f };

	m_pModelCom->Play_Animation(fTimeDelta);

	_float3 vHipTrans = m_pModelCom->Get_HipTranslation();

	_float3 vRelativeTranslation = {};
	XMStoreFloat3(&vRelativeTranslation, XMLoadFloat3(&vHipTrans) - XMLoadFloat3(&vPreHipTrans));
	_float fNewScale = m_pModelCom->Get_ModelConfig().fNewScale;
	/*fNewScale = 1.f;*/

	_matrix matTemp = ::XMMatrixTranslation(vRelativeTranslation.x * fNewScale, vRelativeTranslation.y * fNewScale, vRelativeTranslation.z * fNewScale);
	_matrix matRelative = ::XMMatrixMultiply(m_pTransformCom->Get_WorldMatrix(), matTemp);
	_float4x4 matWorld = {};
	XMStoreFloat4x4(&matWorld, matRelative);
	m_pTransformCom->Set_WorldMatrix(matWorld);

	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);

	m_vPreTargetDir = m_vTargetDir;
}

HRESULT CPlayer::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", i, aiTextureType_DIFFUSE);

		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;


		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CPlayer::OnKeyInput(_float fTimeDelta)
{
	if (m_pCamera == nullptr)
		return;

	CTransform* pCamTransform = dynamic_cast<CTransform*>(m_pCamera->Get_Component(g_strTransformTag));
	if (pCamTransform == nullptr)
		return;
	_vector vCamLook = pCamTransform->Get_State_Vector(CTransform::STATE_LOOK);
	_vector vCamRight = pCamTransform->Get_State_Vector(CTransform::STATE_RIGHT);

	if (m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80)
	{
		m_pTransformCom->Go_Straight(fTimeDelta, 1, vCamLook);
		if(!m_bIsJumping)
			m_eState = RUN;
		XMStoreFloat3(&m_vTargetDir, vCamLook);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80)
	{
		m_pTransformCom->Go_Backward(fTimeDelta, 1, vCamLook);
		if (!m_bIsJumping)
			m_eState = RUN;
		XMStoreFloat3(&m_vTargetDir, -vCamLook);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80)
	{
		m_pTransformCom->Go_Left(fTimeDelta, 1, vCamRight);
		if (!m_bIsJumping)
			m_eState = RUN;
		XMStoreFloat3(&m_vTargetDir, -vCamRight);
	}
	if (m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80)
	{
		m_pTransformCom->Go_Right(fTimeDelta, 1, vCamRight);
		if (!m_bIsJumping)
			m_eState = RUN;
		XMStoreFloat3(&m_vTargetDir, vCamRight);
	}
	if (m_pGameInstance->KeyDown(DIK_SPACE))
	{
		m_pTransformCom->Go_Right(fTimeDelta, 1, vCamRight);
		m_eState = JUMP;
		m_bIsJumping = true;
	}
	if (m_pGameInstance->MouseDown(DIMKS_LBUTTON)) // 공속 옵션 넣어야함
	{
		if (m_bIsJumping == true)
		{
			m_eState = AIR_ATTACK;
			m_iLastAttack = 0;
		}
		else if (m_iLastAttack == 0 || m_iLastAttack == 3)
		{
			m_eState = ATTACK1;
			m_iLastAttack = 1;
		}
		else if (m_iLastAttack == 1)
		{
			m_eState = ATTACK2;
			m_iLastAttack = 2;
		}
		else if (m_iLastAttack == 2)
		{
			m_eState = ATTACK3;
			m_iLastAttack = 3;
		}
	}

	if (!(m_pGameInstance->Get_DIKeyState(DIK_W) & 0x80) && !(m_pGameInstance->Get_DIKeyState(DIK_A) & 0x80) &&
		!(m_pGameInstance->Get_DIKeyState(DIK_S) & 0x80) && !(m_pGameInstance->Get_DIKeyState(DIK_D) & 0x80) && 
		!(m_pGameInstance->Get_DIKeyState(DIK_SPACE) & 0x80) && !(m_pGameInstance->MouseDown(DIMKS_LBUTTON)) && 
		!(m_pGameInstance->MouseDown(DIMKS_RBUTTON)))
	{
		m_eState = IDLE;
	}
}
void CPlayer::StateToAnimation()
{
	if (m_pModelCom == nullptr)
		return;
	string Jack_ = "Jack_";
	string _Anim = "_Anim";
	string _Jack = "_Jack";

	string Spear_Run2 = Jack_ + "Spear_Run2" + _Anim + _Jack;
	string Spear_Idle = Jack_ + "Spear_Idle" + _Anim + _Jack;
	string Spear_Jump = Jack_ + "Spear_Jump" + _Anim + _Jack;
	string Spear_Atk2 = Jack_ + "Spear_Atk2" + _Anim + _Jack;
	string Spear_Atk3 = Jack_ + "Spear_Atk3" + _Anim + _Jack;
	string Spear_Atk5 = Jack_ + "Spear_Atk5" + _Anim + _Jack;
	if (m_eWeapon == WPN_SPEAR)
	{
		if (m_eState == IDLE)
		{
			m_pModelCom->Find_Animation(Spear_Idle, true);
		}
		if (m_eState == RUN)
		{
			m_pModelCom->Find_Animation(Spear_Run2, true);
		}
		if (m_eState == JUMP)
		{
			m_pModelCom->Find_Animation(Spear_Jump, true);
		}
		if (m_eState == ATTACK1)
		{
			m_pModelCom->Find_Animation(Spear_Atk2, true);
		}
		if (m_eState == ATTACK2)
		{
			m_pModelCom->Find_Animation(Spear_Atk3, true);
		}
		if (m_eState == ATTACK3)
		{
			m_pModelCom->Find_Animation(Spear_Atk5, true);
		}
	}
}

void CPlayer::Update_CamPos()
{
	if (m_pTransformCom != nullptr)
	{
		_vector vPlayerPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
		m_pCamera->Copy_PlayerPos(vPlayerPos);
	}
}

_float CPlayer::Calculate_Angle()
{
	_float dotProduct = XMVectorGetX(XMVector3Dot(XMLoadFloat3(&m_vPreTargetDir), XMLoadFloat3(&m_vTargetDir)));
	_float magnitudeA = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vPreTargetDir)));
	_float magnitudeB = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vTargetDir)));
	if (magnitudeA == 0.0f || magnitudeB == 0.0f)
		return _float();
		
	_float cosine = dotProduct / (magnitudeA * magnitudeB);
	return acos(cosine);
}

void CPlayer::Turn_Body(_float fAngle)
{
	/*if (abs(fAngle) < XMConvertToRadians(2.f))
		return;
	_vector quaternion = XMQuaternionRotationAxis(XMVectorSet(0,1,0,0), fAngle);

	_float t = 0.5f;
	_vector lerpedQuaternion = XMQuaternionSlerp(XMQuaternionIdentity(), quaternion, t);

	_float4 lerpedResult;
	XMStoreFloat4(&lerpedResult, lerpedQuaternion);
	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	matWorld = XMMatrixRotationQuaternion(lerpedQuaternion) * matWorld;
	_float4x4 matResult = {};
	XMStoreFloat4x4(&matResult, matWorld);
	m_pTransformCom->Set_WorldMatrix(matResult);*/
}

HRESULT CPlayer::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
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

HRESULT CPlayer::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;



	return S_OK;
}

CPlayer* CPlayer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPlayer* pInstance = new CPlayer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CPlayer::Clone(void* pArg)
{
	CPlayer* pInstance = new CPlayer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CPlayer"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();
	Safe_Release(m_pCamera);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	
}
