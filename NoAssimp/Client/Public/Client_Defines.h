#pragma once


#include "GameInstance.h"

namespace Client
{
	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_MAPTOOL, 
		LEVEL_GAMEPLAY, LEVEL_GAMEPLAY2, LEVEL_GAMEPLAY3, LEVEL_SLIMEBOSS, LEVEL_END };
    enum STATE { IDLE, RUN, JUMP, ROLL, ATTACK1, ATTACK2, ATTACK3, AIR_ATTACK, AIR_ATTACK_END, ROLL_ATTACK, PLAYER_END };
	enum SLIMEBOSS { SLIMEBOSS_DEATH, SLIMEBOSS_FORK, SLIMEBOSS_HIT, SLIMEBOSS_IDLE, SLIMEBOSS_INVOKE, SLIMEBOSS_INVOKE_BEGIN
		, SLIMEBOSS_LIGHTNING, SLIMEBOSS_MEGASPAWN, SLIMEBOSS_NOT_OPEN, SLIMEBOSS_PILLAR_SPAWN, SLIMEBOSS_REAP, SLIMEBOSS_REAP_BEGIN
		, SLIMEBOSS_REAP_END, SLIMEBOSS_SPAWN, SLIMEBOSS_END};
    enum WEAPON { WPN_SPEAR, WPN_RIFLE, WPN_SCYTHE, WPN_END };
	const unsigned int	g_iWinSizeX = 1280;
	const unsigned int	g_iWinSizeY = 720;

    typedef struct
    {
		STATE eState = { PLAYER_END };
		STATE ePreState = { IDLE };
		WEAPON eWeapon = { };
		
		_bool bIsJumping = { false };
		_bool bIsAttacking = { false };
		_bool bIsRolling = { false };
		_bool bAtkPreInput = { false };
		_bool bRollPreInput = { false };
		_bool bJumpPreInput = { false };

		_uint iLastAttack = { 3 };

		_float fTempJumpTime = { 0.f };
		_float fRollEndTime = { 0.f };

		_float3	vDirection = {};
    }INFO;
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

#define fZ_LifeBarMask 0.8f
#define fZ_CrowLifeBar 0.7f
