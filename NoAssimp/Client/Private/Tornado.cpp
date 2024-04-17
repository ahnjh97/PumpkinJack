#include "stdafx.h"
#include "Tornado.h"
#include "Wizard.h"

CTornado::CTornado(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CTornado::CTornado(const CTornado& rhs)
	: CGameObject{ rhs }
{

}

HRESULT CTornado::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTornado::Initialize(void* pArg)
{
	GAMEOBJECT_DESC GameObjectDesc = {}; // = (GAMEOBJECT_DESC*)pArg;
	if (pArg)
		GameObjectDesc = *(GAMEOBJECT_DESC*)pArg;

	if (GameObjectDesc.fSpeedPerSec == 1.f) // Levitation 용 아님
		m_bMove = true;

	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Add_Components(LEVEL(GameObjectDesc.iLevel))))
		return E_FAIL;

	_int iRandNum = m_pGameInstance->RandomNumber(0, 1);
	if (iRandNum == 0)
		m_iTurnDir = -1;

	return S_OK;
}

void CTornado::Tick(_float fTimeDelta)
{
	if (m_bDead == true)
		return;

	Get_PlayerPtr();
	m_fTime += fTimeDelta;
	m_fDamageTime += fTimeDelta;
	m_fTornadoSize += exp(m_fTime) * fTimeDelta;
	m_pTransformCom->Turn(XMVectorSet(0, 1, 0, 0), fTimeDelta * 7.f * m_iTurnDir);

	if (m_bMove) {
		if (m_fHoleWallColTime > 3.f) {
			Set_Dead();
			return;
		}
			
		_float3 vScale = m_pTransformCom->Get_Scaled();
		if (vScale.x < 15.f)
			EnlargeTornado(fTimeDelta);

		_float fHeight = m_pTransformCom->Get_Pos_Float4().y;
		if (fHeight >= 0)
			MoveTowardsPlayer(fTimeDelta);
		else if(false == m_bFall)
			m_pTransformCom->Fall(-fTimeDelta * 3.f);

		Check_TornadoCol(fTimeDelta);

		if (m_bHoleWallCol)
			m_fHoleWallColTime += fTimeDelta;
		else
			Check_HoleWallCol();

		if (false == m_bFall)
			Check_Fall();

		if (m_bFall) {
			if (m_pTransformCom->Get_Pos_Float4().y < -50.f)
				Set_Dead();
			else
				m_pTransformCom->Fall(fTimeDelta * 3.f);
		}
	}
	else { // LevitationTornado
		_float fHeight = m_pTransformCom->Get_Pos_Float4().y;
		if (fHeight < 0)
			m_pTransformCom->Fall(-fTimeDelta * 4.5f);
	}

	Check_PlayerCol();

	if(m_pColliderCom != nullptr)
		m_pColliderCom->Tick(m_pTransformCom->Get_WorldMatrix());
}

void CTornado::Late_Tick(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
}

HRESULT CTornado::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE)))
			return E_FAIL;

		/* 이 함수 내부에서 호출되는 Apply함수 호출 이전에 쉐이더 전역에 던져야할 모든 데이ㅏ터를 다 던져야한다. */
		if (FAILED(m_pShaderCom->Begin(7))) // Tornado
			return E_FAIL;

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pColliderCom != nullptr)
		m_pColliderCom->Render();
#endif

	return S_OK;
}

void CTornado::Get_PlayerPtr()
{
	if (m_pPlayer == nullptr) {
		m_pPlayer = m_pGameInstance->Get_Player(LEVEL_WIZARD);
		if (m_pPlayer != nullptr) {
			Safe_AddRef(m_pPlayer);
			CTransform* pPlayerTransform = dynamic_cast<CTransform*>(m_pPlayer->Get_Component(g_strTransformTag));
			m_pPlayerWorldMatrix = pPlayerTransform->Get_WorldFloat4x4_Ptr();
		}
	}
}

void CTornado::Check_PlayerCol()
{
	if (m_pPlayer == nullptr)
		return;

	CCollider* pPlayerCollider = dynamic_cast<CCollider*>(m_pPlayer->Get_Component(TEXT("Com_Collider")));
	if (m_pColliderCom->Intersect(pPlayerCollider)) {
		if (m_fDamageTime >= 1.f) {
			if(m_bMove)
				m_pPlayer->Deduct_HP(20);
			else
				m_pPlayer->Deduct_HP(10);

			m_fDamageTime = 0.f;
		}
	}
}

