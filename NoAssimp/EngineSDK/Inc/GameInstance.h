#pragma once

/* 클라이언트개발자가 엔진의 기능을 사용하기위해서 항상 접근해야하는 클래스. */
#include "Renderer.h"
#include "Component_Manager.h"
#include "PipeLine.h"
#include <locale>
#include <codecvt>

BEGIN(Engine)

class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)
private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	HRESULT Initialize_Engine(HINSTANCE hInstance, _uint iNumLevels, const ENGINE_DESC& EngineDesc, _Inout_ ID3D11Device** ppDevice, _Inout_ ID3D11DeviceContext** ppContext);
	HRESULT	Initialize_ImGui(_uint iLevelIndex, class CMyImGui* pImGui);
	void Tick_Engine(_float fTimeDelta);
	HRESULT Draw(const _float4& vClearColor = _float4(0.f, 0.f, 1.f, 1.f));
	HRESULT Clear(_uint iClearLevelIndex);

public: /* For.Input_Device */
	_byte	Get_DIKeyState(_ubyte byKeyID);
	_byte	Get_DIMouseState(MOUSEKEYSTATE eMouse);
	_long	Get_DIMouseMove(MOUSEMOVESTATE eMouseState);
	_bool	KeyDown(_ubyte byKeyID);
	_bool	KeyUp(_ubyte byKeyID);
	_bool	MouseDown(_int button);
	_bool	MouseUp(_int button);

public: /* For.Renderer */
	HRESULT Add_RenderGroup(CRenderer::RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
#ifdef _DEBUG
public:
	HRESULT Add_DebugComponents(class CComponent* pRenderComponent);
#endif
public: /* For.Level_Manager */
	HRESULT Open_Level(_uint iNewLevelID, class CLevel* pNewLevel);
	_uint* Get_CurrentLevelID();

public: /* For.Object_Manager */
	HRESULT Add_Prototype(const wstring& wstrPrototypeTag, class CGameObject* pPrototype);
	HRESULT Add_Clone(_uint iLevelIndex, const wstring& wstrLayerTag, const wstring& wstrPrototypeTag, void* pArg = nullptr);
	class CGameObject* Clone_GameObject(const wstring& wstrPrototypeTag, void* pArg = nullptr);
	const CComponent* Get_Component(_uint iLevelIndex, const wstring& wstrLayerTag, const wstring& wstrComTag, _uint iIndex = 0);
	_bool Binarize_Objects(_uint iLevel, string strLevel);
	CGameObject* Select_ModelByPicking(_uint iLevelIndex, const wstring& wstrLayerTag); // MapTool에서의 모델선택
	CGameObject* Get_GameObject(_uint iLevelIndex, const wstring& wstrLayerTag, const wstring& wstrPrototypeTag);
	CGameObject* Get_LastGameObject(_uint iLevelIndex, const wstring& wstrLayerTag);
	list<CGameObject*>& Get_ObjectList(_uint iLevelIndex, const wstring& wstrLayerTag);
	void Clear_Layer(_uint iLevelIndex, const wstring& wstrLayerTag);
	CGameObject* Get_Player(_uint iLevelIndex);
	_bool Check_Collision(_uint iLevelIndex, const wstring& wstrLayerTag, class CCollider* pMyCollider, const wstring& wstrColliderTag = L"Com_Collider");
	_float* Get_CamFarPtr();

public: /* For.Component_Manager */
	HRESULT Add_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag, class CComponent* pPrototype);
	class CComponent* Clone_Component(_uint iLevelIndex, const wstring& wstrPrototypeTag, void* pArg = nullptr);
	class CComponent* Find_Prototype(_uint iLevelIndex, const wstring& wstrPrototypeTag);

public: /* For.Timer_Manager */
	HRESULT Add_Timer(const wstring& wstrTimerTag);
	_float Compute_TimeDelta(const wstring& wstrTimerTag);

