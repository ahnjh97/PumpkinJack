#include "..\Public\GameInstance.h"

#include "Graphic_Device.h"
#include "Input_Device.h"
#include "Object_Manager.h"
#include "Level_Manager.h"
#include "Timer_Manager.h"
#include "ImGui_Manager.h"
#include "Light_Manager.h"
#include "Picking.h"
#include "Renderer.h"
#include <random>

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()	
{

}

HRESULT CGameInstance::Initialize_Engine(HINSTANCE hInstance, _uint iNumLevels, const ENGINE_DESC& EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext)
{
	/* 그래픽 디바이스를 초기화한다 .*/
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc, ppDevice, ppContext);
	if(nullptr == m_pGraphic_Device)
		return E_FAIL;	

	m_pInput_Device = CInput_Device::Create(hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	/*m_pFont_Manager = CFont_Manager::Create(*ppGraphic_Device);
	if (nullptr == m_pFont_Manager)
		return E_FAIL;
*/
	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDevice, *ppContext);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	

	/* 인풋 디바이스를 초기화한다 .*/

	/* 사운드 디바이스를 초기화한다 .*/

	/* 오브젝트 매니져의 공간 예약을 한다. */
	m_pObject_Manager = CObject_Manager::Create(iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	/* 컴포넌트 매니져의 공간 예약을 한다. */
	m_pComponent_Manager = CComponent_Manager::Create(iNumLevels);
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	m_pPicking = CPicking::Create(*ppDevice, *ppContext, EngineDesc.hWnd, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY);
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	Set_BlendState(*ppDevice);
	return S_OK;
}

HRESULT CGameInstance::Initialize_ImGui(_uint iLevelIndex, CMyImGui* pImGui)
{
	m_pImGui_Manager = CImGui_Manager::Create(pImGui);

	if (nullptr == m_pImGui_Manager)
		return E_FAIL;

	if (iLevelIndex != Level_MapTool)
		return S_OK;

	// 컴포넌트 이름 imgui로 가져오기
	vector<string> vecAnim;
	vector<string> vecNonAnim;
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	m_pComponent_Manager->PushBackPrototypes(vecAnim, vecNonAnim);
	m_pImGui_Manager->Set_Vectors(vecAnim, vecNonAnim);

	return S_OK;
}

void CGameInstance::Tick_Engine(_float fTimeDelta)
{
	if (nullptr == m_pLevel_Manager || 
		nullptr == m_pObject_Manager || 
		nullptr == m_pPipeLine)
		return;

	m_pInput_Device->Tick();

	m_pObject_Manager->Tick(fTimeDelta);	
	
	m_pPicking->Update();
	m_pPipeLine->Tick();


	m_pObject_Manager->Late_Tick(fTimeDelta);
	
	/* 반복적인 갱신이 필요한 객체들의 Tick함수를 호출한다. */
	m_pLevel_Manager->Tick(fTimeDelta);

	if ((Get_CurrentLevelID() != Level_MapTool) && (Get_DIKeyState(DIK_ESCAPE) & 0x80))
		PostQuitMessage(0);

	if (m_pImGui_Manager)
		m_pImGui_Manager->Tick(fTimeDelta);
	CalculateFPS(fTimeDelta);

}

HRESULT CGameInstance::Draw(const _float4& vClearColor)
{
	if (nullptr == m_pGraphic_Device || 
		nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pGraphic_Device->Clear_BackBuffer_View(vClearColor);
	m_pGraphic_Device->Clear_DepthStencil_View();

	/* 화면에 그려져야할 객체들을 그리낟. == 오브젝트 매니져에 들어가있을꺼야 .*/
	/* 오브젝트 매니져에 렌더함수를 만들어서 호출하면 객체들을 다 그린다. */

	/* But. CRenderer객체의 렌더함수를 호출하여 객체를 그리낟. */
	m_pRenderer->Render();

	m_pLevel_Manager->Render();	

	if (m_pImGui_Manager)
		m_pImGui_Manager->Render();

	m_pGraphic_Device->Present();

	return S_OK;
}

HRESULT CGameInstance::Clear(_uint iClearLevelIndex)
{
	if (nullptr == m_pObject_Manager ||
		nullptr == m_pComponent_Manager)
		return E_FAIL;

	/* 지정된 레벨용 자원(텍스쳐, 사운드, 객체등등) 을 삭제한다. */

	/* 사본 게임오브젝트. */
	m_pObject_Manager->Clear(iClearLevelIndex);

	/* 컴포넌트 원형 */
	m_pComponent_Manager->Clear(iClearLevelIndex);


	return S_OK;
}

_byte CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIKeyState(byKeyID);	
}

