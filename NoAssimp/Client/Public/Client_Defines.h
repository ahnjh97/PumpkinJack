#pragma once


#include "GameInstance.h"

namespace Client
{
	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_MAPTOOL, 
		LEVEL_GAMEPLAY, LEVEL_GAMEPLAY2, LEVEL_GAMEPLAY3, LEVEL_GAMEPLAY4, LEVEL_END };

	const unsigned int	g_iWinSizeX = 1280;
	const unsigned int	g_iWinSizeY = 720;
}


extern HWND g_hWnd;
extern HINSTANCE g_hInst;

using namespace std;
using namespace Client;

#define ADD_GAMEOBJECT_PROTOTYPE(tag, className) \
    do { \
        wstring wstrResult = L"Prototype_GameObject_" + wstring(tag); \
        if (FAILED(m_pGameInstance->Add_Prototype(wstring(wstrResult), className::Create(m_pDevice, m_pContext)))) \
            return E_FAIL; \
    } while (0)

//#define ADD_COMPONENT_PROTOTYPE(tag, className, filePath) \
//    do { \
//        wstring wstrResult = L"Prototype_Component_" + wstring(tag); \
//        wstring wstrFullPath = L"../../../Resources/" + wstring(filePath);   \
//        if (FAILED(m_pGameInstance->Add_Prototype(wstring(wstrResult), className::Create(m_pDevice, m_pContext, wstring(wstrFullPath))))) \
//            return E_FAIL; \
//    } while (0)
