#include "ImGui_Manager.h"
#include "GameInstance.h"
#include "MyImGui.h"

CImGui_Manager::CImGui_Manager()
{
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

CImGui_Manager* CImGui_Manager::Create(CMyImGui* pImGui)
{
	CImGui_Manager* pInstance = new CImGui_Manager();

	if (FAILED(pInstance->Initiailize(pImGui)))
	{
		MSG_BOX(TEXT("Failed to Created : IMGUI Manager"));
		Safe_Release<CImGui_Manager*>(pInstance);
	}

	return pInstance;
}

void CImGui_Manager::Free()
{
	__super::Free();

	Safe_Release<CMyImGui*>(m_pImGui);
}
