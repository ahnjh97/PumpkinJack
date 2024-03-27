#pragma once
#include "Component.h"

BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct
	{
		_int			iCurrentIndex = { 0 };
	}NAVIGATION_DESC;
private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& rhs);
	virtual ~CNavigation() = default;

public:
	virtual HRESULT Initialize_Prototype(const wstring& strDataFile);
	virtual HRESULT Initialize(void* pArg) override;
	void Tick(_fmatrix WorldMatrix);
	_bool isMove(_fvector vPosition);

#ifdef _DEBUG
public:
	virtual HRESULT Render();
#endif

private:
	vector<class CCell*>				m_Cells;

	static _float4x4					m_WorldMatrix;

	/* 이 네비게이션을 이용하는 객체가 어떤 셀에 존재하는가?! */
	_int								m_iCurrentIndex = { -1 };

#ifdef _DEBUG
private:
	class CShader* m_pShader = { nullptr };
#endif

private:
	HRESULT SetUp_Neighbors();

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wstring& strDataFile);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END