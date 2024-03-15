#include "..\Public\Animation.h"
#include "Channel.h"
#include "Bone.h"

CAnimation::CAnimation()
{
}

CAnimation::CAnimation(const CAnimation& rhs)
	: m_fDuration{ rhs.m_fDuration }
	, m_fTickPerSecond{ rhs.m_fTickPerSecond }
	, m_fTrackPosition{ rhs.m_fTrackPosition }
	, m_iNumChannels{ rhs.m_iNumChannels }
	, m_Channels{ rhs.m_Channels }
	, m_CurrentKeyFrameIndices{ rhs.m_CurrentKeyFrameIndices }
	, m_isFinished{ rhs.m_isFinished }

{
	strcpy_s(m_szName, rhs.m_szName);
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);
}

HRESULT CAnimation::Initialize(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	Read_AnimationData(fileStream);
	m_CurrentKeyFrameIndices.resize(m_iNumChannels);
	
	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel* pChannel = CChannel::Create(Bones, fileStream);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}
	return S_OK;
}

void CAnimation::Update_TransformationMatrix(_float fTimeDelta, const vector<class CBone*>& Bones, _bool isLoop)
{
	m_isFinished = false;
	string strName = m_szName;
	if ((strName.find("attack") != string::npos) || (strName.find("Air") != string::npos) 
		|| (strName.find("Shoot") != string::npos) || (strName.find("Atk") != string::npos))
		fTimeDelta = fTimeDelta * 1.55f;

	m_fTrackPosition += m_fTickPerSecond * fTimeDelta;

	if (m_fDuration <= m_fTrackPosition)
	{
		if (false == isLoop)
		{
			m_isFinished = true;
			for (_uint i = 0; i < m_iNumChannels; ++i)
			{
				/* 이 뼈의 상태행렬을 만들어서 CBone의 TransformationMatrix를 바꿔라. */
				m_Channels[i]->Update_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
			}
			return;
		}
		m_fTrackPosition = 0.f;
	}

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		/* 이 뼈의 상태행렬을 만들어서 CBone의 TransformationMatrix를 바꿔라. */
		m_Channels[i]->Update_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
	}
}

void CAnimation::Reset(const vector<class CBone*>& Bones)
{
	m_fTrackPosition = 0.f;
	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		/* 이 뼈의 상태행렬을 만들어서 CBone의 TransformationMatrix를 바꿔라. */
		m_Channels[i]->Update_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
	}
}

void CAnimation::Read_AnimationData(ifstream& fileStream)
{
	fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	string strName = m_szName;
	size_t pos = strName.find('|');
	if (pos != string::npos)
	{
		strName = strName.substr(pos + 1);
		strcpy_s(m_szName, strName.c_str());
	}
		
	fileStream.read(reinterpret_cast<char*>(&m_fDuration), sizeof(m_fDuration));
	fileStream.read(reinterpret_cast<char*>(&m_fTickPerSecond), sizeof(m_fTickPerSecond));
	fileStream.read(reinterpret_cast<char*>(&m_iNumChannels), sizeof(m_iNumChannels));
}

CAnimation* CAnimation::Create(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(Bones, fileStream)))
	{
		MSG_BOX(TEXT("Failed To Create : CAnimation"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CAnimation* CAnimation::Clone()
{
	return new CAnimation(*this);
}


void CAnimation::Free()
{
	for (auto& pChannel : m_Channels)
		Safe_Release(pChannel);

	m_Channels.clear();
}

