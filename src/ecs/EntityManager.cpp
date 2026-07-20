#include "para-edit/ecs/EntityManager.hpp"

#include "para-edit/core/Logger.hpp"

namespace para {

Entity EntityManager::CreateEntity() {
    // check that an entity can be created
    if (m_nextEntity == MAX_ENTITIES && m_freeEntities.empty()) {
        LOG_ERROR("EntityManager: failed to create entity. maximum entities ("
                  << MAX_ENTITIES << ") reached");
        return INVALID_ENTITY;
    }

    // try to reuse a freed entity (m_freeEntities), otherwise issue a new one
    Entity id;
    if (!m_freeEntities.empty()) {
        id = m_freeEntities.back();
        m_freeEntities.pop_back();
    } else {
        id = m_nextEntity;
        m_nextEntity++;
    }
    m_alive[id] = true;

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