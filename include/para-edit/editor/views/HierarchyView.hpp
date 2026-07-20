#pragma once

#include <string_view>

#include "para-edit/ecs/systems/HierarchySystem.hpp"
#include "para-edit/editor/View.hpp"

namespace para {
class HierarchyView : public View {
   public:
    HierarchyView(Editor& editor)
        : View(editor) {}
    void OnRender() override;

   private:
    void m_DrawEntity(Entity entity, Hierarchy* hc, Scene& scene, HierarchySystem* hs);
    // strings can't be constexprs so use string view
    static constexpr std::string_view m_dragDropKey = "ENTITY_DRAG";
};
}  // namespace para