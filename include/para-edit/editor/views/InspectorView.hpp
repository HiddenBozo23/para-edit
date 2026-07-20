#pragma once

#include <functional>
#include <string>
#include <typeindex>
#include <utility>

#include "para-edit/ecs/Scene.hpp"
#include "para-edit/editor/CommandManager.hpp"
#include "para-edit/editor/View.hpp"
#include "para-edit/editor/commands.hpp"

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
                             std::function<void(Entity entity, Scene& scene, CommandManager& cm)> draw) {
        m_ComponentInspector inspector;
        inspector.displayName = displayName;

        inspector.Has = [](Entity entity, Scene& scene) {
            return scene.GetComponent<T>(entity) != nullptr;
        };

        inspector.Draw = draw;

        inspector.Add = [](Entity entity, Scene& scene, CommandManager& cm) {
            cm.Execute<AddComponentCommand<T>>(scene);
        };

        inspector.Remove = [](Entity, Scene& scene, CommandManager& cm) {
            cm.Execute<RemoveComponentCommand<T>>(scene);
        };

        m_componentInspectorPairs.emplace_back(typeid(T), inspector);
    }

    std::vector<std::pair<std::type_index, m_ComponentInspector>> m_componentInspectorPairs;
};
}  // namespace para