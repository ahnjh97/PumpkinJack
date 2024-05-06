#include "stdafx.h"
#include "Level_GamePlay.h"
#include "Level_Loading.h"
#include "ImGui_Client.h"
#include "Camera_Jack.h"
#include "Terrain.h"
#include "Inven.h"

CLevel_GamePlay::CLevel_GamePlay(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{

}

HRESULT CLevel_GamePlay::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	string strFileName = "../../../objects_txt/Level_GamePlay.txt";
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
	if(FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_LandObject()))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Initialize_ImGui(LEVEL_GAMEPLAY, CImGui_Client::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	
	m_InputFile.close();

	return S_OK;
}

void CLevel_GamePlay::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	if (m_pGameInstance->KeyDown(DIK_RETURN)) {
		if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_SLIMEBOSS))))
			return;
	}
}

HRESULT CLevel_GamePlay::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	_int iFps = m_pGameInstance->Get_FPS();
	wstring wstrFPS = to_wstring(iFps);
	wstring wstrLevelFPS = wstring(L"Level_GamePlay  FPS: ") + wstrFPS;
	SetWindowText(g_hWnd, wstrLevelFPS.c_str());

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(0.3f, 0.3f, 0.3f, 1.f);
	LightDesc.vSpecular = _float4(0.1f, 0.1f, 0.1f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const wstring & strLayerTag)
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
	
	CameraDesc.fDistance = 70.f;
	CameraDesc.fHeight = 20.f;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Camera_Jack"), &CameraDesc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_LandObject()
{
	if (FAILED(Ready_ParsedObjects()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_ParsedObjects()
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
		tempDesc.iLevel = LEVEL_GAMEPLAY;

		if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Jack")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_LevelGamePlay")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Map"), TEXT("Prototype_GameObject_TongMap_GamePlay"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Ballista")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Ballista"), TEXT("Prototype_GameObject_Ballista"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_ballistaRotator")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_BallistaRotator"), TEXT("Prototype_GameObject_BallistaRotator"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_ballistaBase")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_BallistaBase"), TEXT("Prototype_GameObject_Monster"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_SkullCrossBow"))	{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_SkullCrossBow"), TEXT("Prototype_GameObject_SkullCrossBow"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Kart")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Kart"), TEXT("Prototype_GameObject_Kart"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Rail") || tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_RailLeft60")
			|| tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_RailLeft90") || tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_RailRight60")
			|| tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_RailRight90"))
		{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Rail"), TEXT("Prototype_GameObject_Rail"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_crateBoxPotion"))	{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_CrateBox"), TEXT("Prototype_GameObject_CrateBox"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_fenceStack_SmallCol")) {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Trigger"), TEXT("Prototype_GameObject_Trigger"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_MinePlatform")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_MinePlatform"), TEXT("Prototype_GameObject_Monster"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Cube")) {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_CollisionCube"), TEXT("Prototype_GameObject_CollisionCube"), &tempDesc)))
				return E_FAIL;
		}
		else {
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Monster"), &tempDesc)))
				return E_FAIL;
		}
	}
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_UI(const wstring& strLayerTag)
{
	CGameObject::GAMEOBJECT_DESC tDesc = {};
	tDesc.matWorld._43 = fZ_LifeBarMask;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_LifeBar"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_CrowLifeBar;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_LifeBar"), &tDesc)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_DamageEffect"), TEXT("Prototype_GameObject_DamageEffect"), &tDesc)))
		return E_FAIL;

	CInven::INVEN_DESC tInvenDesc{};
	for (_int i = 0; i < WPN_END; i++)
	{
		tInvenDesc.eWeapon = (WEAPON)i;
		tInvenDesc.matWorld._43 = 0.9f - i * 0.1f;
		tInvenDesc.iLevel = LEVEL_GAMEPLAY;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Inven"), &tInvenDesc)))
			return E_FAIL;
		
		tInvenDesc.matWorld._43 = 0.85f - i * 0.1f;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Indicator"), &tInvenDesc)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_BackGround(const wstring & strLayerTag)
{
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_GAMEPLAY;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Sky"), &tDesc)))
		return E_FAIL;

	CTerrain::TERRAIN_DESC terrainDesc{};
	terrainDesc.iLevel = LEVEL_GAMEPLAY;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Terrain"), &terrainDesc)))
		return E_FAIL;

	return S_OK;
}

CLevel_GamePlay * CLevel_GamePlay::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_GamePlay*		pInstance = new CLevel_GamePlay(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_GamePlay"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_GamePlay::Free()
{
	m_pGameInstance->Hide_ImGui();
	__super::Free();
}
