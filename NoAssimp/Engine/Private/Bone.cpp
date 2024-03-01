#include "Bone.h"



CBone::CBone()
{
}

HRESULT CBone::Initialize(_int  iParentIndex)
{
	m_iParentBoneIndex = iParentIndex;

	/*strcpy_s(m_szName, pAINode->mName.data);

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(_float4x4));
	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());*/

	return S_OK;
}

void CBone::Invalidate_CombinedTransformationMatrix(const vector<CBone*>& Bones)
{
	if (-1 == m_iParentBoneIndex)
		m_CombinedTransformationMatrix = m_TransformationMatrix;
	else
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}


}

CBone* CBone::Create( _int  iParentIndex)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(iParentIndex)))
	{
		MSG_BOX(TEXT("Failed To Created : CBone"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CBone::Free()
{

}
