#include "stdafx.h"
#include "Loader.h"
#include <process.h>
#include <locale>
#include <codecvt>
#include "GameInstance.h"
#include "Camera_Free.h"
#include "Camera_Jack.h"
#include "BackGround.h"
#include "Terrain.h"
#include "Monster.h"
#include "Player.h"
#include "Grid.h"
#include "Tool_Anim.h"
#include "Tool_NonAnim.h"
#include "Body.h"
#include "Weapon.h"
#include "SlimeBoss.h"
#include "SlimeBoss_Slime.h"
#include "Knife.h"
//#include "Effect.h"
//#include "Sky.h"

CLoader::CLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
	, m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

_uint APIENTRY LoadingMain(void* pArg)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	/* 로더에게 지정된 레벨을 준비해라*/
	CLoader* pLoader = (CLoader*)pArg;

	if (FAILED(pLoader->Start()))
		return 1;

	return 0;
}

HRESULT CLoader::Initialize(LEVEL eNextLevelID)
{
	m_eNextLevelID = eNextLevelID;

	InitializeCriticalSection(&m_Critical_Section);

	/* 스레드를 생성하낟. */
	m_hThread = (HANDLE)_beginthreadex(nullptr, 0, LoadingMain, this, 0, nullptr);
	if (0 == m_hThread)
		return E_FAIL;

	Insert_ScaleRotationData();
	return S_OK;
}

HRESULT CLoader::Start()
{
	EnterCriticalSection(&m_Critical_Section);

	HRESULT		hr = { 0 };

	switch (m_eNextLevelID)
	{
	case LEVEL_LOGO:
		hr = Loading_For_Logo();
		break;
	case LEVEL_GAMEPLAY:
		hr = Loading_For_GamePlay(LEVEL_GAMEPLAY);
		break;
	case LEVEL_MAPTOOL:
		hr = Loading_For_MapTool(LEVEL_MAPTOOL);
		break;
	case LEVEL_SLIMEBOSS:
		hr = Loading_For_SlimeBoss(LEVEL_SLIMEBOSS);
		break;
	}

	if (FAILED(hr))
		return E_FAIL;

	LeaveCriticalSection(&m_Critical_Section);

	return S_OK;
}

