#pragma once

namespace Engine
{
	typedef struct
	{
		HWND			hWnd;
		bool			isWindowed;
		unsigned int	iWinSizeX, iWinSizeY;
	}ENGINE_DESC;

	typedef struct
	{
		class CTexture* MaterialTextures[AI_TEXTURE_TYPE_MAX];
	}MESH_MATERIAL;

	typedef struct
	{
		XMFLOAT3	vScale;
		XMFLOAT4	vRotation;
		XMFLOAT3	vTranslation;
		float		fTime;
	}KEYFRAME;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vTexcoord;

		static const unsigned int	iNumElements = { 2 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[2];
	}VTXPOSTEX;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		static const unsigned int	iNumElements = { 3 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[3];
	}VTXNORTEX;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		static const unsigned int	iNumElements = { 4 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[4];

	}VTXMESH;

	typedef struct ENGINE_DLL
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		/* 이 정점에게 영향을 주는 뼈들의 인덱스 */
		/* 뼈들의 인덱스 : 이 메시에게 영향을 주는 뼈들의 인덱스를 의미한다. */
		XMUINT4			vBlendIndices;
		XMFLOAT4		vBlendWeights;

		static const unsigned int	iNumElements = { 6 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[6];
	}VTXANIMMESH;

	typedef struct
	{
		_char	mName[MAX_PATH];
		_uint	mMaterialIndex;
		_uint	mNumVertices;
		_uint	mNumFaces;
		//VTXMESH* pVertices; // mNumVertices만큼 반복
		//_uint* pIndices; // mNumFaces * 3만큼 반복
	}MESH_NONANIM;

	typedef struct
	{
		_uint mVertexId;
		_float mWeight;
	}WEIGHT;

	typedef struct
	{
		_char	mName[MAX_PATH];
		_uint mNumWeights;
		//WEIGHT* mWeights; // mNumWeights만큼 반복
	}BONE;

	typedef struct
	{
		_char	mName[MAX_PATH];
		_uint	mMaterialIndex;
		_uint	mNumVertices;
		_uint	mNumFaces;
		//VTXANIMMESH* pVertices; // mNumVertices만큼 반복
		//_uint* pIndices; // mNumFaces * 3만큼 반복
		//_uint mNumBones;
		//BONE* mBones; // mNumBones만큼 반복
	}MESH_ANIM;

	//typedef struct
	//{
	//	string mKey;
	//	_uint mSemantic = 0;
	//	_uint mIndex = 0;
	//	_uint mDataLength = 0;
	//	_uint mType = 1;
	//	_char* mData = nullptr; // mDataLenght만큼 반복
	//}MATERIAL_PROPERTY;

	//typedef struct
	//{
	//	MATERIAL_PROPERTY* mProperties; // mNumProperties만큼 반복
	//	_uint mNumProperties;
	//	_uint mNumAllocated;
	//}MATERIAL;

	typedef struct
	{
		class CTexture* MaterialTextures[21];
	}MATERIAL_TEXTURE;

	typedef struct
	{
		_uint mNumMeshes; 
		//MESH_NONANIM* mMeshes; // mNumMeshes만큼 반복
		_uint mNumMaterials;
		//MATERIAL_TEXTURE* mMaterials; // mNumMaterials만큼 반복
	}MODEL_NONANIM;

	// MODEL_NONANIM.mNumMeshes 만큼 반복: MESH_NONANIM. { mName, mMaterialIndex, mNumVertices,	mNumFaces } -> VTXMESH * mNumVertices -> _uint * m_iNumIndices
	// MODEL_NONANIM.mNumMaterials * 20번 만큼 반복: _tchar szPerfectPath[MAX_PATH]
	// NoAssimp Client에서는 CMesh에 Get_MeshInfo함수 만들어서 mNumVertices, mNumFaces를 가져와서 파일읽기 시작위치 지정해줄것

	typedef struct
	{
		_char	mName[MAX_PATH]; // strcpy로 복사해서 사용
		_float4x4 mTransformation;
		_int mParentBoneIndex;
	}NODE;

	typedef struct
	{
		_uint mNumBones;
		//NODE* mBones; // mNumBones만큼 반복
		_uint mNumMeshes;
		//MESH_ANIM* mMeshes; // mNumMeshes만큼 반복
		_uint mNumMaterials;
		//MATERIAL_TEXTURE* mMaterials; // mNumMaterials만큼 반복
	}MODEL_ANIM;
}