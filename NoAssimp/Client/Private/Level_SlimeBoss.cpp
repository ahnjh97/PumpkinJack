#include "stdafx.h"
#include "Level_SlimeBoss.h"
#include "Level_Loading.h"
#include "ImGui_Client.h"
#include "Camera_Jack.h"
#include "Terrain.h"
#include "Inven.h"

CLevel_SlimeBoss::CLevel_SlimeBoss(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel{ pDevice, pContext }
{
}

HRESULT CLevel_SlimeBoss::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	string strFileName = "../../../objects_txt/Level_SlimeBoss.txt";
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

	if (FAILED(Ready_Layer_BackGround(TEXT("Layer_BackGround"))))
		return E_FAIL;

	if (FAILED(Ready_LandObject()))
		return E_FAIL;

	m_InputFile.close();

	m_pGameInstance->UnHide_ImGui();
	return S_OK;
}

void CLevel_SlimeBoss::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	if (m_pGameInstance->KeyDown(DIK_F) && !m_bPressed)	{
		m_bPressed = true;
		CGameObject* pNotOpen = m_pGameInstance->Get_GameObject(LEVEL_SLIMEBOSS, TEXT("Layer_Monster"), TEXT("Prototype_GameObject_Monster"));
		pNotOpen->Set_Dead();
		CGameObject::GAMEOBJECT_DESC tempDesc = {};
		PARSE_DATA tParseData = pNotOpen->Get_ParseData();
		tempDesc.matWorld._41 = tParseData.matWorld._41;
		tempDesc.matWorld._42 = tParseData.matWorld._42;
		tempDesc.matWorld._43 = tParseData.matWorld._43;
		tempDesc.iLevel = LEVEL_SLIMEBOSS;
		tempDesc.wstrModelTag = TEXT("Prototype_Component_Model_SlimeBoss");
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_SlimeBoss"), TEXT("Prototype_GameObject_SlimeBoss"), &tempDesc)))
			return;
	}

	if (m_pGameInstance->KeyDown(DIK_RETURN))
	{
		if (FAILED(m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL_WIZARD))))
			return;
	}
}

HRESULT CLevel_SlimeBoss::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	_int iFps = m_pGameInstance->Get_FPS();
	wstring wstrFPS = to_wstring(iFps);
	wstring wstrLevelFPS = wstring(L"Level_SlimeBoss  FPS: ") + wstrFPS;
	SetWindowText(g_hWnd, wstrLevelFPS.c_str());

	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vPosition = _float4(0.f, 0.f, 0.f, 1.f);
	LightDesc.vRange = 100.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(0.15f, 0.15f, 0.15f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_Layer_Camera(const wstring& strLayerTag)
{
	CCamera_Jack::CAMERA_JACK_DESC		CameraDesc{};

	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(50.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, 10.f, -7.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 60.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	CameraDesc.fDistance = 72.f;
	CameraDesc.fHeight = 26.f;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Camera_Jack"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_Layer_BackGround(const wstring& strLayerTag)
{
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_SLIMEBOSS;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Sky"), &tDesc)))
		return E_FAIL;

	CTerrain::TERRAIN_DESC terrainDesc{};
	terrainDesc.iLevel = LEVEL_SLIMEBOSS;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Terrain"), &terrainDesc)))
		return E_FAIL;

	terrainDesc.matWorld._42 = -155.f;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Terrain_SlimeBossRoom"), &terrainDesc)))
		return E_FAIL;

	tDesc.matWorld._11 = 15.f;
	tDesc.matWorld._22 = 15.f;
	tDesc.matWorld._33 = 15.f;
	tDesc.matWorld._41 = 200.f;
	tDesc.matWorld._42 = 10.f;
	tDesc.matWorld._43 = 100.f;
	//if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_Portal"), TEXT("Prototype_GameObject_Portal"), &tDesc)))
	//	return E_FAIL;

	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_LandObject()
{
	if(FAILED(Ready_ParsedObjects(TEXT("Layer_Monster"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_ParsedObjects(const wstring& strLayerTag)
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
		tempDesc.iLevel = LEVEL_SLIMEBOSS;
		if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Jack")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"), &tempDesc)))
				return E_FAIL;
		}
		else if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_SlimeBoss")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_SlimeBoss"), TEXT("Prototype_GameObject_SlimeBoss"), &tempDesc)))
				return E_FAIL;
		}
		else if(tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_LevelSlimeBoss")){
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, TEXT("Layer_Map"), TEXT("Prototype_GameObject_TongMap_SlimeBoss"), &tempDesc)))
				return E_FAIL;
		}
		else{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Monster"), &tempDesc)))
				return E_FAIL;
		}
	}

	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_Layer_Effect(const wstring& strLayerTag)
{
	return S_OK;
}

HRESULT CLevel_SlimeBoss::Ready_UI(const wstring& strLayerTag)
{
	CGameObject::GAMEOBJECT_DESC tDesc = {};
	tDesc.matWorld._43 = fZ_LifeBarMask;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_LifeBar"), &tDesc)))
		return E_FAIL;
	tDesc.matWorld._43 = fZ_CrowLifeBar;
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_LifeBar"), &tDesc)))
		return E_FAIL;

	CInven::INVEN_DESC tInvenDesc{};
	for (_int i = 0; i < WPN_END; i++)
	{
		tInvenDesc.eWeapon = (WEAPON)i;
		tInvenDesc.matWorld._43 = 0.9f - i * 0.1f;
		tInvenDesc.iLevel = LEVEL_SLIMEBOSS;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Inven"), &tInvenDesc)))
			return E_FAIL;
		tInvenDesc.matWorld._43 = 0.85f - i * 0.1f;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_SLIMEBOSS, strLayerTag, TEXT("Prototype_GameObject_Indicator"), &tInvenDesc)))
			return E_FAIL;
	}
	return S_OK;
}

CLevel_SlimeBoss* CLevel_SlimeBoss::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_SlimeBoss* pInstance = new CLevel_SlimeBoss(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_SlimeBoss"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLevel_SlimeBoss::Free()
{
	m_pGameInstance->Hide_ImGui();
	__super::Free();
}