HRESULT CLoader::Loading_For_Logo()
{
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	/* For.Prototype_Component_Texture_Logo */
	if(FAILED(Add_Texture(LEVEL_LOGO, "Logo", "Logo/wallpaper.jpg")))
		return E_FAIL;


	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");

	m_strLoadingText = TEXT("객체의 원형를(을) 로딩 중 입니다.");

	/* For.Prototype_GameObject_BackGround */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_BackGround"),
		CBackGround::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_GamePlay(LEVEL eLevel)
{
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

	/* Prototype_Component_Texture_Terrain */
	if (FAILED(Add_Texture(eLevel, "Terrain", "Terrain/Tile0.jpg")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Grass", "Terrain/grassTile%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/Mask.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Brush", "Terrain/Brush.png")))
		return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");

	/* Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 100, 100))))
		return E_FAIL;

	TraverseAnimModels(eLevel, L"../../../model_txt/Anim/");
	TraverseNonAnimModels(eLevel, L"../../../model_txt/NonAnim/");

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");

	Add_Shaders(eLevel);


	m_strLoadingText = TEXT("객체를(을) 로딩 중 입니다.");
	/* For.Prototype_GameObject_Terrain */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Terrain"), CTerrain);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Jack"), CCamera_Jack);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Monster"), CMonster);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Player"), CPlayer); 
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Body"), CBody);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Spear"), CWeapon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Rifle"), CWeapon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Scythe"), CWeapon);
	///* For.Prototype_GameObject_Effect */
	//if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Effect"),
	//	CEffect::Create(m_pGraphic_Device))))
	//	return E_FAIL;

	///* For.Prototype_GameObject_Sky */
	//if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_Sky"),
	//	CSky::Create(m_pGraphic_Device))))
	//	return E_FAIL;
	//
	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_MapTool(LEVEL eLevel)
{
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

	/* Prototype_Component_Texture_Grid */
	if(FAILED(Add_Texture(eLevel, "Grid", "Terrain/White.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain", "Terrain/Tile0.jpg")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Grass", "Terrain/grassTile%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/Mask.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Brush", "Terrain/Brush.png")))
		return E_FAIL;

	m_strLoadingText = TEXT("컴포넌트를(을) 로딩 중 입니다.");
	/* Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(LEVEL_MAPTOOL, TEXT("Prototype_Component_VIBuffer_Grid"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 400, 400))))
		return E_FAIL;


	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");

	TraverseAnimModels(eLevel, L"../../../model_txt/Anim/");
	TraverseNonAnimModels(eLevel, L"../../../model_txt/NonAnim/");

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");

	if(FAILED(Add_Shaders(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("객체를(을) 로딩 중 입니다.");

	/* For.Prototype_GameObject_Grid */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Grid"), CGrid);

	///* For.Prototype_GameObject_Camera_Free */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);

	/* For.Prototype_GameObject_Tool_Anim */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tool_Anim"), CTool_Anim);

	/* For.Prototype_GameObject_Tool_NonAnim */
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tool_NonAnim"), CTool_NonAnim);

	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_SlimeBoss(LEVEL eLevel)
{
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");

	/* Prototype_Component_Texture_Terrain */
	if (FAILED(Add_Texture(eLevel, "Terrain", "Terrain/Tile0.jpg")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Grass", "Terrain/grassTile%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/Mask.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Brush", "Terrain/Brush.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "SlimeBoss_Emit", "CommonTextures/slimeBossTex_Emit.dds")))
		return E_FAIL;

	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");

	/* Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 400, 400))))
		return E_FAIL;

	TraverseAnimModels(eLevel, L"../../../model_txt/Anim/");
	TraverseNonAnimModels(eLevel, L"../../../model_txt/NonAnim/");

	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");

	Add_Shaders(eLevel);


	m_strLoadingText = TEXT("객체를(을) 로딩 중 입니다.");

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SlimeBoss"), CSlimeBoss);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_SlimeBoss_Slime"), CSlimeBoss_Slime);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Knife"), CKnife);
	m_strLoadingText = TEXT("로딩이 완료되었습니다.");

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Add_Model(LEVEL eLevel, TYPE eModelType, string strModelName)
{
	_float fScale = 10.f;
	MODEL_CONFIG tConfig{};
	auto ScaleIter = m_ModelConfigs.find(strModelName);
	if (ScaleIter != m_ModelConfigs.end())
		tConfig = ScaleIter->second;

	_matrix		TransformMatrix = XMMatrixIdentity();
	TransformMatrix = XMMatrixScaling(tConfig.fScale, tConfig.fScale, tConfig.fScale) * XMMatrixRotationY(XMConvertToRadians(tConfig.fDegree));

	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring wstrModelName = converter.from_bytes(strModelName);
	wstring wstrPrototypeTag = L"Prototype_Component_Model_" + wstrModelName;

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag,
		CModel::Create(m_pDevice, m_pContext, eModelType, strModelName, TransformMatrix, tConfig))))
		return E_FAIL;


	if (eModelType == TYPE_ANIM)
	{
		_float fNewScale = 1.f;
		if(ScaleIter != m_ModelConfigs.end())
			fNewScale = tConfig.fNewScale;

		TransformMatrix = XMMatrixScaling(fNewScale, fNewScale, fNewScale) * XMMatrixRotationY(XMConvertToRadians(tConfig.fDegree - 180.f));
		if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag + L"_NonAnim",
			CModel::Create(m_pDevice, m_pContext, TYPE_NONANIM, strModelName, TransformMatrix, tConfig, true))))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CLoader::Add_Models_GamePlay()
{
	/* Prototype_Component_Model_Jack */

	return S_OK;
}

HRESULT CLoader::Add_Shaders(LEVEL eLevel)
{
	/* For.Prototype_Component_Shader_VtxNorTex */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxAnimModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxAnimModel"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxAnimModel.hlsl"), VTXANIMMESH::Elements, VTXANIMMESH::iNumElements))))
		return E_FAIL;
	
	/* For.Prototype_Component_Shader_VtxModel */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel_Knife"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel_Knife.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Add_Texture(LEVEL eLevel, string strPrototypeName, string strFolderAndFileName, _uint iNumTextures)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring wstrPrototypeTag = L"Prototype_Component_Texture_" + converter.from_bytes(strPrototypeName);
	wstring wstrFullPath = L"../../../Resources/Textures/" + converter.from_bytes(strFolderAndFileName);

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag,	CTexture::Create(m_pDevice, m_pContext, wstrFullPath, iNumTextures))))
		return E_FAIL;

	return S_OK;
}

