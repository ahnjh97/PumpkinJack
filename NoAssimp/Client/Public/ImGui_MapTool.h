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
	void				On_DIK_Escape();
	void				On_DIK_Delete();
	void				OptionChange();
	// ImGui
	_bool				Menu_Level();
	void				Menu_Anim();
	void				Menu_NonAnim();
	void				Menu_Animation();
	void				Edit_Object();
	void				Menu_Mode();
	// File I/O
	void				Save_Level();
	void				Load_Level();

	void				Save_Cells();
	void				Load_Cells();

	CCell*				Pick_Cell();
	void				Delete_Cells();
	void				Add_Cells();

private:
	_bool						bCallNewFrame = { false };
	_bool						bSeeAnim = { false };
	_bool						bAnimWindow = { false };
	_bool						bIsLevelOpen = { false };
	_bool						bNavigationMode = { false };

	_int						m_iOption = { };
	_int						m_iMode = {};

	vector<string>				vecLevelName = {"Level_Static", "Level_Loading", "Level_Logo", "Level_MapTool", 
		"Level_GamePlay", "Level_GamePlay2","Level_GamePlay3", "Level_SlimeBoss", "Level_End"};
	vector<CAnimation*>			vecAnimations;
	vector<string>				vecMode = { "Objects", "Navigation", "None" };

	vector<_uint>				vecCellOptions;
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