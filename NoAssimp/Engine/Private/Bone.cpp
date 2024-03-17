#include "Bone.h"



CBone::CBone()
{
}

HRESULT CBone::Initialize(ifstream& fileStream)
{
	Read_BoneData(fileStream);

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_matNewCombined, XMMatrixIdentity());

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix TransformatrixMatrix, _int _realRootIndex)
{
	if (-1 == m_iParentBoneIndex)
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * TransformatrixMatrix);
	else if (m_iParentBoneIndex == (_realRootIndex - 1)) // 드디어 찾았다 루트노드
	{
		/*m_TransformationMatrix._41 = 0.f;
		m_TransformationMatrix._42 = 0.f;
		m_TransformationMatrix._43 = 0.f;*/
		
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}
	else
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}
}

void CBone::Update_NewCombinedMatrix(const vector<CBone*>& Bones, _fmatrix TransformatrixMatrix)
{
	if (-1 == m_iParentBoneIndex)
		XMStoreFloat4x4(&m_matNewCombined, XMLoadFloat4x4(&m_TransformationMatrix) * TransformatrixMatrix);
	else
	{
		XMStoreFloat4x4(&m_matNewCombined,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_matNewCombined));
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
