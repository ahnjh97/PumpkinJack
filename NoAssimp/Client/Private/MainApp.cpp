#include "stdafx.h"
#include "Inven.h"
#include "MainApp.h"
#include "LifeBar.h"
#include "UI_Weapon.h"
#include "Indicator.h"
#include "BackGround.h"
#include "GameInstance.h"
#include "Level_Loading.h"


CMainApp::CMainApp()
	: m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);	

	//D3D11_SAMPLER_DESC

	/*m_pGraphic_Device->SetRenderState(D3DRS_TEXTUREFACTOR, 0x);*/
}


HRESULT CMainApp::Initialize()
{
	ENGINE_DESC		EngineDesc = {};

	EngineDesc.hWnd = g_hWnd;
	EngineDesc.isWindowed = true;
	EngineDesc.iWinSizeX = g_iWinSizeX;
	EngineDesc.iWinSizeY = g_iWinSizeY;

	/* 내 게임의 기초 초기화 과정을 거치자. */
	if (FAILED(m_pGameInstance->Initialize_Engine(g_hInst, LEVEL_END, EngineDesc, &m_pDevice, &m_pContext)))
		return E_FAIL;

	if (FAILED(Ready_Prototype_Component_For_Static()))
		return E_FAIL;

	if (FAILED(Open_Level(LEVEL_LOGO)))
		return E_FAIL;
	
	return S_OK;
}

void CMainApp::Tick(_float fTimeDelta)
{
	m_pGameInstance->Tick_Engine(fTimeDelta);
}

HRESULT CMainApp::Render()
{
	if (nullptr == m_pGameInstance)
		return E_FAIL;

	m_pGameInstance->Draw();	

	return	S_OK;
}

HRESULT CMainApp::Open_Level(LEVEL eLevelID)
{
	if (LEVEL_LOADING == eLevelID)
		return E_FAIL;

	m_pGameInstance->Open_Level(LEVEL_LOADING, CLevel_Loading::Create(m_pDevice, m_pContext, eLevelID));

	return	S_OK;
}

HRESULT CMainApp::Ready_Prototype_Component_For_Static()
{
	/* For.Prototype_Component_VIBuffer_Rect */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		CVIBuffer_Rect::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	/* For.Prototype_Component_Shader_VtxPosTex */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_LifeBar"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex_LifeBar.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_CrowLifeBar"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex_CrowLifeBar.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimModel_Slime"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel_Slime.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_Inven"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxPosTex_Inven.hlsl"), VTXPOSTEX::Elements, VTXPOSTEX::iNumElements))))
		return E_FAIL;
	if(FAILED(Add_Texture(LEVEL_STATIC, "LoadingScreens", "Logo/T_LoadingScreens%d.dds", 6)))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "LoadingScreens_GameLogo", "Logo/T_LoadingScreens_GameLogo.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "LoadingScreens_Holder", "Logo/T_LoadingScreens_Holder.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "LoadingScreens_Jack", "Logo/T_LoadingScreens_Jack.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "Spinner", "Logo/Spinner.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "LifeBarMaskHolder", "Logo/T_LifebarMaskHolder.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "LifeBarMask", "Logo/T_LifebarMask.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "CrowLifeBar", "Logo/UI_CrowLifebar2.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "LifeBar", "Logo/T_LifeBar%d.dds", 2)))
		return E_FAIL;
 	if (FAILED(Add_Texture(LEVEL_STATIC, "Inven", "Weapon/UI_Inventory_3_%d.dds", 3)))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "Indicator", "Weapon/UI_Indicator_3_%d.dds", 3)))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "Spear", "Weapon/T_Weapon_Spear.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "Rifle", "Weapon/T_Weapon_Rifle.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(LEVEL_STATIC, "Scythe", "Weapon/T_Weapon_Scythe.dds")))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_LoadingScreen"),
		CBackGround::Create(m_pDevice, m_pContext, true))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_LifeBar"),
		CLifeBar::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Inven"),
		CInven::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_UI_Weapon"),
		CUI_Weapon::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Indicator"),
		CIndicator::Create(m_pDevice, m_pContext))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Collider_AABB"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_AABB))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_OBB))))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		CCollider::Create(m_pDevice, m_pContext, CCollider::TYPE_SPHERE))))
		return E_FAIL;
	return	S_OK;
}

HRESULT CMainApp::Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring wstrPrototypeTag = L"Prototype_Component_Texture_" + converter.from_bytes(strPrototypeName);
	wstring wstrFullPath = L"../../../Resources/Textures/" + converter.from_bytes(strFolderAndFileName);

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CTexture::Create(m_pDevice, m_pContext, wstrFullPath, iNumTextures))))
		return E_FAIL;

	return S_OK;
}

CMainApp * CMainApp::Create()
{
	CMainApp*		pInstance = new CMainApp();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX(TEXT("Failed To Create : CMainApp"));
		
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMainApp::Free()
{
	__super::Free();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	Safe_Release(m_pGameInstance);

	CGameInstance::Release_Engine();
}
