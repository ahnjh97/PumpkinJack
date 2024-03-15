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
	for (auto& pChannel : m_Channels)
		Safe_AddRef(pChannel);
}

HRESULT CAnimation::Initialize(const aiAnimation* pAIAnimation, const vector<CBone*>& Bones)
{
	strcpy_s(m_szName, pAIAnimation->mName.data);

	m_fDuration = pAIAnimation->mDuration;

	m_fTickPerSecond = pAIAnimation->mTicksPerSecond;



	/* 이 애니메이션은 몇개의 뼈를 컨트롤해야하는가? */
	m_iNumChannels = pAIAnimation->mNumChannels;

	m_CurrentKeyFrameIndices.resize(m_iNumChannels);

	for (size_t i = 0; i < m_iNumChannels; i++)
	{
		CChannel* pChannel = CChannel::Create(pAIAnimation->mChannels[i], Bones);
		if (nullptr == pChannel)
			return E_FAIL;

		m_Channels.push_back(pChannel);
	}

	return S_OK;
}

void CAnimation::Update_TransformationMatrix(_float fTimeDelta, const vector<CBone*>& Bones, _bool isLoop)
{
	m_isFinished = false;

	m_fTrackPosition += m_fTickPerSecond * fTimeDelta;

	if (m_fDuration <= m_fTrackPosition)
	{
		if (false == isLoop)
		{
			m_isFinished = true;
			return;
		}

		m_fTrackPosition = 0.f;
	}

	for (_uint i = 0; i < m_iNumChannels; ++i)
	{
		/* 이 뼈의 상태행렬을 만들어서 CBone의 TransformationMatrix를 바꿔라. */
		m_Channels[i]->Invalidate_TransformationMatrix(Bones, m_fTrackPosition, &m_CurrentKeyFrameIndices[i]);
	}

}

void CAnimation::Write_AnimationData(ofstream& fileStream)
{
	//string strName = m_szName;
	//

	//size_t pos = strName.find_last_of('|');
	//if (pos != string::npos) {
	//	strName = strName.substr(pos + 1);
	//	{
	//		size_t lastUnderscore = strName.rfind('_');
	//		if (lastUnderscore != string::npos && lastUnderscore > 0) {
	//			// 뒤에서 두번째 UnderBar 및 뒷부분 삭제
	//			strName = strName.substr(0, lastUnderscore);
	//		}
	//	}
	//	strcpy_s(m_szName, strName.c_str());
	//}

	fileStream.write(reinterpret_cast<const char*>(&m_szName), sizeof(m_szName));
	fileStream.write(reinterpret_cast<const char*>(&m_fDuration), sizeof(m_fDuration));
	fileStream.write(reinterpret_cast<const char*>(&m_fTickPerSecond), sizeof(m_fTickPerSecond));
	fileStream.write(reinterpret_cast<const char*>(&m_iNumChannels), sizeof(m_iNumChannels));

	for (auto vecIter : m_Channels)
	{
		vecIter->Write_ChannelData(fileStream);
	}
}

CAnimation* CAnimation::Create(const aiAnimation* pAIAnimation, const vector<CBone*>& Bones)
{
	CAnimation* pInstance = new CAnimation();

	if (FAILED(pInstance->Initialize(pAIAnimation, Bones)))
	{
		MSG_BOX(TEXT("Failed To Created : CAnimation"));

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

