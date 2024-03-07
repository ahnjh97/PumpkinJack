#pragma once
#include "Base.h"

BEGIN(Engine)

class CImGui_Manager : public CBase
{
private:
	CImGui_Manager();
	virtual ~CImGui_Manager() = default;

public:
	void Set_Vectors(vector<string> vecAnim, vector<string> vecNonAnim);
	_float3 Get_PickPos();
	_bool IsPicking();
public:
	HRESULT					Initiailize(class CMyImGui* pImGui);
	void					Tick(_float fTimeDelta);
	HRESULT					Render();

public:
	static CImGui_Manager* Create(class CMyImGui* pImGui);
	virtual void			Free();

private:
	class CMyImGui* m_pImGui = { nullptr };
};

END