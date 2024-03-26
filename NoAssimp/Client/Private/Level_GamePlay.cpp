#include "stdafx.h"
#include "Level_GamePlay.h"
#include "ImGui_Client.h"
#include "Camera_Jack.h"
#include "Level_Loading.h"
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
	/*
	if (FAILED(Ready_Layer_Effect(TEXT("Layer_Effect"))))
		return E_FAIL;

	*/
	if (FAILED(m_pGameInstance->Initialize_ImGui(LEVEL_GAMEPLAY, CImGui_Client::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	
	m_InputFile.close();
	return S_OK;
}

void CLevel_GamePlay::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);
	if (m_pGameInstance->KeyDown(DIK_RETURN))
	{
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
	LightDesc.vPosition = _float4(0.f, 0.f, 0.f, 1.f);
	LightDesc.vRange = 100.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(0.15f, 0.15f, 0.15f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Camera(const wstring & strLayerTag)
{
	CCamera_Jack::CAMERA_JACK_DESC		CameraDesc{};

	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(60.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, 10.f, -7.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 20.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);
	
	CameraDesc.fDistance = 40.f;
	CameraDesc.fHeight = 10.f;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Camera_Jack"), &CameraDesc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_LandObject()
{

	if (FAILED(Ready_Layer_Monster(TEXT("Layer_Monster"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Player(TEXT("Layer_Player"))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Player(const wstring& strLayerTag)
{
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Monster(const wstring & strLayerTag)
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

		if (tempDesc.wstrModelTag == TEXT("Prototype_Component_Model_Jack"))
		{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, TEXT("Layer_Player"), TEXT("Prototype_GameObject_Player"), &tempDesc)))
				return E_FAIL;
		}
		else
		{
			if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Monster"), &tempDesc)))
				return E_FAIL;
		}
	}
	
	return S_OK;
}

HRESULT CLevel_GamePlay::Ready_Layer_Effect(const wstring & strLayerTag)
{
	/*for (size_t i = 0; i < 20; i++)
	{
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Effect"))))
			return E_FAIL;
	}*/

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
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Terrain"), &tDesc)))
		return E_FAIL;

	/*
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_GAMEPLAY, strLayerTag, TEXT("Prototype_GameObject_Sky"))))
		return E_FAIL;
*/
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
	m_pGameInstance->Release_ImGui();
	__super::Free();
}
