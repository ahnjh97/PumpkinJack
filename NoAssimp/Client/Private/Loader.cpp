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
#include "Sky.h"

#include "BallistaRotator.h"
#include "SkullCrossBow.h"
#include "SoldierWeapon.h"
#include "CollisionCube.h"
#include "Map_GamePlay.h"
#include "Ballista.h"
#include "CrateBox.h"
#include "Trigger.h"
#include "Soldier.h"
#include "Arrow.h"
#include "Kart.h"
#include "Rail.h"

#include "Terrain_SlimeBossRoom.h"
#include "Pillar_SlimeBossRoom.h"
#include "SlimeBoss_Slime.h"
#include "SlimeDamageZone.h"
#include "SlimeBossHead.h"
#include "Map_SlimeBoss.h"
#include "Slime_Slime.h"
#include "SlimeBoss.h"
#include "Pillar.h"
#include "Portal.h"
#include "Knife.h"
#include "Slime.h"

#include "WizardTower.h"
#include "Map_Wizard.h"
#include "TeslaTower.h"
#include "ElecArena.h"
#include "SkySphere.h"
#include "ElecParts.h"
#include "Lightning.h"
#include "HoleWall.h"
#include "Tornado.h"
#include "Wizard.h"
#include "Book.h"

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

	SetUp_ModelScaleRotation();
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
	case LEVEL_WIZARD:
		hr = Loading_For_Wizard(LEVEL_WIZARD);
		break;
	case LEVEL_DEATH:
		hr = Loading_For_Death(LEVEL_DEATH);
		break;
	case LEVEL_EFFECTTOOL:
		hr = Loading_For_EffectTool(LEVEL_EFFECTTOOL);
		break;
	}

	if (FAILED(hr))
		return E_FAIL;

	LeaveCriticalSection(&m_Critical_Section);

	return S_OK;
}

