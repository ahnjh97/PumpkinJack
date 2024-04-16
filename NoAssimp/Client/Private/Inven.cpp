#include "stdafx.h"
#include "Inven.h"
#include "UI_Weapon.h"
#include "Player.h"

CInven::CInven(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CInven::CInven(const CInven& rhs)
	: CGameObject{ rhs }
{
}

HRESULT CInven::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CInven::Initialize(void* pArg)
{
	INVEN_DESC		InvenDesc{};
	if (pArg)
		InvenDesc = *(INVEN_DESC*)pArg;

	InvenDesc.fSpeedPerSec = 10.f;
	InvenDesc.fRotationPerSec = XMConvertToRadians(120.0f);
	m_eWeapon = InvenDesc.eWeapon;

	if (FAILED(__super::Initialize(&InvenDesc)))
		return E_FAIL;

	m_fZ = InvenDesc.matWorld._43;
	if (FAILED(Add_Components(InvenDesc.eWeapon)))
		return E_FAIL;

	if (FAILED(Add_PartObjects((LEVEL)InvenDesc.iLevel)))
		return E_FAIL;

	m_fX = g_iWinSizeX * 0.5f;
	m_fY = g_iWinSizeY * 0.5f;

	m_fSizeX = m_fSizeY = 180;
	m_fX += 540;
	m_fY += 260;
	
	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(
		m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, m_fZ, 1.f ));

	::XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	::XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	_matrix matRotation = XMMatrixRotationAxis(XMVectorSet(0,0,1,0), XMConvertToRadians(-120.f));
	m_vInvenUp[WPN_SPEAR] = XMVectorSet(0, 1, 0, 0);
	m_vInvenUp[WPN_RIFLE] = XMVector3TransformNormal(m_vInvenUp[WPN_SPEAR], matRotation);
	m_vInvenUp[WPN_SCYTHE] = XMVector3TransformNormal(m_vInvenUp[WPN_RIFLE], matRotation);

	return S_OK;
}

void CInven::Tick(_float fTimeDelta)
{
	if (m_pPlayer == nullptr)
	{
		CGameObject* pObject = m_pGameInstance->Get_Player(*m_pGameInstance->Get_CurrentLevelID());
		if (pObject != nullptr)
		{
			m_pPlayer = dynamic_cast<CPlayer*>(pObject);
			Safe_AddRef(m_pPlayer);
		}
	}

	if (m_pPlayer == nullptr)
		return;

	if (true == m_pPlayer->Get_Dead())
		return;

	_vector vUp = m_pTransformCom->Get_State_Vector(CTransform::STATE_UP);
	_float fDegree = XMConvertToDegrees(XMVectorGetY(XMVector3AngleBetweenVectors(vUp, m_vInvenUp[m_pPlayer->Get_Weapon()])));
	if (fDegree > 4.f)
		m_pTransformCom->Turn(XMVectorSet(0, 0, 1, 0), -fTimeDelta * fDegree * 0.045f);
	for (auto& Pair : m_Parts)
		Pair.second->Tick(fTimeDelta);
}

void CInven::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this);
	for (auto& Pair : m_Parts)
		Pair.second->Late_Tick(fTimeDelta);
}

HRESULT CInven::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
	if (FAILED(m_pShaderCom->Begin(2)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

HRESULT CInven::Add_Components(WEAPON eWeapon)
{
	/* For.Com_Texture */
	wstring wstrTextureTag = TEXT("Prototype_Component_Texture_Inven");
	if (FAILED(__super::Add_Component(LEVEL_STATIC, wstrTextureTag,	TEXT("Com_Texture"), (CComponent**)&m_pTextureCom)))
		return E_FAIL;

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;
	
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	return S_OK;
}

HRESULT CInven::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", m_eWeapon)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fAlpha", &m_fAlpha, sizeof(m_fAlpha))))
		return E_FAIL;
	return S_OK;
}

HRESULT CInven::Add_PartObjects(LEVEL eLevel)
{
	CUI_Weapon::UI_WEAPON_DESC tUIWeaponDesc{};
	tUIWeaponDesc.pParentMatrix = m_pTransformCom->Get_WorldFloat4x4_Ptr();
	tUIWeaponDesc.matWorld._43 = tUIWeaponDesc.pParentMatrix->_43 - 0.3f;
	tUIWeaponDesc.eWeapon = m_eWeapon;
	if (FAILED(m_pGameInstance->Add_Clone(eLevel, TEXT("Layer_UI_Weapon"), TEXT("Prototype_GameObject_UI_Weapon"), &tUIWeaponDesc)))
		return E_FAIL;

	return S_OK;
}

CInven* CInven::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CInven* pInstance = new CInven(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CInven"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CInven::Clone(void* pArg)
{
	CInven* pInstance = new CInven(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CInven"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CInven::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);
	for (auto& Pair : m_Parts)
		Safe_Release(Pair.second);
	m_Parts.clear();
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);
}
