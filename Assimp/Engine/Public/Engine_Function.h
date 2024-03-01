#pragma once

namespace Engine
{
	template<typename T>
	void Safe_Delete(T& pPoint)
	{
		if (nullptr != pPoint)
		{
			delete pPoint;
			pPoint = nullptr;
		}	
	}

	template<typename T>
	void Safe_Delete_Array(T& pPoint)
	{
		if (nullptr != pPoint)
		{
			delete[] pPoint;
			pPoint = nullptr;
		}
	}


	template<typename T>
	unsigned int Safe_AddRef(T& pInstance)
	{
		unsigned int iRefCnt = 0;

		if (nullptr != pInstance)		
			iRefCnt = pInstance->AddRef();

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_Release(T& pInstance)
	{
		unsigned int iRefCnt = 0;

		if (nullptr != pInstance)
		{
			iRefCnt = pInstance->Release();

			if(0 == iRefCnt)
				pInstance = nullptr;
		}

		return iRefCnt;
	}
}

