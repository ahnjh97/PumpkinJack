#include "stdafx.h"
#include "Camera_Jack.h"
#include "ImGuizmo.h"

CCamera_Jack::CCamera_Jack(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{

}

CCamera_Jack::CCamera_Jack(const CCamera_Jack& rhs)
	: CCamera{ rhs }
{

}

void CCamera_Jack::Copy_PlayerPos(_fvector vPlayerPos)
{
	XMStoreFloat3(&m_vPlayerPos, vPlayerPos);
}

HRESULT CCamera_Jack::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_Jack::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_DESC* pCameraDesc = (CAMERA_DESC*)pArg;

	m_fMouseSensor = pCameraDesc->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CCamera_Jack::Tick(_float fTimeDelta)
{
	OnMouseInput(fTimeDelta);
	RevolveAroundPlayer(0);
	__super::Bind_PipeLines();
}

void CCamera_Jack::Late_Tick(_float fTimeDelta)
{

}

HRESULT CCamera_Jack::Render()
{
	return S_OK;
}

void CCamera_Jack::OnMouseInput(_float fTimeDelta)
{
	_long	MouseMove = { 0 };
	if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_X))
	{	

		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * MouseMove * m_fMouseSensor);
	}
		
	if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMMS_Y))
		m_pTransformCom->Turn(m_pTransformCom->Get_State_Vector(CTransform::STATE_RIGHT), fTimeDelta * MouseMove * m_fMouseSensor);
}

void CCamera_Jack::RevolveAroundPlayer(_float fAngle)
{
	//_vector vCamPos = m_pTransformCom->Get_State_Vector(CTransform::STATE_POSITION);
	_vector vPlayerPos = XMLoadFloat3(&m_vPlayerPos) + XMVectorSet(0, 20.f, -20.f, 1.f);

	//
	////vRelativePos = XMVector3Transform(vRelativePos, matRotation);

	//_matrix matView = XMMatrixLookToLH(vCamPos, vPlayerPos, XMVectorSet(0, 1, 0, 0));
	//_matrix matViewInverse = XMMatrixInverse(nullptr, matView);
	//_float4x4 matResult = {};
	//XMStoreFloat4x4(&matResult, matViewInverse);
	//m_pTransformCom->Set_WorldMatrix(matResult);

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPlayerPos);
}

CCamera_Jack* CCamera_Jack::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_Jack* pInstance = new CCamera_Jack(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Jack"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CCamera_Jack::Clone(void* pArg)
{
	CCamera_Jack* pInstance = new CCamera_Jack(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Created : CCamera_Jack"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CCamera_Jack::Free()
{
	__super::Free();

}
