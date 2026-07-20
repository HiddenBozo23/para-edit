#pragma once

#include "para-edit/core/Application.hpp"
#include "para-edit/ecs/Scene.hpp"
#include "para-edit/editor/CommandManager.hpp"
#include "para-edit/editor/ViewManager.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace para {
class Editor : public Application {
   public:
    Editor();

    void Run() override;

    void OnRender();

    Scene& GetScene();
    CommandManager& GetCommandManager();

    template <typename T, typename... Args>
    void RunCommand(Args&&... args) {
        m_commandManager.Execute<T>(std::forward<Args>(args)...);
    }

   private:
    void m_SetupDockspace(ImGuiID dockspaceId, ImVec2 size);
    bool m_firstFrame = true;

    Scene m_scene{};
    ViewManager m_viewManager{};
    CommandManager m_commandManager;
};
}  // namespace para