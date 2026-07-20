#include "para-edit/editor/Editor.hpp"

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/systems/HierarchySystem.hpp"
#include "para-edit/ecs/types.hpp"
#include "para-edit/editor/views/HierarchyView.hpp"
#include "para-edit/editor/views/InspectorView.hpp"
#include "para-edit/editor/views/LogView.hpp"

#include <imgui.h>
#include <imgui_internal.h>

namespace para {
Editor::Editor()
    : m_commandManager(m_scene) {
    // setup imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;  // delete later to remember user dock layout
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(m_window->GetHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // add views to editor
    m_viewManager.AddView<HierarchyView>(*this);
    m_viewManager.AddView<LogView>(*this);
    m_viewManager.AddView<InspectorView>(*this);

    // delete this later
    m_scene.RegisterSystem<HierarchySystem>(m_scene);
    m_scene.RegisterComponent<Hierarchy>();

    Signature signature;
    signature.set(m_scene.GetComponentIndex<Hierarchy>());
    m_scene.SetSystemSignature<HierarchySystem>(signature);

    Entity e1 = m_scene.CreateEntity();
    Entity e2 = m_scene.CreateEntity();

    m_scene.AddComponent<Hierarchy>(e1, {});
    m_scene.AddComponent<Hierarchy>(e2, {});

    LOG_DEBUG("TEST");
    LOG_INFO("TEST");
    LOG_WARNING("WARNING");
    LOG_ERROR("WRROR");
    LOG_FATAL("FATALIS");
};

void Editor::Run() {
    float lastTime = 0;
    float deltaTime = 0;

    while (!m_window->ShouldClose()) {
        m_window->Poll();

        deltaTime = m_window->GetTime() - lastTime;
        lastTime = m_window->GetTime();

        // start new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        OnRender();

        ImGui::Render();

        m_graphicsDevice->Clear();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window->SwapBuffer();
    }
}

void Editor::OnRender() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags hostWindowFlags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpaceHost", nullptr, hostWindowFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                     ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar);

    if (m_firstFrame) {
        m_firstFrame = false;
        m_SetupDockspace(dockspace_id, viewport->Size);
    }

    m_viewManager.RenderAll();

    ImGui::End();
}

Scene& Editor::GetScene() {
    return m_scene;
}

CommandManager& Editor::GetCommandManager() {
    return m_commandManager;
}

void Editor::m_SetupDockspace(ImGuiID dockspaceId, ImVec2 size) {
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, size);

    ImGuiID dockLeft, dockRight, dockBottom, dockCenter;

    dockLeft = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.15f, nullptr, &dockspaceId);
    dockRight = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.2f, nullptr, &dockspaceId);
    dockBottom = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Down, 0.2f, nullptr, &dockspaceId);
    dockCenter = dockspaceId;

    ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
    ImGui::DockBuilderDockWindow("Log", dockBottom);
    ImGui::DockBuilderDockWindow("Inspector", dockRight);

    ImGui::DockBuilderFinish(dockspaceId);
}
}  // namespace para