#include "para-edit/ecs/EntityManager.hpp"

#include <algorithm>

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {

Entity EntityManager::CreateEntity(Entity e) {
    // check that an entity can be created
    if (m_nextEntity == MAX_ENTITIES && m_freeEntities.empty()) {
        LOG_ERROR("EntityManager: failed to create entity. maximum entities ("
                  << MAX_ENTITIES << ") reached");
        return INVALID_ENTITY;
    }

    Entity id;

    if (e != INVALID_ENTITY) {
        if (m_alive[e]) {
            LOG_ERROR("EntityManager: failed to create entity ("
                      << e << "). already in use");
            return INVALID_ENTITY;
        }

        // remove e from m_freeEntities
        auto index = std::find(m_freeEntities.begin(), m_freeEntities.end(), e);
        if (index != m_freeEntities.end()) {
            m_freeEntities.erase(index);
        }

        id = e;
        m_alive[id] = true;

    } else {
        // try to reuse a freed entity (m_freeEntities), otherwise issue a new one
        if (!m_freeEntities.empty()) {
            id = m_freeEntities.back();
            m_freeEntities.pop_back();
        } else {
            do {
                id = m_nextEntity;
                m_nextEntity++;

                if (m_nextEntity == MAX_ENTITIES) {
                    LOG_ERROR("EntityManager: failed to create entity. maximum entities ("
                              << MAX_ENTITIES << ") reached");
                    return INVALID_ENTITY;
                }

            } while (m_alive[id]);

            m_alive[id] = true;
        }
    }

    return id;
}

void EntityManager::DestroyEntity(Entity e) {
    if (!GetEntityAliveValid(e)) {
        LOG_ERROR("EntityManager: failed to destroy invalid entity (" << e << ")");
        return;
    }

    m_signatures[e].reset();
    m_freeEntities.push_back(e);
    m_alive[e] = false;
}

void EntityManager::SetSignature(Entity e, Signature s) {
    if (!GetEntityAliveValid(e)) {
        LOG_ERROR("EntityManager: failed to set the signature of invalid entity ("
                  << e << ")");
        return;
    }

    m_signatures[e] = s;
}

Signature EntityManager::GetSignature(Entity e) {
    if (!GetEntityAliveValid(e)) {
        LOG_ERROR("EntityManager: failed to get the signature of invalid entity ("
                  << e << ")");
        return Signature{};
    }

    return m_signatures[e];
}

bool EntityManager::GetEntityAliveValid(Entity e) {
    if (e >= MAX_ENTITIES || e == INVALID_ENTITY) {
        return false;
    }

    return m_alive[e];
}

}  // namespace para