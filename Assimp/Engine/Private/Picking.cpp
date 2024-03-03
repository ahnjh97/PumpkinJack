#include "Picking.h"
#include "Transform.h"
#include "GameInstance.h"

CPicking::CPicking(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice),
	m_pContext(pContext),
	m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(pDevice);
	Safe_AddRef(pContext);
}

HRESULT CPicking::Initialize(HWND hWnd, _uint iWinSizeX, _uint iWinSizeY)
{
	m_hWnd = hWnd;

	m_iWinSizeX = iWinSizeX;
	m_iWinSizeY = iWinSizeY;

	return S_OK;
}

void CPicking::Update()
{
	/* 매번 다른 폴리곤과 충돌을 할때마다 마우스좌표를 다시 구해서 역으로 로컬까지 내려가는과정이 너무 길기때문에.
	모든 폴리곤모델과 충돌할때 공통적으로 거쳐가는 공간(뷰포트->월드)까지는 미리 구해놓는다. */
	/* 모든 모델들이 공통으로 존재하는 월드 스페이스 상의 마우스레이, 마우스 레이의 시작점을 구해놓자. */
	POINT			ptMouse;
	GetCursorPos(&ptMouse);

	/* 뷰포트 상의 마우스 위치를 구하자. */
	ScreenToClient(m_hWnd, &ptMouse);


	/* 0, 0 -> -1, 1 */
	/* wincx * 0.5f, wincy * 0.5f -> 0, 0 */
	/* wincx, wincy -> 1, -1 */

	/* 투영스페이스 상의 마우스 위치를 구하자. */
	_vector		vMousePos = XMVectorSet(ptMouse.x / (m_iWinSizeX * 0.5f) - 1.f, ptMouse.y / -(m_iWinSizeY * 0.5f) + 1.f, 0.f, 1.f);

	_matrix	ProjMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_PROJ);

	/* 뷰스페이스 상의 마우스 위치를 구했다. */
	vMousePos = XMVector3TransformCoord(vMousePos, ProjMatrixInv);

	/* 뷰스페이스 상의 마우스 레이, 마우스 레이의 시작점을 구하자. */
	//m_vRayDir = vMousePos - _float3(0.f, 0.f, 0.f);
	//m_vRayPos = _float3(0.f, 0.f, 0.f);

	_matrix	ViewMatrixInv = m_pGameInstance->Get_Transform_Matrix_Inverse(CPipeLine::D3DTS_VIEW);

	/* 월드스페이스 상의 마우스 위치를 구했다. */
	XMStoreFloat3(&m_vRayDir, XMVector3TransformNormal(vMousePos, ViewMatrixInv));
	XMStoreFloat3(&m_vRayDir, XMVector3TransformCoord(XMVectorSet(0.f, 0.f, 0.f, 1.f), ViewMatrixInv));
}

void CPicking::Transform_PickingToLocalSpace(const CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float3* pRayPos)
{
	_matrix	WorldMatrixInv = pTransform->Get_WorldMatrix_Inverse();
	XMStoreFloat3(pRayDir, XMVector3TransformNormal(XMLoadFloat3(&m_vRayDir), WorldMatrixInv));
	XMStoreFloat3(pRayPos, XMVector3TransformNormal(XMLoadFloat3(&m_vRayPos), WorldMatrixInv));
}

CPicking* CPicking::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, HWND hWnd, _uint iWinSizeX, _uint iWinSizeY)
{
	CPicking* pInstance = new CPicking(pDevice, pContext);

	if (FAILED(pInstance->Initialize(hWnd, iWinSizeX, iWinSizeY)))
	{
		MSG_BOX(TEXT("Failed To Created : CPicking"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPicking::Free()
{
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
