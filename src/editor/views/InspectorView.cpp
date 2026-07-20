#include "para-edit/editor/views/InspectorView.hpp"

#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/types.hpp"
#include "para-edit/editor/CommandManager.hpp"
#include "para-edit/editor/Editor.hpp"

#include <imgui.h>

namespace para {
InspectorView::InspectorView(Editor& editor)
    : View(editor) {
    // register all component inspectors
    // assume component exists at time draw is called (check made in OnRender)
    m_RegisterInspector<Transform>("transform", [](Entity entity, Scene& scene, CommandManager& cm) {
        auto* tc = scene.GetComponent<Transform>(entity);
    });
}

void InspectorView::OnRender() {
    ImGui::Begin("Inspector");

    Entity selected = 0;
    Scene& scene = m_editor.GetScene();
    CommandManager& cm = m_editor.GetCommandManager();

    ImGui::BeginChild("InspectorScrollRegion", ImVec2(0, 0));

    ImGui::TextUnformatted(std::to_string(selected).c_str());

    for (auto& pair : m_componentInspectorPairs) {
        auto& inspector = pair.second;

        if (inspector.Has(selected, scene)) {
            bool open = true;
            if (ImGui::CollapsingHeader(inspector.displayName.c_str(),
                                        &open, ImGuiTreeNodeFlags_DefaultOpen)) {
                if (!open) {
                    inspector.Remove(selected, scene, cm);
                } else {
                    inspector.Draw(selected, scene, cm);
                }
            }
        }
    }

    ImGui::EndChild();

    ImGui::End();
}
}  // namespace para