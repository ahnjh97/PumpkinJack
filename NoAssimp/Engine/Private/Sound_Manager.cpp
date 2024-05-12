#include "Sound_Manager.h"

CSound_Manager::CSound_Manager()
{
	m_pSystem = nullptr;
}

HRESULT CSound_Manager::Initialize()
{
	FMOD_System_Create(&m_pSystem, FMOD_VERSION);

	// 1. 시스템 포인터, 2. 사용할 가상채널 수 , 초기화 방식) 
	FMOD_System_Init(m_pSystem, 32, FMOD_INIT_NORMAL, NULL);

	LoadSoundFile();
	return S_OK;
}

//void CSound_Manager::Release()
//{
//	for (auto& Mypair : m_mapSound)
//	{
//		delete[] Mypair.first;
//		FMOD_Sound_Release(Mypair.second);
//	}
//	m_mapSound.clear();
//	FMOD_System_Release(m_pSystem);
//	FMOD_System_Close(m_pSystem);
//}

_int CSound_Manager::SetVolume(CHANNELID eID, _float _vol)
{
	FMOD_Channel_SetVolume(m_pChannelArr[eID], _vol);

	return 0;

}

_int CSound_Manager::VolumeUp(CHANNELID eID, _float _vol)
{
	if (m_volume < SOUND_MAX) {
		m_volume += _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[eID], m_volume);

	return 0;
}

_int CSound_Manager::VolumeDown(CHANNELID eID, _float _vol)
{
	if (m_volume > SOUND_MIN) {
		m_volume -= _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[eID], m_volume);

	return 0;
}

_int CSound_Manager::VolumeMin(CHANNELID eID)
{
	if (m_volume > SOUND_MIN) {
		m_volume = SOUND_MIN;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[eID], m_volume);

	return 0;
}

_int CSound_Manager::BGMVolumeUp(_float _vol)
{
	if (m_BGMvolume < SOUND_MAX) {
		m_BGMvolume += _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[BGM], m_BGMvolume);

	return 0;
}

_int CSound_Manager::BGMVolumeDown(_float _vol)
{
	if (m_BGMvolume > SOUND_MIN) {
		m_BGMvolume -= _vol;
	}

	FMOD_Channel_SetVolume(m_pChannelArr[BGM], m_BGMvolume);

	return 0;
}

_int CSound_Manager::Pause(CHANNELID eID)
{
	m_bPause = !m_bPause;
	FMOD_Channel_SetPaused(m_pChannelArr[eID], m_bPause);

	return 0;
}

void CSound_Manager::Play_Sound(TCHAR * pSoundKey, CHANNELID eID, _float _vol)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
	{
		return !lstrcmp(pSoundKey, iter.first);
	});

	if (iter == m_mapSound.end())
		return;

	FMOD_BOOL bPlay = FALSE;
	if (FMOD_Channel_IsPlaying(m_pChannelArr[eID], &bPlay))
	{
		FMOD_System_PlaySound(m_pSystem, iter->second, nullptr,FALSE, &m_pChannelArr[eID]);
		if (_vol >= SOUND_MAX)
			_vol = 1.f;
		else if (_vol <= SOUND_MIN)
			_vol = 0.f;
		FMOD_Channel_SetVolume(m_pChannelArr[eID], _vol);
	}
	FMOD_System_Update(m_pSystem);
}

void CSound_Manager::PlayBGM(TCHAR * pSoundKey)
{
	map<TCHAR*, FMOD_SOUND*>::iterator iter;

	iter = find_if(m_mapSound.begin(), m_mapSound.end(), [&](auto& iter)
	{
		return !lstrcmp(pSoundKey, iter.first);
	});

	if (iter == m_mapSound.end())
		return;
	
	FMOD_System_PlaySound(m_pSystem,iter->second,nullptr, FALSE, &m_pChannelArr[BGM]);
	FMOD_Channel_SetMode(m_pChannelArr[BGM], FMOD_LOOP_NORMAL);
	FMOD_System_Update(m_pSystem);
}

void CSound_Manager::StopSound(CHANNELID eID)
{
	FMOD_Channel_Stop(m_pChannelArr[eID]);
}

void CSound_Manager::StopAll()
{
	for (int i = 0; i < MAXCHANNEL; ++i)
		FMOD_Channel_Stop(m_pChannelArr[i]);
}

void CSound_Manager::LoadSoundFile()
{
	_tfinddata64_t fd;
	__int64 handle = _tfindfirst64(L"../Bin/Sound/*.*", &fd);
	if (handle == -1 || handle == 0)
		return;

	_int iResult = 0;

	_char szCurPath[128] = "../Bin/Sound/";
	_char szFullPath[128] = "";
	_char szFilename[MAX_PATH];
	while (iResult != -1)
	{
		WideCharToMultiByte(CP_UTF8, 0, fd.name, -1, szFilename, sizeof(szFilename), NULL, NULL);
		strcpy_s(szFullPath, szCurPath);
		strcat_s(szFullPath, szFilename);
		FMOD_SOUND* pSound = nullptr;

		FMOD_RESULT eRes = FMOD_System_CreateSound(m_pSystem, szFullPath, FMOD_DEFAULT, 0, &pSound);
		if (eRes == FMOD_OK)
		{
			int iLength = strlen(szFilename) + 1;

			TCHAR* pSoundKey = new TCHAR[iLength];
			ZeroMemory(pSoundKey, sizeof(TCHAR) * iLength);
			MultiByteToWideChar(CP_ACP, 0, szFilename, iLength, pSoundKey, iLength);

			m_mapSound.emplace(pSoundKey, pSound);
		}
		iResult = _tfindnext64(handle, &fd);
	}
	FMOD_System_Update(m_pSystem);
	_findclose(handle);
}

CSound_Manager* CSound_Manager::Create()
{
	CSound_Manager* pInstance = new CSound_Manager();

	if (FAILED(pInstance->Initialize()))
	{
		_ASSERT_EXPR(FALSE, TEXT("Failed To Create : CSound_Manager"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSound_Manager::Free()
{
	for (auto& Mypair : m_mapSound)
	{
		delete[] Mypair.first;
		FMOD_Sound_Release(Mypair.second);
	}
	m_mapSound.clear();

	FMOD_System_Release(m_pSystem);
	FMOD_System_Close(m_pSystem);
}
