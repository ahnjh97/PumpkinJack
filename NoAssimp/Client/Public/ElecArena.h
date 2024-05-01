#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CModel;
class CShader;
class CCell;
END

BEGIN(Client)

class CElecArena final : public CGameObject
{
private:
	CElecArena(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CElecArena(const CElecArena& rhs);
	virtual ~CElecArena() = default;

public:
	void Set_Whiten() { m_bWhiten = true; }
	void Set_CirclePosMatrix(_float4x4 _matCirclePositions, _uint iMatIndex) { matCirclePositions[iMatIndex] = _matCirclePositions; 
			m_fCircleTime[iMatIndex] = 0; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	_bool Check_QuadTreeCells(_fvector vPlayerPos, _Out_ _float& fCellHeight);

private:
	_float4x4 matCirclePositions[2];
	_float	m_fCircleTime[2] = { 0, 0 };

	_bool m_bWhiten = { false };

	_float m_fColorOffset = {};
	_float m_fTime = {};

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };

	//vector<vector<CCell*>> m_vecQuad;

private:
	HRESULT Add_Components(LEVEL eLevel, const wstring& _wstrModelTag);
	HRESULT Bind_ShaderResources();

public:
	static CElecArena* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END