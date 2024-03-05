#include "Channel.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones)
{
	strcpy_s(m_szName, pAIChannel->mNodeName.data);

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
		{
			++m_iBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

	m_iNumKeyFrames = max(pAIChannel->mNumScalingKeys, pAIChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(m_iNumKeyFrames, pAIChannel->mNumPositionKeys);

	_float3			vScale;
	_float4			vRotation;
	_float3			vTranslation;
	_float			fTime;

	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME			KeyFrame{};

		if (i < pAIChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
			fTime = pAIChannel->mScalingKeys[i].mTime;
		}

		if (i < pAIChannel->mNumRotationKeys)
		{
			/*memcpy(&vRotation, &pAIChannel->mRotationKeys[i].mValue, sizeof(_float4));*/
			vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;

			fTime = pAIChannel->mRotationKeys[i].mTime;
		}

		if (i < pAIChannel->mNumPositionKeys)
		{
			memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
			fTime = pAIChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vTranslation = vTranslation;
		KeyFrame.fTime = fTime;

		m_KeyFrames.push_back(KeyFrame);
	}

	return S_OK;
}

void CChannel::Invalidate_TransformationMatrix(const vector<class CBone*>& Bones, _float fTrackPosition, _uint* pCurrentKeyFrameIndex)
{
	if (0.0f == fTrackPosition)
		(*pCurrentKeyFrameIndex) = 0;

	KEYFRAME		KeyFrame = m_KeyFrames.back();

	_float3			vScale;
	_float4			vRotation;
	_float3			vTranslation;

	if (KeyFrame.fTime <= fTrackPosition)
	{
		vScale = KeyFrame.vScale;
		vRotation = KeyFrame.vRotation;
		vTranslation = KeyFrame.vTranslation;
	}
	else
	{
		while (fTrackPosition >= m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].fTime)
			++(*pCurrentKeyFrameIndex);

		_float		fRatio = (fTrackPosition - m_KeyFrames[(*pCurrentKeyFrameIndex)].fTime)
			/ (m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].fTime - m_KeyFrames[(*pCurrentKeyFrameIndex)].fTime);

		XMStoreFloat3(&vScale, XMVectorLerp(XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vScale), XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vScale), fRatio));
		XMStoreFloat4(&vRotation, XMQuaternionSlerp(XMLoadFloat4(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vRotation), XMLoadFloat4(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vRotation), fRatio));
		XMStoreFloat3(&vTranslation, XMVectorLerp(XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex)].vTranslation), XMLoadFloat3(&m_KeyFrames[(*pCurrentKeyFrameIndex) + 1].vTranslation), fRatio));
	}


	_matrix		TransformationMatrix = XMMatrixAffineTransformation(XMLoadFloat3(&vScale), XMVectorSet(0.f, 0.f, 0.f, 1.f), XMLoadFloat4(&vRotation), XMVectorSetW(XMLoadFloat3(&vTranslation), 1.f));


	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
}

void CChannel::Write_ChannelData(ofstream& fileStream)
{
	fileStream.write(reinterpret_cast<const char*>(&m_szName), sizeof(m_szName));
	fileStream.write(reinterpret_cast<const char*>(&m_iNumKeyFrames), sizeof(m_iNumKeyFrames));

	for(auto keyframeIter : m_KeyFrames)
		fileStream.write(reinterpret_cast<const char*>(&keyframeIter), sizeof(keyframeIter));
}

CChannel* CChannel::Create(const aiNodeAnim* pAIChannel, const vector<class CBone*>& Bones)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pAIChannel, Bones)))
	{
		MSG_BOX(TEXT("Failed To Created : CChannel"));

		Safe_Release(pInstance);
	}

	return pInstance;
}


void CChannel::Free()
{
	m_KeyFrames.clear();
}
