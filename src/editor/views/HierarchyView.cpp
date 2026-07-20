#include "para-edit/editor/views/HierarchyView.hpp"

#include <string>

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/systems/HierarchySystem.hpp"
#include "para-edit/ecs/types.hpp"
#include "para-edit/editor/Editor.hpp"
#include "para-edit/editor/commands.hpp"
#include "para-edit/editor/theme.hpp"

#include <imgui.h>

namespace para {
void HierarchyView::OnRender() {
    auto& scene = m_editor.GetScene();
    auto* hs = scene.GetSystem<HierarchySystem>();

    if (!hs) {
        LOG_ERROR("HierarchyView: failed to get HierarchySystem from scene");
        return;
    }

    ImGui::Begin("Hierarchy", nullptr);

    // handle unparents
    ImGui::TextColored(theme::accentColor, "drop here to unparent");

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_dragDropKey.data())) {
            Entity droppedEntity = *(const Entity*)payload->Data;
            m_editor.RunCommand<ReparentCommand>(droppedEntity, INVALID_ENTITY);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::Separator();

    // draw entity tree
    auto& entities = hs->GetEntities();
    for (auto& entity : entities) {
        if (auto hc = scene.GetComponent<Hierarchy>(entity)) {
            if (hc->parent == INVALID_ENTITY) {
                m_DrawEntity(entity, hc, scene, hs);
            }
        }
    }

    ImGui::End();
}

void HierarchyView::m_DrawEntity(Entity entity, Hierarchy* hc, Scene& scene, HierarchySystem* hs) {
    std::string label = "entity " + std::to_string(entity);

    bool nodeOpen = ImGui::TreeNodeEx(label.c_str());

    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload(m_dragDropKey.data(), &entity, sizeof(Entity));
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_dragDropKey.data())) {
            Entity droppedEntity = *(const Entity*)payload->Data;
            if (droppedEntity != entity && !hs->IsAncestor(droppedEntity, entity)) {
                m_editor.RunCommand<ReparentCommand>(droppedEntity, entity);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (nodeOpen) {
        Entity child = hc->firstChild;
        Hierarchy* childH;
        while (child != INVALID_ENTITY) {
            childH = scene.GetComponent<Hierarchy>(child);
            m_DrawEntity(child, childH, scene, hs);
            child = childH->nextSibling;
        }
        ImGui::TreePop();
    }
}
}  // namespace para