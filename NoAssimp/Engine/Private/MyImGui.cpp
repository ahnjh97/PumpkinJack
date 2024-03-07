#include "GameInstance.h"
#include "MyImGui.h"
#include "ImGuizmo.h"

CMyImGui::CMyImGui(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :m_pDevice{ pDevice },
    m_pContext{ pContext },
    m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
    Safe_AddRef(m_pGameInstance);
}

void CMyImGui::Set_Vectors(vector<string> vecAnim, vector<string> vecNonAnim)
{
    for (auto animIter : vecAnim) // _strdup: c_str()가 포인터를 반환하는데, 이는 iter의 생명주기만큼만 보존돼서, 데이터를 아예 동적할당후 복사해둠
        vecAnimModels.push_back(_strdup(animIter.c_str())); 



    for (auto nonAnimIter : vecNonAnim)
    {
        _bool bSameName = false;
        for (auto animIter2 : vecAnim)
        {
            if (animIter2 == nonAnimIter)
                bSameName = true;
        }

        if(!bSameName)
            vecNonAnimModels.push_back(_strdup(nonAnimIter.c_str()));

    }        
}

HRESULT CMyImGui::Initialize(HWND hWnd)
{
    /* IMGUI 초기 세팅 */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(m_pDevice, m_pContext);

    return S_OK;
}

void CMyImGui::Tick(_float fTimeDelta)
{
    /* 실제 에디터에 대한 구성 */
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ///* IMGUI STYLE */
    //ImGuiStyle& style = ImGui::GetStyle();
    //style.FrameRounding = style.GrabRounding = 12.f;
    //style.FrameBorderSize = 1.f;
    //style.Colors[ImGuiCol_TabHovered] = ImVec4(1.f, 0.f, 0.f, 0.5f);
    //style.Colors[ImGuiCol_TabActive] = ImVec4(1.f, 0.f, 0.f, 1.0f);
    ///*========================================================================*/
}

HRESULT CMyImGui::Render()
{
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    return S_OK;
}

HRESULT CMyImGui::Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComponentTag, CComponent** ppOut, void* pArg)
{
    CComponent* pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
    if (nullptr == pComponent)
        return E_FAIL;

    auto	iter = m_Components.find(strComponentTag);
    if (iter != m_Components.end())
        return E_FAIL;

    m_Components.emplace(strComponentTag, pComponent);

    *ppOut = pComponent;

    Safe_AddRef(pComponent);

    return S_OK;
}

void CMyImGui::EditGuizmo(_float4x4& worldMatrix)
{
    ImGui::Begin("Guizmo Tool");
    ImGuizmo::BeginFrame();

    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
      //if (m_pGameInstance->KeyDown(DIK_T))
      //    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
      //if (m_pGameInstance->KeyDown(DIK_R))
      //    mCurrentGizmoOperation = ImGuizmo::ROTATE;
      //if (m_pGameInstance->KeyDown(DIK_V))
      //    mCurrentGizmoOperation = ImGuizmo::SCALE;

    if (ImGui::RadioButton("Position", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;

    ImGui::SameLine();

    if (ImGui::RadioButton("Rotation", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;

    ImGui::SameLine();

    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];

    ImGuizmo::DecomposeMatrixToComponents(&worldMatrix._11, matrixTranslation, matrixRotation, matrixScale);
    ImGui::InputFloat3("Position", matrixTranslation);
    ImGui::InputFloat3("Rotation", matrixRotation);
    ImGui::InputFloat3("Scale", matrixScale);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &worldMatrix._11);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;

        ImGui::SameLine();

        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
    }

    static bool useSnap(false);
    ImGui::Checkbox("Grab", &useSnap);

    ImGui::SameLine();

    _float3 snap = { 0.f, 0.f, 0.f };

    switch (mCurrentGizmoOperation)
    {
    case ImGuizmo::TRANSLATE:
        ImGui::InputFloat3("Position Grab", &snap.x);
        break;
    case ImGuizmo::ROTATE:
        ImGui::InputFloat("Rotation Grab", &snap.x);
        break;
    case ImGuizmo::SCALE:
        ImGui::InputFloat("Scale Grab", &snap.x);
        break;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    _float4x4	ProjMatrix, ViewMatrix;
    ViewMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW);
    ProjMatrix = m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ);

    ImGuizmo::Manipulate(&ViewMatrix._11, &ProjMatrix._11, mCurrentGizmoOperation, mCurrentGizmoMode, &worldMatrix._11, NULL, useSnap ? &snap.x : NULL);

    ImGui::End();
}

void CMyImGui::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);

   for (const char* animModel : vecAnimModels)
       free(const_cast<char*>(animModel)); // strdup으로 복사한 메모리를 free로 해제
   vecAnimModels.clear();

   for (const char* nonAnimModel : vecNonAnimModels)
       free(const_cast<char*>(nonAnimModel)); // strdup으로 복사한 메모리를 free로 해제 
   vecNonAnimModels.clear();

    for (auto& Pair : m_Components)
        Safe_Release(Pair.second);

    m_Components.clear();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}