void CLoader::Insert_ScaleRotationData() // Scale, Degaree, NewScale, RootIndex
{
	m_ModelConfigs.emplace("Ballista", MODEL_CONFIG{ 0.03f, 0.f, 2.6f });
	m_ModelConfigs.emplace("Bat", MODEL_CONFIG{ 0.005f, 0.f, 0.5f, 4 });
	m_ModelConfigs.emplace("Charon", MODEL_CONFIG{ 0.03f, 180.f, 2.6f, 4 });
	m_ModelConfigs.emplace("Crow", MODEL_CONFIG{ 0.0003f, 180.f, 0.03f, 1 });
	m_ModelConfigs.emplace("Dummy", MODEL_CONFIG{ 0.005f, 0.f, 0.5f, 3 });
	m_ModelConfigs.emplace("GarGoyle", MODEL_CONFIG{ 0.01f, 0.f, 1.f, 4 });
	m_ModelConfigs.emplace("Horse", MODEL_CONFIG{ 0.03f, 180.f, 2.6f, 3 });
	m_ModelConfigs.emplace("Jack", MODEL_CONFIG{ 0.0025f, 180.f, 0.25f, 3 });
	m_ModelConfigs.emplace("Mummy", MODEL_CONFIG{ 0.03f, 0.f, 2.2f, 4 });
	m_ModelConfigs.emplace("Owl", MODEL_CONFIG{ 0.03f, 0.f, 1.f, 4 });
	m_ModelConfigs.emplace("SkeletonBoss", MODEL_CONFIG{ 0.03f, 0.f, 2.6f, 3 });
	m_ModelConfigs.emplace("Slime", MODEL_CONFIG{ 0.01f, 0.f, 1.f, 4 });
	m_ModelConfigs.emplace("SlimeBoss", MODEL_CONFIG{ 0.026f, 0.f, 2.6f, 4 });
	m_ModelConfigs.emplace("PartSlimeBoss", MODEL_CONFIG{ 0.026f, 0.f, 2.6f, 4 });
	m_ModelConfigs.emplace("Sniper", MODEL_CONFIG{ 0.025f, 0.f, 2.6f, 3 });
	m_ModelConfigs.emplace("Soldier", MODEL_CONFIG{ 0.025f, 0.f, 2.6f, 5 });
	m_ModelConfigs.emplace("Trabuchet", MODEL_CONFIG{ 0.03f, 0.f, 2.6f, 2 });

	m_ModelConfigs.emplace("DestroyedBarn", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("bone", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fence1", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fence2", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fence3", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fence4", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fenceClose", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fenceStack", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("fenceStackSmall", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("grassMeshYellow", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("shroom", MODEL_CONFIG{ 3.f, 0.f, 3.f });

	m_ModelConfigs.emplace("Rifle", MODEL_CONFIG{ 3.f, 0.f, 3.f });
	m_ModelConfigs.emplace("Scythe", MODEL_CONFIG{ 0.22f, 0.f, 0.22f });
	m_ModelConfigs.emplace("Spear", MODEL_CONFIG{ 6.4f, 0.f, 6.4f });

	m_ModelConfigs.emplace("Knife", MODEL_CONFIG{ 5.f, 0.f, 5.f });
}

void CLoader::TraverseAnimModels(LEVEL eLevel, const wstring& rootFolderPath)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((rootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return;
	}

	list<wstring> txtList;
	TraverseDirectory(rootFolderPath, txtList);

	for (auto listIter : txtList)
	{
		wstring temp = listIter.substr(0, listIter.length() - 4);
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		string strModelName = converter.to_bytes(temp);

		if (FAILED(Add_Model(eLevel, TYPE_ANIM, strModelName)))
		{
			wstring wstrError = L"Failed to Add: " + temp;
			MSG_BOX(wstrError.c_str());
		}
	}

	FindClose(hFind);
}

void CLoader::TraverseNonAnimModels(LEVEL eLevel, const wstring& rootFolderPath)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((rootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return;
	}

	list<wstring> AnimtxtList;
	TraverseDirectory(L"../../../model_txt/Anim/", AnimtxtList);
	list<wstring> txtList;
	TraverseDirectory(L"../../../model_txt/NonAnim/", txtList);

	for (auto listIter : txtList)
	{
		_bool bNonAnimVersion = { false };
		wstring temp = listIter.substr(0, listIter.length() - 4);
		wstring_convert<codecvt_utf8<wchar_t>> converter;
		string strModelName = converter.to_bytes(temp);

		for (auto Animtxt : AnimtxtList)
		{
			if (Animtxt.find(temp) != wstring::npos)
			{
				bNonAnimVersion = true;
				break;
			}
		}

		if (bNonAnimVersion == false)
		{
			if (FAILED(Add_Model(eLevel, TYPE_NONANIM, strModelName)))
			{
				wstring wstrError = L"Failed to Add: " + temp;
				MSG_BOX(wstrError.c_str());
			}
		}
	}

	FindClose(hFind);
}

void CLoader::TraverseDirectory(const wstring& rootFolderPath, list<wstring>& fileList)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((rootFolderPath + L"\\*").c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		MSG_BOX(TEXT("폴더를 찾을수없습니다"));
		return;
	}

	do {
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			if (wcscmp(findFileData.cFileName, L".") != 0 && wcscmp(findFileData.cFileName, L"..") != 0) {
				// 재귀적으로 하위 폴더도 순회
				TraverseDirectory(rootFolderPath + L"\\" + findFileData.cFileName, fileList);
			}
		}
		else {
			// 파일이면 리스트에 추가
			fileList.push_back(wstring(findFileData.cFileName));
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}

CLoader* CLoader::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eNextLevelID)
{
	CLoader* pInstance = new CLoader(pDevice, pContext);

	if (FAILED(pInstance->Initialize(eNextLevelID)))
	{
		MSG_BOX(TEXT("Failed To Create : CLoader"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLoader::Free()
{
	WaitForSingleObject(m_hThread, INFINITE);

	CoUninitialize();

	DeleteObject(m_hThread);

	CloseHandle(m_hThread);

	DeleteCriticalSection(&m_Critical_Section);

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

