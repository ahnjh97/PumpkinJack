#include "stdafx.h"
#include "Dash.h"

CDash::CDash(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPart{ pDevice, pContext }
{
}

CDash::CDash(const CDash& rhs)
	: CPart{ rhs }
{

}

HRESULT CDash::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDash::Initialize(void* pArg)
{
	PART_DESC		DashDesc{};
	if (pArg != nullptr)
		DashDesc = *(PART_DESC*)pArg;

	DashDesc.fSpeedPerSec = 10.f;
	DashDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	
	if (FAILED(__super::Initialize(&DashDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(DashDesc.iLevel))))
		return E_FAIL;

	m_iLevel = DashDesc.iLevel;

	m_fScale = 25.f;
	m_pTransformCom->Set_Scaled(m_fScale, m_fScale, m_fScale);

	if (DashDesc.matWorld._11 == 100) {
		m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_UP), 1);
		m_pTransformCom->Set_Pos(::XMVectorSet(-0.7f, 7.5f, 0, 1));
	}
		
	else
		m_pTransformCom->Set_Pos(::XMVectorSet(1.2f, 7.5f, 0, 1));

	return S_OK;
}

void CDash::Tick(_float fTimeDelta)
{
	m_fTime += fTimeDelta;

	if (m_fTime > 0.15f) {
		if (0 == m_iSpriteIndex) {
			m_pGameInstance->StopSound(PLAYER);
			m_pGameInstance->Play_Sound(TEXT("Dash.mp3"), PLAYER, 0.45f);
		}
			
		m_iSpriteIndex++;
		m_fTime = 0.f;
	}
}

void CDash::Late_Tick(_float fTimeDelta)
{
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
	//m_WorldMatrix = LookAtCam_UpFixed();
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_BLEND, this);
}

HRESULT CDash::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(10))) // Dash
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

_float4x4 CDash::LookAtCam_UpFixed()
{
	_float4x4 matNewWorld{};

	_vector vNewLook = m_pGameInstance->Get_CamPosition_Vector() - XMVectorSet(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43, 1);
	vNewLook = XMVector3Normalize(vNewLook);

	// 새로운 right 벡터 계산
	_vector vUp = XMVectorSet(m_WorldMatrix._21, m_WorldMatrix._22, m_WorldMatrix._23, 0); 
	_vector vRight = XMVector3Cross(vUp, vNewLook);
	vRight = XMVector3Normalize(vRight);

	// 새로운 look 및 right 벡터로 회전 행렬 생성
	_matrix rotationMatrix = XMMatrixIdentity();
	rotationMatrix.r[0] = vRight;
	rotationMatrix.r[1] = XMVectorSet(m_WorldMatrix._21, m_WorldMatrix._22, m_WorldMatrix._23, 0);
	rotationMatrix.r[2] = vNewLook; 

	_matrix matTempWorld = rotationMatrix * XMLoadFloat4x4(&m_WorldMatrix);
	XMStoreFloat4x4(&matNewWorld, matTempWorld);

	return matNewWorld;
}

HRESULT CDash::Add_Components(LEVEL eLevel)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Texture_Dash"),
		TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CDash::Bind_ShaderResources()
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
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTime, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_iSpriteIndex", &m_iSpriteIndex, sizeof(_uint))))
		return E_FAIL;

	return S_OK;
}

CDash* CDash::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CDash* pInstance = new CDash(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CDash"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDash::Clone(void* pArg)
{
	CDash* pInstance = new CDash(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CDash"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CDash::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
