#include "..\Public\Transform.h"
#include "Shader.h"
#include "Navigation.h"

CTransform::CTransform(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CComponent(pDevice, pContext)
{	

}

CTransform::CTransform(const CTransform & rhs)
	: CComponent(rhs)
	, m_WorldMatrix(rhs.m_WorldMatrix)
{

}

void CTransform::Set_Scaled(_float fScaleX, _float fScaleY, _float fScaleZ)
{
	Set_State(STATE_RIGHT, XMVector3Normalize(Get_State_Vector(STATE_RIGHT)) * fScaleX);
	Set_State(STATE_UP, XMVector3Normalize(Get_State_Vector(STATE_UP)) * fScaleY);
	Set_State(STATE_LOOK, XMVector3Normalize(Get_State_Vector(STATE_LOOK)) * fScaleZ);
		
}

HRESULT CTransform::Initialize_Prototype()
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	return S_OK;
}

HRESULT CTransform::Initialize(void * pArg)
{
	if (nullptr != pArg)
	{
		TRANSFORM_DESC*		pDesc = (TRANSFORM_DESC*)pArg;

		m_fSpeedPerSec = pDesc->fSpeedPerSec;
		m_fRotationPerSec = pDesc->fRotationPerSec;
		m_WorldMatrix = pDesc->matWorld;
	}

	return S_OK;
}

HRESULT CTransform::Bind_ShaderResource(CShader * pShader, const _char * pConstantName)
{
	return pShader->Bind_Matrix(pConstantName, &m_WorldMatrix);	
}

void CTransform::Go_Straight(_float fTimeDelta, _uint iNum, _fvector vCamLook)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vLook = Get_State_Vector(STATE_LOOK);

	if (iNum != 0)
	{
		vLook = vCamLook;
		vLook = XMVectorSetY(vLook, 0);
	}
		

	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE_POSITION, vPosition);
}



