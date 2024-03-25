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
	CGameObject* Get_LastGameObject(_uint iLevelIndex, const wstring& wstrLayerTag); // MapTool용
	list<CGameObject*>& Get_ObjectList(_uint iLevelIndex, const wstring& wstrLayrTag);
	void Clear_Layer(_uint iLevelIndex, const wstring& wstrLayerTag);
	CGameObject* Get_Player(_uint iLevelIndex);

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

public: /* For.Light_Manager */
	const LIGHT_DESC* Get_LightDesc(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);

	/* For.GameInstance */
	string WstrToStr(const wstring& wstr);
	wstring StrToWstr(const string& str);
	void CalculateFPS(_float fTimeDelta);
	_int Get_FPS() { return iFps; }
	_int RandomNumber(_int iMin, _int iMax);
	void Set_BlendState(ID3D11Device* pDevice);
	void Set_DepthStencilStateEnabled(ID3D11Device* pDevice);
	void Set_DepthStencilStateDisabled(ID3D11Device* pDevice);
	ID3D11BlendState* Get_BlendState() { return m_pBlendState; }
	ID3D11DepthStencilState* Get_DepthStencilStateEnabled() { return m_pDepthStencilStateEnabled; }
	ID3D11DepthStencilState* Get_DepthStencilStateDisabled() { return m_pDepthStencilStateDisabled; }

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


private:
	_uint iFrameCount = { 0 };
	_float fElapsedTime = { 0.f };
	_int iFps = { 0 };
	ID3D11BlendState* m_pBlendState = { nullptr };
	ID3D11DepthStencilState* m_pDepthStencilStateEnabled = { nullptr };
	ID3D11DepthStencilState* m_pDepthStencilStateDisabled = { nullptr };
public:		
	static void Release_Engine();
	virtual void Free() override;
};

END