HRESULT CLoader::Loading_For_Logo()
{
	m_strLoadingText = TEXT("Loading Textures for Level_Logo...");
	/* For.Prototype_Component_Texture_Logo */
	if(FAILED(Add_Texture(LEVEL_LOGO, "Logo", "Logo/wallpaper.jpg")))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Models for Level_Logo...");

	m_strLoadingText = TEXT("Loading Shaders for Level_Logo...");

	m_strLoadingText = TEXT("Loading GameObject Prototypes for Level_Logo...");

	/* For.Prototype_GameObject_BackGround */
	if (FAILED(m_pGameInstance->Add_Prototype(TEXT("Prototype_GameObject_BackGround"),
		CBackGround::Create(m_pDevice, m_pContext))))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Complete: Level_Logo");

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_MapTool(LEVEL eLevel)
{
	m_strLoadingText = TEXT("텍스쳐를(을) 로딩 중 입니다.");
	if (FAILED(Add_Texture(eLevel, "Grid", "Terrain/White.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Terrain", "Terrain/Tile0.jpg")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Grass", "Terrain/grassTile%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/WaterMask.bmp")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Water", "Terrain/Water.dds")))
		return E_FAIL;


	m_strLoadingText = TEXT("컴포넌트를(을) 로딩 중 입니다.");
	/* Prototype_Component_VIBuffer_Terrain_GamePlay */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_GamePlay"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/Level_GamePlay.bmp")))))
		return E_FAIL;

	/* Prototype_Component_VIBuffer_Terrain_SlimeBoss */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_SlimeBoss"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/Level_SlimeBoss.bmp")))))
		return E_FAIL;

	/* Prototype_Component_VIBuffer_Terrain_SlimeBossRoom */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_SlimeBossRoom"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/SlimeBossRoom.bmp")))))
		return E_FAIL;

	/* Prototype_Component_VIBuffer_Terrain_Wizard */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_Wizard"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/Level_Wizard.bmp")))))
		return E_FAIL;


	m_strLoadingText = TEXT("모델를(을) 로딩 중 입니다.");
	TraverseAnimModels(eLevel, L"../../../model_txt/Anim/");
	TraverseNonAnimModels(eLevel, L"../../../model_txt/NonAnim/");


	m_strLoadingText = TEXT("셰이더를(을) 로딩 중 입니다.");
	if (FAILED(Add_Shaders(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("객체를(을) 로딩 중 입니다.");

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Grid"), CGrid);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tool_Anim"), CTool_Anim);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tool_NonAnim"), CTool_NonAnim);


	m_strLoadingText = TEXT("로딩이 완료되었습니다.");
	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_GamePlay(LEVEL eLevel)
{
	m_strLoadingText = TEXT("Loading Textures for Level_GamePlay...");
	if (FAILED(Add_Texture(eLevel, "Terrain", "Terrain/Tile0.jpg")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Grass", "Terrain/grassTile%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/WaterMask.bmp")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Water", "Terrain/Water.dds")))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading NavigationCells for Level_GamePlay...");
	/* Prototype_Component_VIBuffer_Terrain_GamePlay */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_GamePlay"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/Level_GamePlay.bmp")))))
		return E_FAIL;

	/* For.Prototype_Component_Navigation */
	wstring wstrNavigationTag = TEXT("Prototype_Component_Navigation") + to_wstring(eLevel);
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrNavigationTag,
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../../../objects_txt/Level_GamePlay_Cells.txt")))))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Models for Level_GamePlay...");
	vector<string> vecAnimModelNames = { "Jack", "Ballista", "Dummy", "SkullCrossBow", "Soldier", "Trabuchet" };
	vector<string> vecNonAnimModelNames = { "Spear", "Rifle", "Scythe", "LevelGamePlay",
		"ballistaArrow", "ballistaBase", "ballistaRotator", "crateBoxPotion", "fenceStack_SmallCol", "Cube",
		"Kart", "MinePlatform", "Rail", "RailLeft60", "RailLeft90", "RailRight60", "RailRight90" };

	if (FAILED(Add_Models(eLevel, vecAnimModelNames, vecNonAnimModelNames)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Shaders for Level_GamePlay...");
	if (FAILED(Add_Shaders(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading GameObject Prototypes for Level_GamePlay...");
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Terrain"), CTerrain);
	//ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Jack"), CCamera_Jack);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Monster"), CMonster);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Player"), CPlayer); 
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Body"), CBody);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Spear"), CWeapon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Rifle"), CWeapon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Scythe"), CWeapon);

	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TongMap_GamePlay"), CMap_GamePlay);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Ballista"), CBallista);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Arrow"), CArrow);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("BallistaRotator"), CBallistaRotator);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SkullCrossBow"), CSkullCrossBow);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Kart"), CKart);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Rail"), CRail);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CrateBox"), CCrateBox);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Trigger"), CTrigger);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Soldier"), CSoldier);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_SoldierWeapon"), CSoldierWeapon);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("CollisionCube"), CCollisionCube);

	m_strLoadingText = TEXT("Loading Complete: Level_GamePlay");

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_SlimeBoss(LEVEL eLevel)
{
	m_strLoadingText = TEXT("Loading Textures for Level_SlimeBoss...");
	if (FAILED(Add_Texture(eLevel, "Terrain", "Terrain/Tile0.jpg")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Grass", "Terrain/grassTile%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/Mask.png")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "SlimeBoss_Emit", "CommonTextures/slimeBossTex_Emit.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "pineTreeAlpha_Tex", "CommonTextures/pineTreeAlpha_Tex.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Water", "CommonTextures/Water.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "waterCausticText", "CommonTextures/waterCausticText.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Portal", "Portal/Portal.dds")))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading NavigationCells for Level_SlimeBoss...");
	/* Prototype_Component_VIBuffer_Terrain_SlimeBoss */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_SlimeBoss"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/Level_SlimeBoss.bmp")))))
		return E_FAIL;

	/* Prototype_Component_VIBuffer_Terrain_SlimeBossRoom */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain_SlimeBossRoom"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, TEXT("../../../Resources/Textures/Terrain/SlimeBossRoom.bmp")))))
		return E_FAIL;

	/* For.Prototype_Component_Navigation */
	wstring wstrNavigationTag = TEXT("Prototype_Component_Navigation") + to_wstring(eLevel);
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrNavigationTag,
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../../../objects_txt/Level_SlimeBoss_Cells.txt")))))
		return E_FAIL;

	wstrNavigationTag = TEXT("Prototype_Component_Navigation_SlimeBossRoom");
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrNavigationTag,
		CNavigation::Create(m_pDevice, m_pContext, TEXT("../../../objects_txt/SlimeBossRoom_Cells.txt")))))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Models for Level_SlimeBoss...");
	vector<string> vecAnimModelNames = { "Jack", "PartSlime", "PartSlimeBoss", "Slime", "SlimeBoss" };
	vector<string> vecNonAnimModelNames = { "Spear", "Rifle", "Scythe", "LevelSlimeBoss",
		"cemeteryFence1", "cemeteryPillar", "Knife", "NotOpen", "SlimeBossHead", "SlimeDamageZone"};

	if (FAILED(Add_Models(eLevel, vecAnimModelNames, vecNonAnimModelNames)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Shaders for Level_SlimeBoss...");
	if (FAILED(Add_Shaders(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading GameObject Prototypes for Level_SlimeBoss...");
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SlimeBoss"), CSlimeBoss);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_SlimeBoss_Slime"), CSlimeBoss_Slime);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Knife"), CKnife);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TongMap_SlimeBoss"), CMap_SlimeBoss);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("cemeteryPillar"), CPillar);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Portal"), CPortal);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Pillar_SlimeBossRoom"), CPillar_SlimeBossRoom);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SlimeBossHead"), CSlimeBossHead);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Slime"), CSlime);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Part_Slime_Slime"), CSlime_Slime);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SlimeDamageZone"), CSlimeDamageZone);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Terrain_SlimeBossRoom"), CTerrain_SlimeBossRoom);


	m_strLoadingText = TEXT("Loading Complete: Level_SlimeBoss");

	m_isFinished = true;

	return S_OK;
}