void CTransform::Go_Backward(_float fTimeDelta, _uint iNum, _fvector vCamLook)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vLook = Get_State_Vector(STATE_LOOK);

	if (iNum != 0)
	{
		vLook = vCamLook;
		vLook = XMVectorSetY(vLook, 0);
	}

	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Left(_float fTimeDelta, _uint iNum, _fvector vCamRight)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vRight = Get_State_Vector(STATE_RIGHT);

	if (iNum != 0)
	{
		vRight = vCamRight;
		vRight = XMVectorSetY(vRight, 0);
	}

	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Go_Right(_float fTimeDelta, _uint iNum, _fvector vCamRight)
{
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	_vector		vRight = Get_State_Vector(STATE_RIGHT);

	if (iNum != 0)
	{
		vRight = vCamRight;
		vRight = XMVectorSetY(vRight, 0);
	}

	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Move(_float fTimeDelta, _fvector vTargetDir, CNavigation* pNavigation)
{
	_vector vDir = XMVectorSetY(vTargetDir, 0.f);
	vDir = XMVectorSetW(vDir, 0.f);
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	vPosition += XMVector3Normalize(vDir) * m_fSpeedPerSec * fTimeDelta;
	if (m_bWallCol == true)
		return;

	if(nullptr == pNavigation || m_bPortalCol)
		Set_State(STATE_POSITION, vPosition);
	else
	{
		_bool bIsMovable = pNavigation->IsMovable(vPosition);
		if (true == bIsMovable)
			Set_State(STATE_POSITION, vPosition);
		else if (false == bIsMovable)
			Slide(fTimeDelta, XMLoadFloat3(&pNavigation->Get_LineDir()), pNavigation);
	}
}

void CTransform::Fall(_float fTimeDelta)
{
	_vector vDir = XMVectorSet(0, -1, 0, 0);
	_vector		vPosition = Get_State_Vector(STATE_POSITION);
	vPosition += XMVector3Normalize(vDir) * m_fSpeedPerSec * fTimeDelta;
	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Jump(_float fTimeDelta)
{
	m_fJumpTime += fTimeDelta;
	m_fJumpPower = 0.55f - (m_fJumpTime * 1.3f);

	_vector vDir = XMVectorSet(0, 1, 0, 0);
	_vector	vPosition = Get_State_Vector(STATE_POSITION);
	vPosition += XMVector3Normalize(vDir) * m_fJumpPower * fTimeDelta * 90.f;
	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Gravity(_float fTimeDelta)
{
	if (m_bGroundCol)
		return;
	_vector vDir = XMVectorSet(0, 1, 0, 0);
	_vector	vPosition = Get_State_Vector(STATE_POSITION);
	vPosition -= XMVector3Normalize(vDir) * m_fGravity * fTimeDelta * fTimeDelta * 0.5f;
	Set_State(STATE_POSITION, vPosition);
}

void CTransform::Look_At(_fvector vAt)
{
	_vector		vLook = vAt - Get_State_Vector(STATE_POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	_vector		vUp = XMVector3Cross(vLook, vRight);

	_float3		vScaled = Get_Scaled();

	Set_State(STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);
	Set_State(STATE_UP, XMVector3Normalize(vUp) * vScaled.y);
	Set_State(STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::Get_Look_LandObject(_fvector vAt, _Out_ _float4& _vRight, _Out_ _float4& _vLook)
{
	_vector		vUp = Get_State_Vector(STATE_UP);
	_vector		vLook = vAt - Get_State_Vector(STATE_POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	vLook = XMVector3Cross(vRight, vUp);

	_float3		vScaled = Get_Scaled();

	XMStoreFloat4(&_vRight, XMVector3Normalize(vRight) * vScaled.x);
	XMStoreFloat4(&_vLook, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::Look_At_ForLandObject(_fvector vAt)
{
	_vector		vUp = Get_State_Vector(STATE_UP);
	_vector		vLook = vAt - Get_State_Vector(STATE_POSITION);
	_vector		vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	vLook = XMVector3Cross(vRight, vUp);

	_float3		vScaled = Get_Scaled();

	Set_State(STATE_RIGHT, XMVector3Normalize(vRight) * vScaled.x);	
	Set_State(STATE_LOOK, XMVector3Normalize(vLook) * vScaled.z);
}

void CTransform::Move_toTarget(_fvector vTargetPos, _float fTimeDelta, _float fMinDistance)
{
	_vector vTemp = XMVectorSetW(vTargetPos, 1.f);
	_vector		vPosition = Get_State_Vector(STATE_POSITION);

	_vector		vLook = vTemp - vPosition;
	_float fLength = XMVector3Length(vLook).m128_f32[0];
	if(fMinDistance <= fLength)
		vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	Set_State(CTransform::STATE_POSITION, vPosition);
}

void CTransform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_matrix			RotationMatrix  = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	for (size_t i = 0; i < STATE_POSITION; i++)
	{
		Set_State(STATE(i), 
			XMVector4Transform(Get_State_Vector((STATE)i), RotationMatrix));
		/*Set_State(STATE(i),
			XMVector3TransformNormal(Get_State_Vector((STATE)i), RotationMatrix));*/
	}

}

void CTransform::Rotation(_fvector vAxis, _float fRadian)
{
	_float3		vScaled = Get_Scaled();

	_vector		vState[] = {
		XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScaled.x,
		XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScaled.y, 
		XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScaled.z, 		
	};

	_matrix			RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	for (size_t i = 0; i < STATE_POSITION; i++)
	{
		Set_State(STATE(i),
			XMVector4Transform(vState[(STATE)i], RotationMatrix));
	}
}

void CTransform::Slide(_float fTimeDelta, _fvector _vSlideDir, CNavigation* pNavigation)
{
	if(XMVector3Equal(_vSlideDir, XMVectorSet(0.f, 0.f, 0.f, 0.f)))
		return;

	_vector vLook = Get_State_Vector(STATE_LOOK);
	vLook = XMVector3Normalize(XMVectorSetY(vLook, 0.f));

	_vector vSlideDir = XMVector3Normalize(XMVectorSetY(_vSlideDir, 0.f));

	_float fAngle = ::XMVectorGetX(::XMVector3AngleBetweenVectors(vLook, vSlideDir));
	_float fY = ::XMVectorGetY(::XMVector3Cross(vLook, vSlideDir));
	if (fY < 0)
		fAngle = -fAngle;

	_vector	vPosition = Get_State_Vector(STATE_POSITION);
	vPosition += XMVector3Normalize(vSlideDir) * cosf(fAngle) * m_fSpeedPerSec * fTimeDelta;
	if(pNavigation->IsMovable(vPosition))
		Set_State(STATE_POSITION, vPosition);
}

CTransform * CTransform::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CTransform*		pInstance = new CTransform(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTransform"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent * CTransform::Clone(void * pArg)
{
	CTransform*		pInstance = new CTransform(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Cloned : CTransform"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTransform::Free()
{
	__super::Free();
}
