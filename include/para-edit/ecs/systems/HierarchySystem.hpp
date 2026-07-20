#pragma once

#include "para-edit/ecs/Scene.hpp"
#include "para-edit/ecs/SystemManager.hpp"
#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {

class HierarchySystem : public System {
   public:
    explicit HierarchySystem(Scene& s) : System(s) {}

    void OnEntityRemoved(Entity e) override;

    void SetParent(Entity childE, Entity parentE);

    bool IsAncestor(Entity entity, Entity ancestor);

   private:
    bool m_cascadeDelete{};

    void m_UnlinkFromParent(Hierarchy* eH);
};

}  // namespace para