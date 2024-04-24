#include "stdafx.h"
#include "ElecArena.h"
#include "Cell.h"

CElecArena::CElecArena(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CElecArena::CElecArena(const CElecArena& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CElecArena::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElecArena::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel), GameObjectDesc.wstrModelTag)))
		return E_FAIL;

	m_vecQuad = m_pModelCom->SetUp_QuadGroundCells();

	return S_OK;
}

void CElecArena::Tick(_float fTimeDelta)
{
	if (m_fTime > 4.f) {
		Set_Dead();
		return;
	}
		
	if (m_bWhiten && m_fColorOffset < 0.5f)
		m_fColorOffset += fTimeDelta * 0.25f;

	if (m_fColorOffset >= 0.5f)
		m_fTime += fTimeDelta;
}

void CElecArena::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_FIELD, this);
}

HRESULT CElecArena::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(6))) // ElecArena
			return E_FAIL;

		m_pModelCom->Render(i);
	}

	return S_OK;
}

_int CElecArena::Compute_Quadrant(_fvector vPlayerPos)
{
	if (nullptr == m_pTransformCom)
		return -1;

	_matrix matWorldInv = m_pTransformCom->Get_WorldMatrix_Inverse();
	_vector vPlayerElecArenaLocalPos = XMVector3TransformCoord(vPlayerPos, matWorldInv);
	CCell::QUAD eQuad{};

	if (XMVectorGetX(vPlayerElecArenaLocalPos) >= 0) {
		if (XMVectorGetZ(vPlayerElecArenaLocalPos) >= 0)
			eQuad = CCell::RT;
		else
			eQuad = CCell::RB;
	}
	else {
		if (XMVectorGetZ(vPlayerElecArenaLocalPos) <= 0)
			eQuad = CCell::LB;
		else
			eQuad = CCell::LT;
	}

	return eQuad;
}

_bool CElecArena::Check_ElecArenaGroundCell(_fvector vPlayerPos, _float& fCellHeight)
{
	_int iQuad = Compute_Quadrant(vPlayerPos);

	if (m_vecQuad[iQuad].empty())
		return false;

	if (nullptr == m_pTransformCom)
		return false;

	_matrix matWorld = m_pTransformCom->Get_WorldMatrix();
	_matrix matWorldInv = m_pTransformCom->Get_WorldMatrix_Inverse();

	_vector vNewPos = XMVector3TransformCoord(vPlayerPos, matWorldInv);
	for (auto& groundCell : m_vecQuad[iQuad]) {
		if (true == groundCell->IsIn(vNewPos)) {
			fCellHeight = XMVectorGetY(XMVector3TransformCoord(groundCell->Get_Point(CCell::POINT_A), matWorld));
			return true;
		}
	}

	return false;
}

_bool CElecArena::Check_QuadTreeCells(_fvector vPlayerPos, _float& fCellHeight)
{
	return m_pModelCom->Check_QuadTreeCell(vPlayerPos, m_pTransformCom->Get_WorldMatrix(), fCellHeight);
}

HRESULT CElecArena::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");

	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CElecArena::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldFloat4x4_Ptr())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fColorOffset", &m_fColorOffset, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CElecArena* CElecArena::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElecArena* pInstance = new CElecArena(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CElecArena"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CElecArena::Clone(void* pArg)
{
	CElecArena* pInstance = new CElecArena(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CElecArena"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CElecArena::Free()
{
	__super::Free();
	
	for (auto& vecCells : m_vecQuad) {
		for (auto& groundCell : vecCells) {
			Safe_Release(groundCell);
		}
	}
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