_byte CGameInstance::Get_DIMouseState(MOUSEKEYSTATE eMouse)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseState(eMouse);
}

_long CGameInstance::Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

_bool CGameInstance::KeyDown(_ubyte byKeyID)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->KeyDown(byKeyID);
}

_bool CGameInstance::KeyUp(_ubyte byKeyID)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->KeyUp(byKeyID);
}

_bool CGameInstance::MouseDown(_int button)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->MouseDown(button);
}

_bool CGameInstance::MouseUp(_int button)
{
	if (nullptr == m_pInput_Device)
		return 0;

	return m_pInput_Device->MouseUp(button);
}

HRESULT CGameInstance::Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroup, CGameObject * pRenderObject)
{
	if (nullptr == m_pRenderer)
		return E_FAIL;

	return m_pRenderer->Add_RenderGroup(eRenderGroup, pRenderObject);	
}

HRESULT CGameInstance::Open_Level(_uint iNewLevelID, CLevel * pNewLevel)
{
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	return m_pLevel_Manager->Open_Level(iNewLevelID, pNewLevel);
}

_uint CGameInstance::Get_CurrentLevelID()
{
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	return m_pLevel_Manager->Get_CurrentLevelID();
}

HRESULT CGameInstance::Add_Prototype(const wstring & strPrototypeTag, CGameObject * pPrototype)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Prototype(strPrototypeTag, pPrototype);	
}

HRESULT CGameInstance::Add_Clone(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	return m_pObject_Manager->Add_Clone(iLevelIndex, strLayerTag, strPrototypeTag, pArg);
}

CGameObject* CGameInstance::Clone_GameObject(const wstring& strPrototypeTag, void* pArg)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Clone_GameObject(strPrototypeTag, pArg);
}

const CComponent * CGameInstance::Get_Component(_uint iLevelIndex, const wstring & strLayerTag, const wstring & strComTag, _uint iIndex)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComTag, iIndex);
}

_bool CGameInstance::Binarize_Objects(_uint iLevel, string strLevel)
{
	if (nullptr == m_pObject_Manager)
		return false;

	return m_pObject_Manager->Binarize_Objects(iLevel, strLevel);
}

CGameObject* CGameInstance::Select_ModelByPicking(_uint iLevelIndex, const wstring& wstrLayerTag)
{
	if (nullptr == m_pObject_Manager)
		return false;

	return m_pObject_Manager->Select_ModelByPicking(iLevelIndex, wstrLayerTag);
}

CGameObject* CGameInstance::Get_GameObject(_uint iLevelIndex, const wstring& strLayerTag, const wstring& strPrototypeTag)
{
	if (nullptr == m_pObject_Manager)
		return false;

	return m_pObject_Manager->Get_GameObject(iLevelIndex, strLayerTag, strPrototypeTag);
}

CGameObject* CGameInstance::Get_LastGameObject(_uint iLevelIndex, const wstring& wstrLayerTag)
{
	if (nullptr == m_pObject_Manager)
		return false;

	return m_pObject_Manager->Get_LastGameObject(iLevelIndex, wstrLayerTag);
}

void CGameInstance::Clear_Layer(_uint iLevelIndex, const wstring& wstrLayerTag)
{
	if (nullptr == m_pObject_Manager)
		return;

	return m_pObject_Manager->Clear_Layer(iLevelIndex, wstrLayerTag);
}

HRESULT CGameInstance::Add_Prototype(_uint iLevelIndex, const wstring & strPrototypeTag, CComponent * pPrototype)
{
	if (nullptr == m_pComponent_Manager)
		return E_FAIL;

	return m_pComponent_Manager->Add_Prototype(iLevelIndex, strPrototypeTag, pPrototype);
}

CComponent * CGameInstance::Clone_Component(_uint iLevelIndex, const wstring & strPrototypeTag, void * pArg)
{
	if (nullptr == m_pComponent_Manager)
		return nullptr;

	return m_pComponent_Manager->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
}

CComponent* CGameInstance::Find_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag)
{
	if (nullptr == m_pComponent_Manager)
		return nullptr;

	return m_pComponent_Manager->Find_Prototype(iLevelIndex, strPrototypeTag);
}

HRESULT CGameInstance::Add_Timer(const wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	return m_pTimer_Manager->Add_Timer(strTimerTag);	
}

_float CGameInstance::Compute_TimeDelta(const wstring & strTimerTag)
{
	if (nullptr == m_pTimer_Manager)
		return 0.0f;

	return m_pTimer_Manager->Compute_TimeDelta(strTimerTag);
}

void CGameInstance::Set_Transform(CPipeLine::TRANSFORMSTATE eState, _fmatrix TransformMatrix)
{
	if (nullptr == m_pPipeLine)
		return;

	m_pPipeLine->Set_Transform(eState, TransformMatrix);
}