HRESULT CLoader::Loading_For_Wizard(LEVEL eLevel)
{
	m_strLoadingText = TEXT("Loading Textures for Level_Wizard...");
	if (FAILED(Add_Texture(eLevel, "VoronoiNoise", "CommonTextures/voronoiNoise.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "TigerStripes", "Effects/tigerStripes.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "TopBottomFade", "Effects/topBottomFadeTex.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "ElecParts", "Effects/elecParts.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Noise", "Effects/reaperSolid.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Circle", "Effects/Circle%d.dds" ,2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "FireNoise", "Effects/fireNoise.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "VoronoiBlur", "Effects/voronoiNoiseBlur.dds")))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading VIBuffers for Level_Wizard...");

	if (FAILED(Add_Instance_Models(eLevel)))
		return E_FAIL;

	if (FAILED(Add_VIBuffer_Instance_Points(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Models for Level_Wizard...");
	vector<string> vecAnimModelNames = 	{ "Jack", "Wizard" };
	vector<string> vecNonAnimModelNames = { "Spear", "Rifle", "Scythe", "LevelWizard",
		"Amulet", "Book", "elecArena1", "elecArena2", "elecArena3", "HoleWall", "SkySphere",
		"TeslaTower", "Tornado", "WizardBrickGround", "WizardTower" };

	if (FAILED(Add_Models(eLevel, vecAnimModelNames, vecNonAnimModelNames)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Shaders for Level_Wizard...");
	if (FAILED(Add_Shaders(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading GameObject Prototypes for Level_Wizard...");
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Wizard"), CWizard);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TongMap_Wizard"), CMap_Wizard);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("SkySphere"), CSkySphere);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tornado"), CTornado);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HoleWall"), CHoleWall);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("WizardTower"), CWizardTower);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("TeslaTower"), CTeslaTower);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("ElecArena"), CElecArena);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Book"), CBook);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("ElecParts"), CElecParts);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Lightning"), CLightning);


	m_strLoadingText = TEXT("Loading Complete: Level_Wizard");
	m_isFinished = true;
	return S_OK;
}

HRESULT CLoader::Loading_For_Death(LEVEL eLevel)
{

	return S_OK;
}

