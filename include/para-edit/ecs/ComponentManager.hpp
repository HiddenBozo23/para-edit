// header-only because everything is template code

#pragma once

#include <array>
#include <memory>
#include <typeindex>
#include <unordered_map>

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {

class IComponentArray {
   public:
    virtual ~IComponentArray() = default;
    virtual void OnEntityDestroyed(Entity e) = 0;
};

template <typename T>
class ComponentArray : public IComponentArray {
   public:
    ComponentArray() { m_entityToIndex.fill(INVALID_INDEX); }

    void InsertData(Entity e, T c) {
        if (!m_ValidateEntity(e)) {
            LOG_ERROR("ComponentArray: failed to insert ("
                      << typeid(T).name() << ") component for "
                      << "entity (" << e << "). entity is invalid");
            return;
        }
        if (m_entityToIndex[e] != INVALID_INDEX) {
            LOG_ERROR("ComponentArray: failed to insert ("
                      << typeid(T).name() << ") component for "
                      << "entity (" << e << "). component already exists for this entity");
            return;
        }

        size_t lastIndex = m_components.size();

        m_entityToIndex[e] = lastIndex;
        m_indexToEntity.push_back(e);
        m_components.push_back(std::move(c));
    }

    void RemoveData(Entity e) {
        if (!m_ValidateEntity(e)) {
            LOG_ERROR("ComponentArray: failed to remove ("
                      << typeid(T).name() << ") component for "
                      << "entity (" << e << "). entity is invalid");
            return;
        }
        if (m_entityToIndex[e] == INVALID_INDEX) {
            LOG_ERROR("ComponentArray: failed to remove ("
                      << typeid(T).name() << ") component for "
                      << "entity (" << e << "). component does not exist for this entity");
            return;
        }

        // maintain the density of m_component (by replacing the removed data with the end of the array)
        if (m_entityToIndex[e] != m_components.size() - 1) {
            size_t indexOfRemovedEntity = m_entityToIndex[e];
            size_t indexOfLastElement = m_components.size() - 1;
            Entity entityOfLastElement = m_indexToEntity[indexOfLastElement];

            m_components[indexOfRemovedEntity] = std::move(m_components[indexOfLastElement]);
            m_indexToEntity[indexOfRemovedEntity] = entityOfLastElement;
            m_entityToIndex[entityOfLastElement] = indexOfRemovedEntity;
        }

        m_entityToIndex[e] = INVALID_INDEX;
        m_components.pop_back();
        m_indexToEntity.pop_back();
    }

    T* GetData(Entity e) {
        if (!m_ValidateEntity(e)) {
            LOG_ERROR("ComponentArray: failed to get ("
                      << typeid(T).name() << ") component for "
                      << "entity (" << e << "). entity is invalid");
            return nullptr;
        }
        if (m_entityToIndex[e] == INVALID_INDEX) {
            LOG_ERROR("ComponentArray: failed to get ("
                      << typeid(T).name() << ") component for "
                      << "entity (" << e << "). component does not exist for this entity");
            return nullptr;
        }

        return &m_components[m_entityToIndex[e]];
    }

    void OnEntityDestroyed(Entity e) override {
        if (m_ValidateEntity(e) && m_entityToIndex[e] != INVALID_INDEX) {
            RemoveData(e);
        }
    }

   private:
    std::vector<T> m_components{};
    std::array<size_t, MAX_ENTITIES> m_entityToIndex{};
    std::vector<Entity> m_indexToEntity{};

    bool m_ValidateEntity(Entity e) {
        return (e < MAX_ENTITIES && e != INVALID_ENTITY);
    }
};

class ComponentManager {
   public:
    template <typename T>
    void RegisterComponent() {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_componentIndexMap.find(typeIndex) != m_componentIndexMap.end()) {
            LOG_ERROR("ComponentManager: failed to register component array of type ("
                      << typeid(T).name() << "). already registered");
            return;
        }
        if (m_nextComponentIndex >= MAX_COMPONENTS) {
            LOG_ERROR("ComponentManager: failed to register component array of type ("
                      << typeid(T).name() << "). maximum components (" << MAX_COMPONENTS << ") reached.");
            return;
        }

        m_componentIndexMap.insert({typeIndex, m_nextComponentIndex});
        m_componentArrayMap.insert({typeIndex, std::make_unique<ComponentArray<T>>()});
        m_nextComponentIndex++;
    }

    template <typename T>
    ComponentIndex GetComponentIndex() {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_componentIndexMap.find(typeIndex) == m_componentIndexMap.end()) {
            LOG_ERROR("ComponentManager: failed to get component index for type ("
                      << typeid(T).name() << "). not registered");
            return INVALID_COMPONENT;
        }

        return m_componentIndexMap[typeIndex];
    }

    template <typename T>
    void AddComponent(Entity e, T c) {
        auto componentArray = m_GetComponentArray<T>();

        if (!componentArray) {
            LOG_ERROR("ComponentManager: failed add component of type ("
                      << typeid(T).name() << "). not registered");
            return;
        }

        componentArray->InsertData(e, std::move(c));
    }

    template <typename T>
    void RemoveComponent(Entity e) {
        auto componentArray = m_GetComponentArray<T>();

        if (!componentArray) {
            LOG_ERROR("ComponentManager: failed remove component of type ("
                      << typeid(T).name() << "). not registered");
            return;
        }

        componentArray->RemoveData(e);
    }

    template <typename T>
    T* GetComponent(Entity e) {
        auto componentArray = m_GetComponentArray<T>();

        if (!componentArray) {
            LOG_ERROR("ComponentManager: failed get component of type ("
                      << typeid(T).name() << "). not registered");
            return nullptr;
        }

        return componentArray->GetData(e);
    }

    void EntityDestroyed(Entity e) {
        for (auto& pair : m_componentArrayMap) {
            auto& component = pair.second;
            component->OnEntityDestroyed(e);
        }
    }

   private:
    std::unordered_map<std::type_index, ComponentIndex> m_componentIndexMap{};
    std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> m_componentArrayMap{};
    ComponentIndex m_nextComponentIndex{};

    template <typename T>
    ComponentArray<T>* m_GetComponentArray() {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_componentArrayMap.find(typeIndex) == m_componentArrayMap.end()) {
            return nullptr;
        }

        return static_cast<ComponentArray<T>*>(m_componentArrayMap[typeIndex].get());
    }
};
}  // namespace para