#pragma once
#include "Client_Defines.h"
#include "MyImGui.h"

BEGIN(Client)

class CImGui_Client : public CMyImGui
{
private:
	CImGui_Client(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CImGui_Client() = default;



public:
	HRESULT						Initialize();
	virtual void				Tick(_float fTimeDelta)		override;
	virtual HRESULT				Render()					override;

	void						OnRightClick();
	void						Edit_Object();

	void						See_Animation();

private:
	_bool						bCallNewFrame = { false };
	_bool						bSeeAnim = { false };
	_bool						bAnimWindow = { false };

	vector<string>				vecLevelName = {"Level_Static", "Level_Loading", "Level_Logo", "Level_MapTool", 
		"Level_GamePlay", "Level_GamePlay2","Level_GamePlay3", "Level_GamePlay4", "Level_End"};
	vector<CAnimation*>			vecAnimations;
	string						strCurLevel = {};
	string						strCurModel = {};

public:
	static CImGui_Client* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free();
};

END