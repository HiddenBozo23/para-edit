#pragma once

#include <any>

#include "para-edit/ecs/Scene.hpp"
#include "para-edit/editor/CommandManager.hpp"
#include "para-edit/editor/View.hpp"
#include "para-edit/editor/commands.hpp"

#include <imgui.h>

namespace para {
class InspectorView : public View {
   public:
    InspectorView(Editor& editor);
    void OnRender() override;

   private:
    struct m_ComponentInspector {
        std::string displayName;
        std::function<bool(Entity, Scene&)> Has;
        std::function<void(Entity, Scene&, CommandManager&)> Draw;
        std::function<void(Entity, Scene&, CommandManager&)> Add;
        std::function<void(Entity, Scene&, CommandManager&)> Remove;
    };

    template <typename T>
    void m_RegisterInspector(const std::string& displayName,
                             std::function<void(Entity entity, T& component, Scene& scene, CommandManager& cm)> drawFn) {
        m_ComponentInspector inspector;
        inspector.displayName = displayName;

        inspector.Has = [](Entity entity, Scene& scene) {
            return scene.HasComponent<T>(entity);
        };

        inspector.Draw = [drawFn](Entity entity, Scene& scene, CommandManager& cm) {
            if (scene.HasComponent<T>(entity)) {
                drawFn(entity, *scene.GetComponent<T>(entity), scene, cm);
            }
        };

        inspector.Add = [](Entity entity, Scene& scene, CommandManager& cm) {
            cm.Execute<AddComponentCommand<T>>(entity);
        };

        inspector.Remove = [](Entity entity, Scene& scene, CommandManager& cm) {
            cm.Execute<RemoveComponentCommand<T>>(entity);
        };

        m_componentInspectorPairs.emplace_back(typeid(T), inspector);
    }

    template <typename T>
    void m_HandleEditCommand(T& value, Entity entity, Scene& scene, CommandManager& cm) {
        if (ImGui::IsItemActivated()) {
            m_editStartValue = value;
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            cm.Execute<ValueUpdateCommand<T>>(entity, std::any_cast<T>(m_editStartValue), value);

            m_editStartValue.reset();
        }
    }

    std::vector<std::pair<std::type_index, m_ComponentInspector>> m_componentInspectorPairs;

    std::any m_editStartValue;
};
}  // namespace para