public: /* For.PipeLine */
	void Set_Transform(CPipeLine::TRANSFORMSTATE eState, _fmatrix TransformMatrix);
	_matrix Get_Transform_Matrix(CPipeLine::TRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4(CPipeLine::TRANSFORMSTATE eState) const;
	_matrix Get_Transform_Matrix_Inverse(CPipeLine::TRANSFORMSTATE eState) const;
	_float4x4 Get_Transform_Float4x4_Inverse(CPipeLine::TRANSFORMSTATE eState) const;
	_vector Get_CamPosition_Vector() const;
	_float4 Get_CamPosition_Float4() const;

public: /* For. Picking */
	void Transform_PickingToLocalSpace(const class CTransform* pTransform, _Out_ _float3* pRayDir, _Out_ _float3* pRayPos);

public: /* For.ImGui_Manager */
	_float3 Get_PickPos();
	_bool IsPicking();
	HRESULT	Release_ImGui();
	void Hide_ImGui();
	void UnHide_ImGui();

public: /* For.Light_Manager */
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);

public: /* For.Target_Manager */
	HRESULT Add_RenderTarget(const wstring& strRenderTargetTag, _uint iSizeX, _uint iSizeY, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const wstring& strMRTTag, const wstring& strRenderTargetTag);
	HRESULT Begin_MRT(const wstring& strMRTTag);
	HRESULT End_MRT();
	HRESULT Bind_RTShaderResource(class CShader* pShader, const wstring& strRenderTargetTag, const _char* pConstantName);
	HRESULT Copy_Resource(const wstring& strRenderTargetTag, ID3D11Texture2D** ppTextureHub);

#ifdef _DEBUG
	HRESULT Ready_RTVDebug(const wstring& strRenderTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Draw_RTVDebug(const wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif

public: /* For.CoordConverter */
	_vector Compute_WorldPos(const _float2& vViewportPos, const wstring& strZRenderTargetTag, _uint iOffset = 0);
	_vector Compute_WorldPos_Float4(const _float4& vPlayerPos, const wstring& strZRenderTargetTag, _uint iOffset = 0);

	/* For.GameInstance */
	string WstrToStr(const wstring& wstr);
	wstring StrToWstr(const string& str);
	void CalculateFPS(_float fTimeDelta);
	_int Get_FPS() { return iFps; }
	_int RandomNumber(_int iMin, _int iMax);
	_bool Get_Toggle() { return m_bToggle; }
	void Toggle_Toggle() { m_bToggle = !m_bToggle; }
	_bool Get_NavigationMode() { return m_bNavigationMode; }
	void Toggle_NavigationMode() { m_bNavigationMode = !m_bNavigationMode; }

private:
	class CGraphic_Device*			m_pGraphic_Device = { nullptr };
	class CInput_Device*			m_pInput_Device = { nullptr };
	class CLevel_Manager*			m_pLevel_Manager = { nullptr };
	class CObject_Manager*			m_pObject_Manager = { nullptr };
	class CComponent_Manager*		m_pComponent_Manager = { nullptr };
	class CRenderer*				m_pRenderer = { nullptr };
	class CTimer_Manager*			m_pTimer_Manager = { nullptr };	
	class CPipeLine*				m_pPipeLine = { nullptr };
	class CImGui_Manager*			m_pImGui_Manager = { nullptr };
	class CPicking*					m_pPicking = { nullptr };
	class CLight_Manager*			m_pLight_Manager = { nullptr };
	class CRenderTarget_Manager*	m_pRenderTarget_Manager = { nullptr };
	class CCoordConverter*			m_pCoordConverter = { nullptr };

private:
	_uint iFrameCount = { 0 };
	_float fElapsedTime = { 0.f };
	_int iFps = { 0 };
	_bool m_bToggle = { true };
	_bool m_bNavigationMode = { true };

public:		
	static void Release_Engine();
	virtual void Free() override;
};

END