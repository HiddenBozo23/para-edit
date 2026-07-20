#pragma once

#include <array>
#include <vector>

#include "para-edit/ecs/types.hpp"

namespace para {

class EntityManager {
   public:
    Entity CreateEntity();
    void DestroyEntity(Entity e);

    void SetSignature(Entity e, Signature s);
    Signature GetSignature(Entity e);

    bool GetEntityAliveValid(Entity e);

   private:
    Entity m_nextEntity{};
    std::vector<Entity> m_freeEntities{};

    std::bitset<MAX_ENTITIES> m_alive{};
    std::array<Signature, MAX_ENTITIES> m_signatures{};
};

}  // namespace para