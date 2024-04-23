#include "stdafx.h"
#include "Level_Loading.h"
#include "Level_Wizard.h"
#include "ImGui_Client.h"
#include "Camera_Jack.h"
#include "Terrain.h"
#include "Inven.h"

CLevel_Wizard::CLevel_Wizard(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_Wizard::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	string strFileName = "../../../objects_txt/Level_Wizard.txt";
	m_InputFile.open(strFileName, ios::in | ios::binary);
	if (!m_InputFile.is_open())
	{
		string tempstr = "Failed : " + strFileName;
		MessageBoxA(nullptr, tempstr.c_str(), "error", MB_OK);
		return E_FAIL;
	}

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_UI(TEXT("Layer_UI"))))
		return E_FAIL;
	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround()))
		return E_FAIL;


	if (FAILED(Ready_LandObject()))
		return E_FAIL;

	m_InputFile.close();

	m_pGameInstance->UnHide_ImGui();
	return S_OK;
}

void CLevel_Wizard::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	//m_fTime += fTimeDelta;

	//if (m_bBooks && ((m_fTime >= 1.f && m_fTime < 1.5f) || (m_fTime >= 3.f && m_fTime < 3.5f)
	//	|| (m_fTime >= 5.f && m_fTime < 5.5f) || (m_fTime >= 7.f && m_fTime < 7.5f))) {
	//	Add_Books();
	//	m_bBooks = false;
	//}

	//if (!m_bBooks && ((m_fTime >= 2.f && m_fTime < 2.5f) || (m_fTime >= 4.f && m_fTime < 4.5f)
	//	|| (m_fTime >= 6.f && m_fTime < 6.5f) || (m_fTime >= 8.f && m_fTime < 8.5f))) {
	//	Add_Books();
	//	m_bBooks = true;
	//}
}

HRESULT CLevel_Wizard::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	_int iFps = m_pGameInstance->Get_FPS();
	wstring wstrFPS = to_wstring(iFps);
	wstring wstrLevelFPS = wstring(L"Level_Wizard  FPS: ") + wstrFPS;
	SetWindowText(g_hWnd, wstrLevelFPS.c_str());

	return S_OK;
}

HRESULT CLevel_Wizard::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(0.1f, 0.1f, 0.1f, 1.f);
	LightDesc.vSpecular = _float4(0.1f, 0.1f, 0.1f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Wizard::Ready_Layer_Camera(const wstring& strLayerTag)
{
	CCamera_Jack::CAMERA_JACK_DESC		CameraDesc{};

	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(50.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 2000.0f;
	CameraDesc.vEye = _float4(0.f, 10.f, -7.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 20.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	CameraDesc.fDistance = 82.f;
	CameraDesc.fHeight = 30.f;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, strLayerTag, TEXT("Prototype_GameObject_Camera_Jack"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Wizard::Ready_LandObject()
{

	if (FAILED(Ready_ParsedObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_Wizard::Ready_ParsedObjects()
{
	wchar_t wcharPrototypeTag[MAX_PATH];
	wchar_t wcharModelTag[MAX_PATH];
	_float4x4 matWorld{};

	while (!m_InputFile.eof())
	{
		m_InputFile.read(reinterpret_cast<char*>(&wcharPrototypeTag), sizeof(wcharPrototypeTag));
		m_InputFile.read(reinterpret_cast<char*>(&matWorld), sizeof(matWorld));
		m_InputFile.read(reinterpret_cast<char*>(&wcharModelTag), sizeof(wcharModelTag));

		if (m_InputFile.eof())
			break;

		wstring wstrPrototypeTag = wcharPrototypeTag;
		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		tempDesc.matWorld = matWorld;
		tempDesc.wstrModelTag = wcharModelTag;
		tempDesc.iLevel = LEVEL_WIZARD;

		if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Jack")) {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_LevelWizard")) {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_Map"), TEXT("Prototype_GameObject_TongMap_Wizard"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Wizard")) {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_Wizard"), TEXT("Prototype_GameObject_Wizard"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_WizardTower")) {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_WizardTower"), TEXT("Prototype_GameObject_WizardTower"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_elecArena1")) {
			for (_int i = 1; i < 4; i++) {
				tempDesc.wstrModelTag = TEXT("Prototype_Component_Model_elecArena") + to_wstring(i);
				tempDesc.matWorld._42 += (1.f) * (i - 1);
				if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_ElecArena"), TEXT("Prototype_GameObject_ElecArena"), &tempDesc)))
					return E_FAIL;
			}
		}
		else {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Monster"), &tempDesc)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevel_Wizard::Ready_UI(const wstring& strLayerTag)
{
	CGameObject::GAMEOBJECT_DESC tDesc = {};
	tDesc.matWorld._43 = fZ_LifeBarMask;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, strLayerTag, TEXT("Prototype_GameObject_LifeBar"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_CrowLifeBar;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, strLayerTag, TEXT("Prototype_GameObject_LifeBar"), &tDesc)))
		return E_FAIL;

	CInven::INVEN_DESC tInvenDesc{};
	for (_int i = 0; i < WPN_END; i++)
	{
		tInvenDesc.eWeapon = (WEAPON)i;
		tInvenDesc.matWorld._43 = 0.9f - i * 0.1f;
		tInvenDesc.iLevel = LEVEL_WIZARD;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, strLayerTag, TEXT("Prototype_GameObject_Inven"), &tInvenDesc)))
			return E_FAIL;

		tInvenDesc.matWorld._43 = 0.85f - i * 0.1f;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, strLayerTag, TEXT("Prototype_GameObject_Indicator"), &tInvenDesc)))
			return E_FAIL;
	}

	return S_OK;
}

void CLevel_Wizard::Add_Books()
{
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_WIZARD;
	m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_Book"), TEXT("Prototype_GameObject_Book"), &tDesc);
}

HRESULT CLevel_Wizard::Ready_Layer_BackGround()
{
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_WIZARD;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_SkySphere"), TEXT("Prototype_GameObject_SkySphere"), &tDesc)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_WIZARD, TEXT("Layer_Book"), TEXT("Prototype_GameObject_Book"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

CLevel_Wizard* CLevel_Wizard::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_Wizard* pInstance = new CLevel_Wizard(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_Wizard"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_Wizard::Free()
{
	m_pGameInstance->Hide_ImGui();
	__super::Free();
}