HRESULT CLoader::Loading_For_EffectTool(LEVEL eLevel)
{
	m_strLoadingText = TEXT("Loading Textures for Level_EffectTool...");
	if (FAILED(Add_Texture(eLevel, "Ground", "Terrain/groundTile%d.dds", 4)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Mask", "Terrain/WaterMask.bmp")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Water", "Terrain/Water.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "VoronoiNoise", "CommonTextures/voronoiNoise.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "TigerStripes", "Effects/tigerStripes.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "TopBottomFade", "Effects/topBottomFadeTex.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "ElecParts", "Effects/elecParts.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Noise", "Effects/reaperSolid.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Portal", "Portal/Portal.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "Circle", "Effects/Circle%d.dds", 2)))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "FireNoise", "Effects/fireNoise.dds")))
		return E_FAIL;
	if (FAILED(Add_Texture(eLevel, "VoronoiBlur", "Effects/voronoiNoiseBlur.dds")))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading VIBuffers for Level_EffectTool...");
	/* Prototype_Component_VIBuffer_Terrain */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_VIBuffer_Terrain"),
		CVIBuffer_Terrain::Create(m_pDevice, m_pContext, 100, 100))))
		return E_FAIL;

	if(FAILED(Add_Instance_Models(eLevel)))
		return E_FAIL;

	if (FAILED(Add_VIBuffer_Instance_Points(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Models for Level_EffectTool...");
	if (FAILED(Add_Model(eLevel, TYPE_NONANIM, "HoleWall")))
		return E_FAIL;
	if (FAILED(Add_Model(eLevel, TYPE_NONANIM, "Book")))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading Shaders for Level_EffectTool...");
	if (FAILED(Add_Shaders(eLevel)))
		return E_FAIL;


	m_strLoadingText = TEXT("Loading GameObject Prototypes for Level_EffectTool...");
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Terrain"), CTerrain);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Tornado"), CTornado);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Camera_Free"), CCamera_Free);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Book"), CBook);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Monster"), CMonster);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("ElecParts"), CElecParts);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("Lightning"), CLightning);
	ADD_GAMEOBJECT_PROTOTYPE(TEXT("HoleWall"), CHoleWall);

	m_strLoadingText = TEXT("Loading Complete: Level_EffectTool");
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

HRESULT CLoader::Add_Models(LEVEL eLevel, vector<string> _vecAnimModelNames, vector<string> _vecNonAnimModelNames)
{
	if (_vecAnimModelNames.empty() || _vecNonAnimModelNames.empty())
		return S_OK;

	for (auto& strAnimModelName : _vecAnimModelNames)
	{
		if (FAILED(Add_Model(eLevel, TYPE_ANIM, strAnimModelName))) {
			MB("Failed to Add : " + strAnimModelName);
			return E_FAIL;
		}
	}

	for (auto& strNonAnimModelName : _vecNonAnimModelNames)
	{
		if (FAILED(Add_Model(eLevel, TYPE_NONANIM, strNonAnimModelName))) {
			MB("Failed to Add : " + strNonAnimModelName);
			return E_FAIL;
		}
	}

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
	
	/* For.Prototype_Component_Shader_VtxModel_Lighted */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxModel_Lighted"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxModel_Lighted.hlsl"), VTXMESH::Elements, VTXMESH::iNumElements))))
		return E_FAIL;

	/* For.Prototype_Component_Shader_VtxInstance_Mesh */
	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxInstance_Mesh"),
		CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxInstance_Mesh.hlsl"), VTXINSTANCE_MESH::Elements, VTXINSTANCE_MESH::iNumElements))))
		return E_FAIL;

	if (eLevel == LEVEL_SLIMEBOSS) {
		/* For.Prototype_Component_Shader_VtxNorTex_LevelSlimeBoss */
		if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Shader_VtxNorTex_LevelSlimeBoss"),
			CShader::Create(m_pDevice, m_pContext, TEXT("../Bin/ShaderFiles/Shader_VtxNorTex_LevelSlimeBoss.hlsl"), VTXNORTEX::Elements, VTXNORTEX::iNumElements))))
			return E_FAIL;
	}

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

