#include "Bone.h"



CBone::CBone()
{
}

HRESULT CBone::Initialize(ifstream& fileStream)
{
	Read_BoneData(fileStream);

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix TransformatrixMatrix)
{
	if (-1 == m_iParentBoneIndex)
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * TransformatrixMatrix);
	else
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}
}

void CBone::Read_BoneData(ifstream& fileStream)
{
	fileStream.read(reinterpret_cast<char*>(&m_iParentBoneIndex), sizeof(m_iParentBoneIndex));
	fileStream.read(reinterpret_cast<char*>(&m_szName), sizeof(m_szName));
	fileStream.read(reinterpret_cast<char*>(&m_TransformationMatrix), sizeof(m_TransformationMatrix));
}

CBone* CBone::Create(ifstream& fileStream)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(fileStream)))
	{
		MSG_BOX(TEXT("Failed To Create : CBone"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CBone* CBone::Clone()
{
	return new CBone(*this);
}

void CBone::Free()
{

}
