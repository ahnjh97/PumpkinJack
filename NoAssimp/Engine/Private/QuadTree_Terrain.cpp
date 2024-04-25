#include "QuadTree_Terrain.h"
#include "GameInstance.h"

CQuadTree_Terrain::CQuadTree_Terrain()
{

}

HRESULT CQuadTree_Terrain::Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	m_iCorners[CORNER_LT] = iLT;
	m_iCorners[CORNER_RT] = iRT;
	m_iCorners[CORNER_RB] = iRB;
	m_iCorners[CORNER_LB] = iLB;

	if (1 == m_iCorners[CORNER_RT] - m_iCorners[CORNER_LT])
		return S_OK;

	m_iCenter = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RB]) >> 1;

	_uint iLC, iTC, iRC, iBC;

	iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
	iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
	iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
	iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

	m_pChildren[CORNER_LT] = CQuadTree_Terrain::Create(m_iCorners[CORNER_LT], iTC, m_iCenter, iLC);
	m_pChildren[CORNER_RT] = CQuadTree_Terrain::Create(iTC, m_iCorners[CORNER_RT], iRC, m_iCenter);
	m_pChildren[CORNER_RB] = CQuadTree_Terrain::Create(m_iCenter, iRC, m_iCorners[CORNER_RB], iBC);
	m_pChildren[CORNER_LB] = CQuadTree_Terrain::Create(iLC, m_iCenter, iBC, m_iCorners[CORNER_LB]);



	return S_OK;
}

