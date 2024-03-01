#include "..\Public\Camera.h"
#include "GameInstance.h"

CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{


}

CCamera::CCamera(const CCamera & rhs)
	: CGameObject(rhs)
{
}

HRESULT CCamera::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera::Initialize(void * pArg)
{
	CAMERA_DESC*		pCameraDesc = (CAMERA_DESC*)pArg;

	m_fFovy = pCameraDesc->fFovy;
	m_fAspect = pCameraDesc->fAspect;
	m_fNear = pCameraDesc->fNear;
	m_fFar = pCameraDesc->fFar;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMLoadFloat4(&pCameraDesc->vEye));
	m_pTransformCom->Look_At(XMLoadFloat4(&pCameraDesc->vAt));

	return S_OK;
}

void CCamera::Tick(_float fTimeDelta)
{
}

void CCamera::Late_Tick(_float fTimeDelta)
{
}


HRESULT CCamera::Bind_PipeLines()
{	
	/* dx9 : 고정기능렌더링파이프라인. 현재 카메라에서 설정할 수 있는 행렬들을 장치에 바인딩하여 추후 렌더릴ㅇ되는 정점들에게 알아서 곱할 수 있도록 한다. */
	/* dx11 : 사용자 정의 렌더링 파이프라인(ㅅㅖ이더). */
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, XMMatrixPerspectiveFovLH(m_fFovy, m_fAspect, m_fNear, m_fFar));	

	return S_OK;
}

void CCamera::Free()
{
	__super::Free();


}
