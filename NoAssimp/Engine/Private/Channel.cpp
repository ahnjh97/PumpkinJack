#include "Channel.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	fileStream.read(reinterpret_cast<char*>(&m_iNumKeyFrames), sizeof(m_iNumKeyFrames));

	auto	iter = find_if(Bones.begin(), Bones.end(), [&](CBone* pBone)->_bool
		{
			++m_iBoneIndex;
			return pBone->Compare_Name(m_szName);
		});

	m_KeyFrames.reserve(m_iNumKeyFrames);
	
	for (_uint i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME tempKeyFrame = {};
		fileStream.read(reinterpret_cast<char*>(&tempKeyFrame), sizeof(tempKeyFrame));
		m_KeyFrames.push_back(tempKeyFrame);
	}

	return S_OK;
}

void CChannel::Update_TransformationMatrix(const vector<class CBone*>& Bones, _float fTrackPosition, _uint* pCurrentKeyFrameIndex)
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

CChannel* CChannel::Create(const vector<class CBone*>& Bones, ifstream& fileStream)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(Bones, fileStream)))
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
