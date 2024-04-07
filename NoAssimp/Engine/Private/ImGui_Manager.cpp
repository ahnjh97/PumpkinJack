#include "ImGui_Manager.h"
#include "GameInstance.h"
#include "MyImGui.h"

CImGui_Manager::CImGui_Manager()
{
}

void CImGui_Manager::Set_Vectors(vector<string> vecAnim, vector<string> vecNonAnim)
{
	if (nullptr == m_pImGui)
		return;

	return m_pImGui->Set_Vectors(vecAnim, vecNonAnim);
}

_float3 CImGui_Manager::Get_PickPos()
{
	if (nullptr == m_pImGui)
		return _float3();

	return m_pImGui->Get_PickPos();
}

_bool CImGui_Manager::IsPicking()
{
	if (nullptr == m_pImGui)
		return _bool();

	return m_pImGui->IsPicking();
}

HRESULT CImGui_Manager::Initiailize(CMyImGui* pImGui)
{
	if (nullptr == pImGui)
		return E_FAIL;

	m_pImGui = pImGui;

	return S_OK;
}

void CImGui_Manager::Tick(_float fTimeDelta)
{
	if (nullptr != m_pImGui)
		m_pImGui->Tick(fTimeDelta);
}

HRESULT CImGui_Manager::Render()
{
	if (nullptr != m_pImGui)
		m_pImGui->Render();

	return S_OK;
}

HRESULT CImGui_Manager::Release_ImGui()
{
	if(m_pImGui != nullptr)
		Safe_Release(m_pImGui);

	return S_OK;
}

CImGui_Manager* CImGui_Manager::Create(CMyImGui* pImGui)
{
	CImGui_Manager* pInstance = new CImGui_Manager();

	if (FAILED(pInstance->Initiailize(pImGui)))
	{
		MSG_BOX(TEXT("Failed To Create : IMGUI Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CImGui_Manager::Free()
{
	__super::Free();
	Safe_Release(m_pImGui);
}
