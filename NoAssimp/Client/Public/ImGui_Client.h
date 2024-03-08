#pragma once
#include "Client_Defines.h"
#include "MyImGui.h"
#include "Tool_Anim.h"
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

private:
	_bool						bCallNewFrame = { false };
	_bool						bSeeAnim = { false };
	_bool						bAnimWindow = { false };

	vector<string>				vecLevelName = {"Level_Static", "Level_Loading", "Level_Logo", "Level_MapTool", 
		"Level_GamePlay", "Level_GamePlay2","Level_GamePlay3", "Level_GamePlay4", "Level_End"};
	vector<CAnimation*>			vecAnimations;


	CModel*						pNonAnimModel = { nullptr };

	CTool_Anim*					pAnimObject = { nullptr };
	CModel*						pAnimModel = { nullptr };

public:
	static CImGui_Client* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void				Free();
};

END