_matrix CGameInstance::Get_Transform_Matrix(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_Transform_Matrix(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_Transform_Float4x4(eState);
}

_matrix CGameInstance::Get_Transform_Matrix_Inverse(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return XMMatrixIdentity();

	return m_pPipeLine->Get_Transform_Matrix_Inverse(eState);
}

_float4x4 CGameInstance::Get_Transform_Float4x4_Inverse(CPipeLine::TRANSFORMSTATE eState) const
{
	if (nullptr == m_pPipeLine)
		return _float4x4();

	return m_pPipeLine->Get_Transform_Float4x4_Inverse(eState);
}

_vector CGameInstance::Get_CamPosition_Vector() const
{
	if (nullptr == m_pPipeLine)
		return XMVectorZero();

	return m_pPipeLine->Get_CamPosition_Vector();
}

_float4 CGameInstance::Get_CamPosition_Float4() const
{
	if (nullptr == m_pPipeLine)
		return _float4();

	return m_pPipeLine->Get_CamPosition_Float4();
}

void CGameInstance::Transform_PickingToLocalSpace(const CTransform* pTransform, _float3* pRayDir, _float3* pRayPos)
{
	m_pPicking->Transform_PickingToLocalSpace(pTransform, pRayDir, pRayPos);
}

_float3 CGameInstance::Get_PickPos()
{
	if(nullptr == m_pImGui_Manager)
		return _float3();

	return m_pImGui_Manager->Get_PickPos();
}

_bool CGameInstance::IsPicking()
{
	if (nullptr == m_pImGui_Manager)
		return _bool();

	return m_pImGui_Manager->IsPicking();
}

HRESULT CGameInstance::Release_ImGui()
{
	if (nullptr == m_pImGui_Manager)
		return E_FAIL;

	return m_pImGui_Manager->Release_ImGui();
}

const LIGHT_DESC* CGameInstance::Get_LightDesc(_uint iIndex)
{
	return m_pLight_Manager->Get_LightDesc(iIndex);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_Light(LightDesc);
}

string CGameInstance::WstrToStr(const wstring& wstr)
{
	wstring_convert<codecvt_utf8<wchar_t>> cnvt;
	return cnvt.to_bytes(wstr);
}

wstring CGameInstance::StrToWstr(const string& str)
{
	wstring_convert<codecvt_utf8<wchar_t>> cnvt;
	return cnvt.from_bytes(str);
}

void CGameInstance::CalculateFPS(_float fTimeDelta)
{
	iFrameCount++;
	fElapsedTime += fTimeDelta;

	if (fElapsedTime >= 1.0)  // fps
	{
		iFps = iFrameCount / fElapsedTime;

		iFrameCount = 0;
		fElapsedTime = 0.f;
	}
}

_int CGameInstance::RandomNumber(_int iMin, _int iMax)
{
	random_device rd;  
	mt19937 gen(rd()); 

	uniform_int_distribution<> dis(iMin, iMax);

	return dis(gen);
}

void CGameInstance::Set_BlendState(ID3D11Device* pDevice)
{
	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE; // 또는 TRUE, 필요에 따라 설정

	// 렌더 타겟에 대한 블렌드 설정 구조체 생성 및 초기화
	D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
	rtBlendDesc.BlendEnable = TRUE; // 블렌딩 활성화
	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA; // 소스 블렌드 팩터
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // 대상 블렌드 팩터
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD; // 블렌드 연산
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE; // 알파 소스 블렌드 팩터
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO; // 알파 대상 블렌드 팩터
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD; // 알파 블렌드 연산
	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // 색상 쓰기 마스크

	// 렌더 타겟에 블렌드 설정 적용
	blendDesc.RenderTarget[0] = rtBlendDesc; // 여러 렌더 타겟에 대한 설정이 필요한 경우 배열에 추가

	pDevice->CreateBlendState(&blendDesc, &m_pBlendState);
}


void CGameInstance::Release_Engine()
{
	CGameInstance::Get_Instance()->Free();

	Destroy_Instance();
}

void CGameInstance::Free()
{
	Safe_Release(m_pBlendState);
	Safe_Release(m_pLight_Manager);
	Safe_Release(m_pPicking);
	Safe_Release(m_pImGui_Manager);
	Safe_Release(m_pPipeLine);
	Safe_Release(m_pTimer_Manager);
	Safe_Release(m_pRenderer);	
	Safe_Release(m_pObject_Manager);
	Safe_Release(m_pComponent_Manager);
	Safe_Release(m_pLevel_Manager);
	Safe_Release(m_pInput_Device);
	Safe_Release(m_pGraphic_Device);
}
