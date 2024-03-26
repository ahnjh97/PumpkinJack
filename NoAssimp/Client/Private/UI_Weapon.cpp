#include "stdafx.h"
#include "UI_Weapon.h"
#include "Player.h"

CUI_Weapon::CUI_Weapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CPart{ pDevice, pContext }
{
}

CUI_Weapon::CUI_Weapon(const CUI_Weapon& rhs)
	: CPart{ rhs }
{
}

HRESULT CUI_Weapon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Weapon::Initialize(void* pArg)
{
	UI_WEAPON_DESC		UIWeaponDesc{};
	if (pArg)
		UIWeaponDesc = *(UI_WEAPON_DESC*)pArg;

	UIWeaponDesc.fSpeedPerSec = 10.f;
	UIWeaponDesc.fRotationPerSec = XMConvertToRadians(120.0f);
	m_eWeapon = UIWeaponDesc.eWeapon;

	if (FAILED(__super::Initialize(&UIWeaponDesc)))
		return E_FAIL;

	m_fZ = UIWeaponDesc.matWorld._43;
	if (FAILED(Add_Components(UIWeaponDesc.eWeapon)))
		return E_FAIL;

	m_fX = 0;
	m_fY = 0;

	m_fSizeX = m_fSizeY = 60;

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX , -m_fY, m_fZ, 1.f));

	::XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	::XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	/*_matrix matRotation = XMMatrixRotationAxis(XMVectorSet(0, 0, 1, 0), XMConvertToRadians(-120.f));
	m_vInvenUp[WPN_SPEAR] = XMVectorSet(0, 1, 0, 0);
	m_vInvenUp[WPN_RIFLE] = XMVector3TransformNormal(m_vInvenUp[WPN_SPEAR], matRotation);
	m_vInvenUp[WPN_SCYTHE] = XMVector3TransformNormal(m_vInvenUp[WPN_RIFLE], matRotation);*/

	return S_OK;
}

void CUI_Weapon::Tick(_float fTimeDelta)
{
	/*if (m_pPlayer == nullptr)
	{
		CGameObject* pObject = m_pGameInstance->Get_Player(*m_pGameInstance->Get_CurrentLevelID());
		if (pObject != nullptr)
		{
			m_pPlayer = dynamic_cast<CPlayer*>(pObject);
			Safe_AddRef(m_pPlayer);
		}
	}*/

	/*_vector vUp = m_pTransformCom->Get_State_Vector(CTransform::STATE_UP);
	_float fDegree = XMConvertToDegrees(XMVectorGetY(XMVector3AngleBetweenVectors(vUp, m_vInvenUp[m_pPlayer->Get_Weapon()])));
	if (fDegree > 4.f)
		m_pTransformCom->Turn(XMVectorSet(0, 0, 1, 0), -fTimeDelta * fDegree * 0.035f);*/
}

void CUI_Weapon::Late_Tick(_float fTimeDelta)
{
	XMStoreFloat4x4(&m_WorldMatrix, m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentMatrix));
	m_WorldMatrix._11 = m_fSizeX;
	m_WorldMatrix._22 = m_fSizeY;
	m_WorldMatrix._43 = m_fZ;
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
}

HRESULT CUI_Weapon::Render()
{
	_float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_pContext->OMSetBlendState(m_pGameInstance->Get_BlendState(), blendFactor, 0xffffffff);

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_Weapon::Add_Components(WEAPON eWeapon)
{
	/* For.Com_Texture*/
	wstring wstrTextureTag = {};
	if (eWeapon == WPN_SPEAR)
		wstrTextureTag = TEXT("Prototype_Component_Texture_Spear");
	else if (eWeapon == WPN_RIFLE)
		wstrTextureTag = TEXT("Prototype_Component_Texture_Rifle");
	else if (eWeapon == WPN_SCYTHE)
		wstrTextureTag = TEXT("Prototype_Component_Texture_Scythe");
	
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrTextureTag, TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex_Inven"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CUI_Weapon::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;
	
	return S_OK;
}



CUI_Weapon* CUI_Weapon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Weapon* pInstance = new CUI_Weapon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CUI_Weapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Weapon::Clone(void* pArg)
{
	CUI_Weapon* pInstance = new CUI_Weapon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CUI_Weapon"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Weapon::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
