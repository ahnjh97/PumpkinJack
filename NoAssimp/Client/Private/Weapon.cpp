#include "stdafx.h"
#include "Weapon.h"
#include "Bone.h"

CWeapon::CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CPart{ pDevice, pContext }
{
}

CWeapon::CWeapon(const CWeapon& rhs)
	: CPart{ rhs }
{
}

HRESULT CWeapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWeapon::Initialize(void* pArg)
{
	WEAPON_DESC* pWeaponDesc = (WEAPON_DESC*)pArg;

	m_pSocket = pWeaponDesc->pSocket;
	m_pState = pWeaponDesc->pState;
	m_pWeapon = pWeaponDesc->pWeapon;

	Safe_AddRef(m_pSocket);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Add_Components((LEVEL)pWeaponDesc->iLevel, pWeaponDesc->wstrModelTag)))
		return E_FAIL;

	
	if (m_pModelCom->Get_ModelName() == "Spear")
	{
		m_pTransformCom->Rotation(XMVectorSet(0, 0, 1, 0), -XM_PIDIV2 + 0.1f);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0, 0, 0, 1));
	}
		
	if (m_pModelCom->Get_ModelName() == "Rifle")
	{
		m_pTransformCom->Rotation(XMVectorSet(0, 0, 1, 0), -XM_PIDIV2);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(0, 0, -1, 1));
	}
		
	if (m_pModelCom->Get_ModelName() == "Scythe")
	{
		m_pTransformCom->Rotation(XMVectorSet(0, 0, 1, 0), -XM_PIDIV2);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, _float4(-3.5f, -0.25f, 0.3f, 1));
	}
		
		
	return S_OK;
}

void CWeapon::Tick(_float fTimeDelta)
{

}

void CWeapon::Late_Tick(_float fTimeDelta)
{
	_matrix matCombined = XMLoadFloat4x4(m_pSocket->Get_CombinedTransformationMatrix());
	_vector vScale, vRot, vTrans;
	XMMatrixDecompose(&vScale, &vRot, &vTrans, matCombined);
	
	_vector vNewScale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	_matrix matNew = XMMatrixAffineTransformation(vNewScale, XMVectorZero(), vRot, vTrans);

	_matrix matRot = XMMatrixRotationY(0.f);
	if (m_pModelCom->Get_ModelName() == "Spear")
	{
		if(*m_pState == IDLE || *m_pState == JUMP || *m_pState == ATTACK3)
			matRot = XMMatrixRotationZ(XMConvertToRadians(-3.f)) * XMMatrixRotationY(XMConvertToRadians(-10.f));
		
	}
	if (m_pModelCom->Get_ModelName() == "Scythe")
		matRot = XMMatrixRotationX(XMConvertToRadians(-65.f));
		
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * matRot * matNew * XMLoadFloat4x4(m_pParentMatrix));
	
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CWeapon::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_bool bCorrectWeapon = { false };
	if (m_pModelCom != nullptr)
	{
		if (*m_pWeapon == WPN_SPEAR && m_pModelCom->Get_ModelName() == "Spear")
			bCorrectWeapon = true;
		if (*m_pWeapon == WPN_SCYTHE && m_pModelCom->Get_ModelName() == "Scythe")
			bCorrectWeapon = true;
		if (*m_pWeapon == WPN_RIFLE && m_pModelCom->Get_ModelName() == "Rifle")
			bCorrectWeapon = true;
	}


	if (bCorrectWeapon)
	{
		_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

		for (size_t i = 0; i < iNumMeshes; i++)
		{
			if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", i, aiTextureType_DIFFUSE)))
				return E_FAIL;

			/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
			if (FAILED(m_pShaderCom->Begin(0)))
				return E_FAIL;

			m_pModelCom->Render(i);
		}
	}

	return S_OK;
}

HRESULT CWeapon::Add_Components(LEVEL eLevel, const wstring& _wstrModelTag)
{
	/* For.Com_Model */
	wstring wstrModelTag = _wstrModelTag;
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Shader_VtxModel"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CWeapon::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	return S_OK;
}

CWeapon* CWeapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWeapon* pInstance = new CWeapon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CWeapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CWeapon::Clone(void* pArg)
{
	CWeapon* pInstance = new CWeapon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CWeapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CWeapon::Free()
{
	__super::Free();

	Safe_Release(m_pSocket);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
}
