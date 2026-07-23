#pragma once

#include "para-edit/core/Application.hpp"
#include "para-edit/ecs/Scene.hpp"
#include "para-edit/ecs/types.hpp"
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
    Entity GetSelected();
    void SetSelected(Entity entity);

    template <typename T, typename... Args>
    void RunCommand(Args&&... args) {
        m_commandManager.Execute<T>(std::forward<Args>(args)...);
    }

    void AddEmptyEntity();

   private:
    struct m_Prefab {
        std::string name;
        std::function<Entity()> constructor;
    };

    void m_RenderMenuBar();

    void m_SetupDockspace(ImGuiID dockspaceId, ImVec2 size);

    Entity m_selected = INVALID_ENTITY;

    Scene m_scene{};
    ViewManager m_viewManager{};
    CommandManager m_commandManager;

    std::vector<m_Prefab> m_prefabs{};

    bool m_firstFrame = true;

    std::vector<std::string> m_hiddenComponents{};
};
}  // namespace para