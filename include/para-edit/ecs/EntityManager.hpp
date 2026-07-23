#pragma once

#include <array>
#include <bitset>
#include <vector>

#include "para-edit/ecs/types.hpp"

namespace para {

class EntityManager {
   public:
    Entity CreateEntity(Entity e = INVALID_ENTITY);
    void DestroyEntity(Entity e);

    void SetSignature(Entity e, Signature s);
    Signature GetSignature(Entity e);

    std::vector<Entity> GetAliveEntities() {
        std::vector<Entity> entities;
        for (Entity e = 0; e < MAX_ENTITIES; e++) {
            if (m_alive[e]) {
                entities.push_back(e);
            }
        }
        return entities;
    }

    bool GetEntityAliveValid(Entity e);

   private:
    Entity m_nextEntity{};
    std::vector<Entity> m_freeEntities{};

    std::bitset<MAX_ENTITIES> m_alive{};
    std::array<Signature, MAX_ENTITIES> m_signatures{};
};

}  // namespace para