void CTornado::Check_TornadoCol(_float fTimeDelta)
{
	list<CGameObject*> tornadoList = m_pGameInstance->Get_ObjectList(LEVEL_WIZARD, TEXT("Layer_Tornado"));
	if (tornadoList.empty())
		return;

	_float3 vSrcPosFloat = m_pColliderCom->Get_Center(CCollider::TYPE_SPHERE);
	_vector vSrsPos = XMLoadFloat3(&vSrcPosFloat);

	for (auto& tornado : tornadoList)
	{
		if (tornado == this || nullptr == tornado)
			continue;

		CCollider* pTornadoCollider = dynamic_cast<CCollider*>(tornado->Get_Component(TEXT("Com_Collider")));
		if (nullptr == pTornadoCollider)
			continue;

		if (m_pColliderCom->Intersect(pTornadoCollider))
		{
			_float3 vDstPosFloat = pTornadoCollider->Get_Center(CCollider::TYPE_SPHERE);
			_vector vDstPos = XMLoadFloat3(&vDstPosFloat);

			_vector vDir = XMVector3Normalize(vSrsPos - vDstPos);

			m_pTransformCom->Move(fTimeDelta * 2.35f, vDir);
		}
	}
}

void CTornado::EnlargeTornado(_float fTimeDelta)
{
	_float fScale = m_fTornadoSize * 7;
	m_pTransformCom->Set_Scaled(fScale, fScale, fScale);
}

void CTornado::MoveTowardsPlayer(_float fTimeDelta)
{
	_float4 vRight{}, vLook;
	m_pTransformCom->Get_Look_LandObject(XMVectorSet(m_pPlayerWorldMatrix->_41, m_pPlayerWorldMatrix->_42, m_pPlayerWorldMatrix->_43, 1), vRight, vLook);
	m_pTransformCom->Move(fTimeDelta * 2.8f, XMLoadFloat4(&vLook));
}

void CTornado::Check_HoleWallCol()
{
	list<CGameObject*> holeWallList = m_pGameInstance->Get_ObjectList(LEVEL_WIZARD, TEXT("Layer_HoleWall"));
	if (holeWallList.empty())
		return;

	for (auto& holeWall : holeWallList)
	{
		if (nullptr == holeWall)
			continue;

		CCollider* pHoleWallCollider = dynamic_cast<CCollider*>(holeWall->Get_Component(TEXT("Com_Collider")));
		if (nullptr == pHoleWallCollider)
			continue;

		if (m_pColliderCom->Intersect(pHoleWallCollider))
			m_bHoleWallCol = true;
	}
}

void CTornado::Check_Fall()
{
	if(m_fTime > 11.f)
		m_bFall = true;
}

HRESULT CTornado::Add_Components(LEVEL eLevel)
{
	/* For.Com_Model */
	wstring wstrModelTag = TEXT("Prototype_Component_Model_Tornado");
	if (FAILED(__super::Add_Component(eLevel, wstrModelTag, TEXT("Com_Model"), (CComponent**)&m_pModelCom)))
		return E_FAIL;
	/* For.Com_Shader */
	wstring wstrShaderTag = TEXT("Prototype_Component_Shader_VtxModel");
	if (FAILED(__super::Add_Component(eLevel, wstrShaderTag, TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;
	/* For.Com_Texture_TigerStripe */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_TigerStripes"), 
		TEXT("Com_Texture_TigerStripe"), (CComponent**) & m_pTextureCom[TEX_TIGERSTRIPE])))
		return E_FAIL;
	/* For.Com_Texture_TopBottomFade */
	if (FAILED(__super::Add_Component(eLevel, TEXT("Prototype_Component_Texture_TopBottomFade"),
		TEXT("Com_Texture_TopBottomFade"), (CComponent**)&m_pTextureCom[TEX_TOPBOTTOMFADE])))
		return E_FAIL;
	/* For.Com_Collider */
	CBounding_Sphere::BOUNDING_SPHERE_DESC		ColliderDesc{};
	ColliderDesc.vCenter = _float3(0.f, 0.f, 0.f);
	ColliderDesc.fRadius = 1.1f;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Collider"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	return S_OK;
}

HRESULT CTornado::Bind_ShaderResources()
{
	if (nullptr == m_pShaderCom)
		return E_FAIL;

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_TIGERSTRIPE]->Bind_ShaderResource(m_pShaderCom, "g_TigerStripes")))
		return E_FAIL;
	if (FAILED(m_pTextureCom[TEX_TOPBOTTOMFADE]->Bind_ShaderResource(m_pShaderCom, "g_TopBottomFadeTex")))
		return E_FAIL;

	return S_OK;
}

CTornado* CTornado::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTornado* pInstance = new CTornado(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed To Create : CTornado"));

		Safe_Release(pInstance);
	}

	return pInstance;

}

CGameObject* CTornado::Clone(void* pArg)
{
	CTornado* pInstance = new CTornado(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed To Create : CTornado"));

		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTornado::Free()
{
	__super::Free();

	Safe_Release(m_pPlayer);
	Safe_Release(m_pColliderCom);
	for(_int i = 0; i < TEX_END; i++)
		Safe_Release(m_pTextureCom[i]);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
}