HRESULT CLoader::Add_Instance_Models(LEVEL eLevel)
{
	CVIBuffer_Instance::INSTANCE_DESC		InstanceDesc{};
	if (eLevel == LEVEL_WIZARD) {
		InstanceDesc.vPivot = _float3(580.f, -150.f, 580.f);
		InstanceDesc.vCenter = _float3(580.f, -50.f, 580.f);
		InstanceDesc.vRange = _float3(300.f, 50.f, 300.f);
		InstanceDesc.vMinScale = _float3(6.f, 4.f, 6.f);
		InstanceDesc.vMaxScale = _float3(9.f, 6.f, 9.f);
		InstanceDesc.iNumInstance = 200;
		InstanceDesc.vLifeTime = _float2(8.f, 12.f);
		InstanceDesc.isLoop = true;
		InstanceDesc.vSpeed = _float2(40.f, 70.f);
	}
	else if (eLevel == LEVEL_EFFECTTOOL) {
		InstanceDesc.vPivot = _float3(0.f, -5.f, 0.f);
		InstanceDesc.vCenter = _float3(0.f, 0.f, 0.f);
		InstanceDesc.vRange = _float3(4.f, 3.f, 3.f);
		InstanceDesc.vMinScale = _float3(2.f, 2.f, 2.f);
		InstanceDesc.vMaxScale = _float3(3, 3, 3);
		InstanceDesc.iNumInstance = 20;
		InstanceDesc.vLifeTime = _float2(3.f, 6.0f);
		InstanceDesc.isLoop = true;
		InstanceDesc.vSpeed = _float2(2.f, 15.f);
	}

	if (FAILED(m_pGameInstance->Add_Prototype(eLevel, TEXT("Prototype_Component_Instance_Model"),
		CInstance_Model::Create(m_pDevice, m_pContext, "Book", InstanceDesc))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoader::Add_VIBuffer_Instance_Points(LEVEL eLevel)
{
	vector<CVIBuffer_Instance::INSTANCE_DESC> vecInstanceDesc;
	vecInstanceDesc.resize(CElecParts::ELEC_END);

	// ELEC_TORNADO
	CVIBuffer_Instance::INSTANCE_DESC		InstanceDesc{};
	InstanceDesc.vCenter = _float3(0.f, 18.f, 0.f);
	InstanceDesc.vRange = _float3(20.f, 36.f, 20.f);
	InstanceDesc.vMinScale = _float3(16.f, 16.f, 16.f);
	InstanceDesc.vMaxScale = _float3(18, 18, 18);
	InstanceDesc.iNumInstance = 30;
	InstanceDesc.vLifeTime = _float2(0.5f, 0.8f);
	InstanceDesc.isLoop = true;
	InstanceDesc.iSpriteIndexOffset = 8;
	InstanceDesc.eShape = CVIBuffer_Instance::SHAPE_DEFAULT;
	vecInstanceDesc[CElecParts::ELEC_TORNADO] = InstanceDesc;

	InstanceDesc.vCenter = _float3(0.f, 0.f, 0.f);
	InstanceDesc.vRange = _float3(10.f, 10.f, 10.f);
	InstanceDesc.iNumInstance = 30;
	InstanceDesc.eShape = CVIBuffer_Instance::SHAPE_SPHERE;
	vecInstanceDesc[CElecParts::ELEC_LIGHTNING1] = InstanceDesc;

	InstanceDesc.vCenter = _float3(0.f, 0.f, 0.f);
	InstanceDesc.vRange = _float3(30.f, 30.f, 20.f);
	InstanceDesc.iNumInstance = 70;
	InstanceDesc.eShape = CVIBuffer_Instance::SHAPE_RING;
	vecInstanceDesc[CElecParts::ELEC_HOLE] = InstanceDesc;

	if (vecInstanceDesc.empty())
		return S_OK;

	for (_int i = 0; i < CElecParts::ELEC_END; i++) {
		if (vecInstanceDesc[i].iSpriteIndexOffset != 0) {
			wstring wstrPrototypeTag = TEXT("Prototype_Component_VIBuffer_Instance_Point") + to_wstring(i);
			if (FAILED(m_pGameInstance->Add_Prototype(eLevel, wstrPrototypeTag, CVIBuffer_Instance_Point::Create(m_pDevice, m_pContext, vecInstanceDesc[i]))))
				return E_FAIL;
		}
	}

	return S_OK;
}

void CLoader::SetUp_ModelScaleRotation() // Scale, Degaree, NewScale, RootIndex
{
	m_ModelConfigs.emplace("Ballista", MODEL_CONFIG{ 0.1f, 180.f, 9.f });
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
	m_ModelConfigs.emplace("Slime", MODEL_CONFIG{ 0.027f, 180.f, 2.7f });
	m_ModelConfigs.emplace("PartSlime", MODEL_CONFIG{ 0.027f, 180.f, 2.7f });
	m_ModelConfigs.emplace("SlimeBoss", MODEL_CONFIG{ 0.044f, 180.f, 4.4f });
	m_ModelConfigs.emplace("PartSlimeBoss", MODEL_CONFIG{ 0.044f, 180.f, 4.4f });
	m_ModelConfigs.emplace("Sniper", MODEL_CONFIG{ 0.042f, 180.f, 4.2f, 3 });
	m_ModelConfigs.emplace("Soldier", MODEL_CONFIG{ 0.042f, 180.f, 4.2f, 5 });
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

	/* Level_GamePlay */
	m_ModelConfigs.emplace("ballistaArrow", MODEL_CONFIG{ 9.f, 0.f, 9.f });
	m_ModelConfigs.emplace("ballistaBase", MODEL_CONFIG{ 9.f, 0.f, 9.f });
	m_ModelConfigs.emplace("ballistaRotator", MODEL_CONFIG{ 9.f, 0.f, 9.f });
	m_ModelConfigs.emplace("SkullCrossBow", MODEL_CONFIG{ 0.07f, 0.f, 7.f });
	m_ModelConfigs.emplace("Kart", MODEL_CONFIG{ 7.f, 0.f, 7.f });
	m_ModelConfigs.emplace("crateBoxPotion", MODEL_CONFIG{ 6.f, 0.f, 6.f });
	m_ModelConfigs.emplace("Rail", MODEL_CONFIG{ 5.8f, 180.f, 5.8f });
	m_ModelConfigs.emplace("RailLeft60", MODEL_CONFIG{ 5.8f, 180.f, 5.8f });
	m_ModelConfigs.emplace("RailLeft90", MODEL_CONFIG{ 5.8f, 180.f, 5.8f });
	m_ModelConfigs.emplace("RailRight60", MODEL_CONFIG{ 5.8f, 180.f, 5.8f });
	m_ModelConfigs.emplace("RailRight90", MODEL_CONFIG{ 5.8f, 180.f, 5.8f });
	m_ModelConfigs.emplace("MinePlatform", MODEL_CONFIG{ 4.2f, 0.f, 4.2f });

	/* Level_SlimeBoss */
	m_ModelConfigs.emplace("Knife", MODEL_CONFIG{ 8.2f, 0.f, 8.2f });
	m_ModelConfigs.emplace("NotOpen", MODEL_CONFIG{ 4.1f, 0.f, 4.1f });
	m_ModelConfigs.emplace("cemeteryPillar", MODEL_CONFIG{ 6.5f, 0.f, 6.5f });
	m_ModelConfigs.emplace("SlimeBossHead", MODEL_CONFIG{ 10.f, 0.f, 10.f });
	m_ModelConfigs.emplace("SlimeDamageZone", MODEL_CONFIG{ 6.f, 0.f, 6.f });
	m_ModelConfigs.emplace("cemeteryFence1", MODEL_CONFIG{ 4.3f, 0.f, 4.3f });

	/* Level_Wizard */
	m_ModelConfigs.emplace("Wizard", MODEL_CONFIG{ 0.12f, 180.f, 12.f });
	m_ModelConfigs.emplace("LevelWizard", MODEL_CONFIG{ 2.4f, 0.f, 2.4f });
	m_ModelConfigs.emplace("Amulet", MODEL_CONFIG{ 5.2f, 0.f, 5.2f });
	m_ModelConfigs.emplace("Book", MODEL_CONFIG{ 4.f, 0.f, 4.f });
	for (_int i = 1; i < 4; i++)
		m_ModelConfigs.emplace("elecArena" + to_string(i), MODEL_CONFIG{ 14.f, 0.f, 14.f });
	m_ModelConfigs.emplace("HoleWall", MODEL_CONFIG{ 12.f, 0.f, 12.f });
	m_ModelConfigs.emplace("TeslaTower", MODEL_CONFIG{ 20.f, 0.f, 20.f });
	//m_ModelConfigs.emplace("Tornado", MODEL_CONFIG{ 15.f, 0.f, 15.f });
	m_ModelConfigs.emplace("WizardBrickGround", MODEL_CONFIG{ 12.f, 0.f, 12.f });
	m_ModelConfigs.emplace("WizardTower", MODEL_CONFIG{ 15.f, 0.f, 15.f });
	m_ModelConfigs.emplace("SkySphere", MODEL_CONFIG{ 0.2f, 0.f, 0.2f });
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

		if (strModelName.find("Level") != string::npos) // 문자열에 Level이 포함될경우
		{
			switch (eLevel) // 해당 레벨의 맵 Model만 불러오기를 하도록 
			{
			case LEVEL_GAMEPLAY:
				if (strModelName != "LevelGamePlay")
					continue;
				break;
			case LEVEL_SLIMEBOSS:
				if (strModelName != "LevelSlimeBoss")
					continue;
				break;
			case LEVEL_WIZARD:
				if (strModelName != "LevelWizard")
					continue;
				break;
			}
		}

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