HRESULT CQuadTree_Terrain::SetUp_Neighbors()
{
	if (nullptr == m_pChildren[CORNER_LT]->m_pChildren[CORNER_LT])
		return S_OK;

	m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChildren[CORNER_RT];
	m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChildren[CORNER_LB];

	m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChildren[CORNER_LT];
	m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pChildren[CORNER_RB];

	m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pChildren[CORNER_LB];
	m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChildren[CORNER_RT];

	m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pChildren[CORNER_RB];
	m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_TOP] = m_pChildren[CORNER_LT];

	if (nullptr != m_pNeighbors[NEIGHBOR_RIGHT])
	{
		m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChildren[CORNER_LT];
		m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->m_pChildren[CORNER_LB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_BOTTOM])
	{
		m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChildren[CORNER_LT];
		m_pChildren[CORNER_RB]->m_pNeighbors[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->m_pChildren[CORNER_RT];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_LEFT])
	{
		m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChildren[CORNER_RT];
		m_pChildren[CORNER_LB]->m_pNeighbors[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->m_pChildren[CORNER_RB];
	}

	if (nullptr != m_pNeighbors[NEIGHBOR_TOP])
	{
		m_pChildren[CORNER_LT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChildren[CORNER_LB];
		m_pChildren[CORNER_RT]->m_pNeighbors[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->m_pChildren[CORNER_RB];
	}

	for (size_t i = 0; i < CORNER_END; i++)
	{
		m_pChildren[i]->SetUp_Neighbors();
	}

	return S_OK;
}

void CQuadTree_Terrain::Culling(CGameInstance* pGameInstance, const _float3* pVerticesPos, _uint* pIndices, _uint* pNumIndices)
{
	/* 더 이상 분열이 되지 않는다면 자식들에 대한 비교를 할 필요가 없다. */
	if (nullptr == m_pChildren[CORNER_LT] ||
		true == isDraw(pGameInstance, pVerticesPos))
	{
		_bool	isIn[4] = {
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]])),
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RT]])),
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RB]])),
			pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LB]])),
		};

		_bool	isDraw[NEIGHBOR_END] = { true, true,true,true };

		if (nullptr != m_pNeighbors[NEIGHBOR_LEFT])
			isDraw[NEIGHBOR_LEFT] = m_pNeighbors[NEIGHBOR_LEFT]->isDraw(pGameInstance, pVerticesPos);
		if (nullptr != m_pNeighbors[NEIGHBOR_TOP])
			isDraw[NEIGHBOR_TOP] = m_pNeighbors[NEIGHBOR_TOP]->isDraw(pGameInstance, pVerticesPos);
		if (nullptr != m_pNeighbors[NEIGHBOR_RIGHT])
			isDraw[NEIGHBOR_RIGHT] = m_pNeighbors[NEIGHBOR_RIGHT]->isDraw(pGameInstance, pVerticesPos);
		if (nullptr != m_pNeighbors[NEIGHBOR_BOTTOM])
			isDraw[NEIGHBOR_BOTTOM] = m_pNeighbors[NEIGHBOR_BOTTOM]->isDraw(pGameInstance, pVerticesPos);



		if (true == isDraw[NEIGHBOR_LEFT] &&
			true == isDraw[NEIGHBOR_TOP] &&
			true == isDraw[NEIGHBOR_RIGHT] &&
			true == isDraw[NEIGHBOR_BOTTOM])
		{
			/* 오르ㅡㄴ쪽 위 삼가형이 그려진다면. */
			if (true == isIn[0] ||
				true == isIn[1] ||
				true == isIn[2])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}

			/* 왼쪽 아래 삼가형이 그려진다면. */
			if (true == isIn[0] ||
				true == isIn[2] ||
				true == isIn[3])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}

			return;
		}

		_uint iLC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_LB]) >> 1;
		_uint iTC = (m_iCorners[CORNER_LT] + m_iCorners[CORNER_RT]) >> 1;
		_uint iRC = (m_iCorners[CORNER_RT] + m_iCorners[CORNER_RB]) >> 1;
		_uint iBC = (m_iCorners[CORNER_LB] + m_iCorners[CORNER_RB]) >> 1;

		if (true == isIn[0] ||
			true == isIn[2] ||
			true == isIn[3])
		{
			if (false == isDraw[NEIGHBOR_LEFT])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iLC;

				pIndices[(*pNumIndices)++] = iLC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
			}

			if (false == isDraw[NEIGHBOR_BOTTOM])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iBC;

				pIndices[(*pNumIndices)++] = iBC;
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LB];
				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
		}

		if (true == isIn[0] ||
			true == isIn[1] ||
			true == isIn[2])
		{
			if (false == isDraw[NEIGHBOR_TOP])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iTC;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_LT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}

			if (false == isDraw[NEIGHBOR_RIGHT])
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCenter;

				pIndices[(*pNumIndices)++] = m_iCenter;
				pIndices[(*pNumIndices)++] = iRC;
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
			}
			else
			{
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RT];
				pIndices[(*pNumIndices)++] = m_iCorners[CORNER_RB];
				pIndices[(*pNumIndices)++] = m_iCenter;
			}
		}

		return;
	}

	/* 현재 쿼트리와 절두체의 충돌비교하여 충돌되었다면.?? : 자식쿼드트리에 대한 비교를 반복적으로 수행하기 위해서. */

	_float		fRange = XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]]) - XMLoadFloat3(&pVerticesPos[m_iCenter])).m128_f32[0];

	// if (현재 쿼트리와 절두체의 충돌비교하여 충돌되었다면.)
	if (pGameInstance->isInFrustum_LocalSpace(XMLoadFloat3(&pVerticesPos[m_iCenter]), fRange))
	{
		for (size_t i = 0; i < CORNER_END; i++)
		{
			m_pChildren[i]->Culling(pGameInstance, pVerticesPos, pIndices, pNumIndices);
		}
	}
	else
	{
		int a = 10;
	}
}

_bool CQuadTree_Terrain::isDraw(CGameInstance* pGameInstance, const _float3* pVerticesPos)
{
	if (nullptr == pGameInstance)
		return false;

	Safe_AddRef(pGameInstance);
	_vector		vCamPos = pGameInstance->Get_CamPosition_Vector();
	Safe_Release(pGameInstance);
	_float		fCenterDistance = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCenter]) - vCamPos));

	_float		fWidth = XMVectorGetX(XMVector3Length(XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_RT]]) - XMLoadFloat3(&pVerticesPos[m_iCorners[CORNER_LT]])));

	if (fCenterDistance * 0.1f > fWidth)
		return true;

	return false;
}

CQuadTree_Terrain* CQuadTree_Terrain::Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB)
{
	CQuadTree_Terrain* pInstance = new CQuadTree_Terrain();

	if (FAILED(pInstance->Initialize(iLT, iRT, iRB, iLB)))
	{
		MSG_BOX(TEXT("Failed to Create : CQuadTree_Terrain"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQuadTree_Terrain::Free()
{
	__super::Free();

	if (m_pChildren[CORNER_RT] != nullptr) {
		for (_int i = 0; i < CORNER_END; i++)
			Safe_Release(m_pChildren[i]);
	}
}

