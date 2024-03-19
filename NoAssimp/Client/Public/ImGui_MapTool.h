#pragma once
#include "Client_Defines.h"
#include "MyImGui.h"
#include "Tool_Anim.h"
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
	void				OnRightClick();
	void				OnLeftClick();
	void				On_DIK_Escape();
	void				On_DIK_Delete();
	// ImGui
	_bool				Menu_Level();
	void				Menu_Anim();
	void				Menu_NonAnim();
	void				Menu_Animation();
	void				Edit_Object();

	// File I/O
	void				Save_Level();
	void				Load_Level();

private:
	_bool						bCallNewFrame = { false };
	_bool						bSeeAnim = { false };
	_bool						bAnimWindow = { false };
	_bool						bIsLevelOpen = { false };
	vector<string>				vecLevelName = {"Level_Static", "Level_Loading", "Level_Logo", "Level_MapTool", 
		"Level_GamePlay", "Level_GamePlay2","Level_GamePlay3", "Level_SlimeBoss", "Level_End"};
	vector<CAnimation*>			vecAnimations;


	CModel*						pNonAnimModel = { nullptr };

	CTool_Anim*					pAnimObject = { nullptr };
	CModel*						pAnimModel = { nullptr };

public:
	static CImGui_MapTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free();
};

END