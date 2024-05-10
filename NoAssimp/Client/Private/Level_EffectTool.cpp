#include "stdafx.h"
#include "Level_EffectTool.h"
#include "SwipeEffect.h"
#include "Camera_Free.h"
#include "HitEffect.h"
#include "Lightning.h"
#include "ElecParts.h"
#include "ElecChain.h"
#include "Terrain.h"
#include "Slash.h"
#include "Dash.h"
#include "Part.h"

CLevel_EffectTool::CLevel_EffectTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CLevel(pDevice, pContext)
{
}

HRESULT CLevel_EffectTool::Initialize()
{
	if (FAILED(__super::Initialize()))
		return E_FAIL;

	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Camera(TEXT("Layer_Camera"))))
		return E_FAIL;

	if (FAILED(Ready_Layer_Grid(TEXT("Layer_Grid"))))
		return E_FAIL;

	CGameObject::GAMEOBJECT_DESC tDesc = {};
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_EFFECTTOOL, TEXT("Layer_DamageEffect"), TEXT("Prototype_GameObject_DamageEffect"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

void CLevel_EffectTool::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (m_pGameInstance->KeyDown(DIK_E))
		Add_Effect(TEXT("Layer_Effect"));
	if (m_pGameInstance->KeyDown(DIK_F))
		Add_Effect2(TEXT("Layer_Effect2"));
	if (m_pGameInstance->KeyDown(DIK_C))
		Add_Object(TEXT("Layer_Object"));
}

HRESULT CLevel_EffectTool::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	_int iFps = m_pGameInstance->Get_FPS();
	wstring wstrFPS = to_wstring(iFps);
	wstring wstrLevelFPS = wstring(L"Level_EffectTool  FPS: ") + wstrFPS;
	SetWindowText(g_hWnd, wstrLevelFPS.c_str());

	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vAmbient = _float4(.6f, .6f, .6f, 1.f);
	LightDesc.vSpecular = _float4(0.15f, 0.15f, 0.15f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Layer_Camera(const wstring& strLayerTag)
{
	CCamera_Free::CAMERA_FREE_DESC		CameraDesc{};

	CameraDesc.fMouseSensor = 0.1f;
	CameraDesc.fFovy = XMConvertToRadians(45.0f);
	CameraDesc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	CameraDesc.fNear = 0.1f;
	CameraDesc.fFar = 1000.0f;
	CameraDesc.vEye = _float4(0.f, 50.f, -100.f, 1.f);
	CameraDesc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	CameraDesc.fSpeedPerSec = 50.f;
	CameraDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_EFFECTTOOL, strLayerTag, TEXT("Prototype_GameObject_Camera_Free"), &CameraDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Ready_Layer_Grid(const wstring& strLayerTag)
{
	CTerrain::TERRAIN_DESC terrainDesc{};
	terrainDesc.iLevel = LEVEL_EFFECTTOOL;
	terrainDesc.iGridIndex = 0;
	if(FAILED(m_pGameInstance->Add_Clone(LEVEL_EFFECTTOOL, strLayerTag, TEXT("Prototype_GameObject_Terrain"), &terrainDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Add_Effect(const wstring& strLayerTag)
{
	CSwipeEffect::SWIPEEFFECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_EFFECTTOOL;
	tDesc.vSlimeBossRight = XMVectorSet(1, 0, 0, 0);
	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_EFFECTTOOL, strLayerTag, TEXT("Prototype_GameObject_SwipeEffect"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Add_Effect2(const wstring& strLayerTag)
{
	CGameObject::GAMEOBJECT_DESC tDesc{};
	tDesc.iLevel = LEVEL_EFFECTTOOL;

	if (FAILED(m_pGameInstance->Add_Clone(LEVEL_EFFECTTOOL, strLayerTag, TEXT("Prototype_GameObject_Explosion"), &tDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLevel_EffectTool::Add_Object(const wstring& strLayerTag)
{
	for (_int i = 0; i < 10; i++) {
		CGameObject::GAMEOBJECT_DESC tDesc{};
		tDesc.iLevel = LEVEL_EFFECTTOOL;
		if (FAILED(m_pGameInstance->Add_Clone(LEVEL_EFFECTTOOL, strLayerTag, TEXT("Prototype_GameObject_SlimeParticles"), &tDesc)))
			return E_FAIL;
	}

	return S_OK;
}

CLevel_EffectTool* CLevel_EffectTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_EffectTool* pInstance = new CLevel_EffectTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CLevel_EffectTool"));

		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLevel_EffectTool::Free()
{
	__super::Free();
}