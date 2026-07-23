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
    m_RegisterInspector<Name>("name", [this](Entity entity,
                                             Name& nc, Scene& scene, CommandManager& cm) {
        ImGui::PushID(entity);
        // ImGui::InputText("name", nc.name, sizeof(Name));
        ImGui::PopID();

        m_HandleEditCommand<Name>(nc, entity, scene, cm);
    });

    m_RegisterInspector<Transform>("transform", [this](Entity entity,
                                                       Transform& tc, Scene& scene, CommandManager& cm) {
        ImGui::DragFloat3("position", &tc.position.x, 0.1f);

        m_HandleEditCommand<Transform>(tc, entity, scene, cm);
    });
}

void InspectorView::OnRender() {
    ImGui::Begin("Inspector");

    Entity selected = m_editor.GetSelected();
    if (selected == INVALID_ENTITY) {
        ImGui::End();
        return;
    }

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