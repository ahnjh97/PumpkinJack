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
		enum TYPE { TYPE_DIRECTIONAL, TYPE_POINT, TYPE_END };
		TYPE		eType;

		XMFLOAT4	vDirection;
		XMFLOAT4	vPosition;
		float		vRange;

		XMFLOAT4	vDiffuse;
		XMFLOAT4	vAmbient;
		XMFLOAT4	vSpecular;

	}LIGHT_DESC;

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

		static const unsigned int	iNumElements = { 1 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[1];
	}VTXPOS;

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
		XMFLOAT3		vTexcoord;

		static const unsigned int	iNumElements = { 2 };
		static const D3D11_INPUT_ELEMENT_DESC	Elements[2];
	}VTXCUBE;

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
		class CTexture* MaterialTextures[21];
	}MATERIAL_TEXTURE;
	
	typedef struct
	{
		_uint iLevel = {};
		wstring wstrPrototypeTag = L"";
		_float4x4 matWorld = _float4x4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		wstring wstrModelTag = L"";
	}PARSE_DATA;

	struct MODEL_CONFIG
	{
		_float fScale = { 1.f };
		_float fDegree = { 0.f };
		_float fNewScale = { 1.f };
		_uint iRealRoot = { 4 };
		MODEL_CONFIG(_float _fScale = { 1.f },
			_float _fDegree = { 0.f },
			_float _fNewScale = { 1.f },
			_uint _iRealRoot = { 4 }) : fScale(_fScale), fDegree(_fDegree), fNewScale(_fNewScale), iRealRoot(_iRealRoot) {}
	};

	// MODEL_NONANIM.mNumMeshes 만큼 반복: MESH_NONANIM. { mName, mMaterialIndex, mNumVertices,	mNumFaces } -> VTXMESH * mNumVertices -> _uint * m_iNumIndices
	// MODEL_NONANIM.mNumMaterials * 20번 만큼 반복: _tchar szPerfectPath[MAX_PATH]
	// NoAssimp Client에서는 CMesh에 Get_MeshInfo함수 만들어서 mNumVertices, mNumFaces를 가져와서 파일읽기 시작위치 지정해줄것
}