#pragma once
#include "Base.h"
//#include "../FMOD/include/fmod.h"
#include "Engine_Defines.h"

BEGIN(Engine)

#define SOUND_MAX 1.0f
#define SOUND_MIN 0.0f
#define SOUND_DEFAULT 0.5f
#define SOUND_WEIGHT 0.1f

class ENGINE_DLL CSound_Manager final : public CBase
{
private:
	CSound_Manager();
	~CSound_Manager() = default;

public:
	HRESULT Initialize();

	//void Release();
public:
	_int  SetVolume(CHANNELID eID, _float _vol);
	_int  VolumeUp(CHANNELID eID, _float _vol);
	_int  VolumeDown(CHANNELID eID, _float _vol);
	_int  VolumeMin(CHANNELID eID);
	_int  BGMVolumeUp(_float _vol);
	_int  BGMVolumeDown(_float _vol);

	_int  Pause(CHANNELID eID);
	void Play_Sound(TCHAR* pSoundKey, CHANNELID eID, _float _vol);
	void PlayBGM(TCHAR* pSoundKey);
	void StopSound(CHANNELID eID);
	void StopAll();

private:
	float m_volume = SOUND_DEFAULT;
	float m_BGMvolume = SOUND_DEFAULT;
	FMOD_BOOL m_bool;

private:
	void LoadSoundFile();

private:
	// 사운드 리소스 정보를 갖는 객체 
	map<TCHAR*, FMOD_SOUND*> m_mapSound;
	// FMOD_CHANNEL : 재생하고 있는 사운드를 관리할 객체 
	FMOD_CHANNEL* m_pChannelArr[MAXCHANNEL];
	// 사운드 ,채널 객체 및 장치를 관리하는 객체 
	FMOD_SYSTEM* m_pSystem;
	_bool		m_bPause = false;

public:
	static CSound_Manager* Create();
	virtual void Free() override;
};

END