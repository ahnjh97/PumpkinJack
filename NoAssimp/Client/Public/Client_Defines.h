#pragma once


#include "GameInstance.h"

namespace Client
{
	enum LEVEL { LEVEL_STATIC, LEVEL_LOADING, LEVEL_LOGO, LEVEL_MAPTOOL, 
		LEVEL_GAMEPLAY, LEVEL_SLIMEBOSS, LEVEL_WIZARD, LEVEL_DEATH, LEVEL_EFFECTTOOL, LEVEL_END };
    enum STATE { IDLE, RUN, JUMP, ROLL, ATTACK1, ATTACK2, ATTACK3, AIR_ATTACK, AIR_ATTACK_END, ROLL_ATTACK, HIGHFALL, PUSHING, DRINK, PLAYER_END };
	enum SLIMEBOSS { SLIMEBOSS_DEATH, SLIMEBOSS_FORK, SLIMEBOSS_HIT, SLIMEBOSS_IDLE, SLIMEBOSS_INVOKE, SLIMEBOSS_INVOKE_BEGIN
		, SLIMEBOSS_LIGHTNING, SLIMEBOSS_MEGASPAWN, SLIMEBOSS_NOT_OPEN, SLIMEBOSS_PILLAR_SPAWN, SLIMEBOSS_REAP, SLIMEBOSS_REAP_BEGIN
		, SLIMEBOSS_REAP_END, SLIMEBOSS_SPAWN, SLIMEBOSS_END};
	enum SLIME { SLIME_ATTACK, SLIME_IDLE, SLIME_MOVE, SLIME_DEATH };
    enum WEAPON { WPN_SPEAR, WPN_RIFLE, WPN_SCYTHE, WPN_END };
	enum BALLISTA { BALLISTA_IDLE, BALLISTA_SHOOT, BALLISTA_END };
	enum SOLDIER { SOLDIER_SPAWN, SOLDIER_IDLE, SOLDIER_RUN, SOLDIER_ATTACK, SOLDIER_SPIN, SOLDIER_HIT, SOLDIER_DEATH, SOLDIER_END };
	enum WIZARD { WIZARD_ATTACK, WIZARD_CHARGE, WIZARD_AIR_IDLE, WIZARD_DIE, WIZARD_DIE2, WIZARD_HIT, WIZARD_IDLE, WIZARD_WEAK, WIZARD_END };
	enum SPELL { SPELL_LIGHTNING, SPELL_TORNADO, SPELL_HOLEWALL, SPELL_END };

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

		_uint iLastAttack = { 3 };

		_float fTempJumpTime = { 0.f };
		_float fRollEndTime = { 0.f };

		_float3	vDirection = {};
    }INFO;

	typedef struct
	{
		WIZARD eState;
		string strCurAnim;
		_int iWizardPhase;
		_int iAttackCount;
		_int iAttackFinishCount;
		SPELL eSpell;
		_float fTime;
		_float fIdleTime;
		_float fWaitTime;
		_float fChargeTime;
		_float fPhaseChangeTime;
	}INFO_WIZARD;
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
