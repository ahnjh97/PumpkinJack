#pragma once
#include "Client_Defines.h"
#include "MyImGui.h"
#include "Tool_Anim.h"

BEGIN(Engine)
class CCell;
class CShader;
END;

BEGIN(Client)

class CImGui_MapTool : public CMyImGui
{
private:
	CImGui_MapTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CImGui_MapTool() = default;

public:
	HRESULT				Initialize();
	virtual void		Tick(_float fTimeDelta)		override;
	virtual HRESULT		Render()					override;

	// On Key/Mouse Input
	void				OnRightClick_ObjectMode();
	void				OnLeftClick_ObjectMode();
	void				OnLeftClick_NavigationMode();
	void				On_DIK_Escape();
	void				On_DIK_Delete();
	void				OptionChange();
	// ImGui
	_bool				Menu_Level();
	void				Menu_Anim();
	void				Menu_NonAnim();
	void				Menu_Animation();
	void				Edit_Object();
	void				Menu_Navigation();
	// File I/O
	void				Save_Level();
	void				Load_Level();

	// Cells
	void				Save_Cells();
	void				Load_Cells();
	CCell*				Pick_Cell(_Out_ _int& iIndex);
	void				Delete_Cells();
	void				Add_Cells();
	_bool				Check_ExistingCell(_float3 fA, _float3 fB, _float3 fC);

private:
	_bool						bCallNewFrame = { false };
	_bool						bSeeAnim = { false };
	_bool						bAnimWindow = { false };
	_bool						bIsLevelOpen = { false };
	_bool						bNavigationMode = { false };

	_int						m_iOption = { 1 };
	_int						m_iMode = {};

	_float4						m_vLeftClickPos = {};
	
	vector<string>				vecLevelName = {"Level_Static", "Level_Loading", "Level_Logo", "Level_MapTool", 
		"Level_GamePlay", "Level_GamePlay2","Level_GamePlay3", "Level_SlimeBoss", "Level_End"};
	vector<CAnimation*>			vecAnimations;
	vector<string>				vecMode = { "Objects", "Navigation", "None" };

	vector<CCell*>				vecCells;

	CModel*						pNonAnimModel = { nullptr };

	CTool_Anim*					pAnimObject = { nullptr };
	CModel*						pAnimModel = { nullptr };

private:
	CShader*					m_pShader = { nullptr };

public:
	static CImGui_MapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free();
};

END