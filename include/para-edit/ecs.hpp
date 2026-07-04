#include <cstdint>
#include <vector>
#include <array>
#include <bitset>
#include <set>
#include <unordered_map>
#include <memory>
#include <typeinfo>
#include <typeindex>

#include "para-edit/logger.hpp"

// --- ENTITY

using Entity = std::uint16_t;
constexpr Entity INVALID_ENTITY = UINT16_MAX;
constexpr std::size_t MAX_ENTITIES = 10000;  // includes entity 0 - max valid value is 9999

using ComponentIndex = std::uint8_t;  // max 255 components
constexpr ComponentIndex INVALID_COMPONENT = UINT8_MAX;
constexpr std::size_t MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

constexpr std::size_t INVALID_INDEX = SIZE_MAX;

class EntityManager {
public:
    Entity CreateEntity() {
        if (m_nextEntity == MAX_ENTITIES && m_freeEntities.empty()) {
            LOG_ERROR("EntityManager: failed to create entity. maximum entities (" << MAX_ENTITIES << ") reached");
            return INVALID_ENTITY;
        }

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

    void DestroyEntity( Entity e ) {
        if (!GetEntityAliveValid(e)) {
            LOG_ERROR("EntityManager: failed to destroy invalid entity (" << e << ")");
            return;
        }

        m_signatures[e].reset();
        m_freeEntities.push_back(e);
        m_alive[e] = false;
    }

    void SetSignature( Entity e, Signature s ) {
        if (!GetEntityAliveValid(e)) {
            LOG_ERROR("EntityManager: failed to set the signature of invalid entity (" << e << ")");
            return;
        }

        m_signatures[e] = s;
    }

    Signature GetSignature( Entity e ) {
        if (!GetEntityAliveValid(e)) {
            LOG_ERROR("EntityManager: failed to get the signature of invalid entity (" << e << ")");
            return Signature{};
        }

        return m_signatures[e];
    }

    bool GetEntityAliveValid( Entity e ) {
        if (e >= MAX_ENTITIES || e == INVALID_ENTITY ) {
            return false;
        }
        return m_alive[e];
    }

private:
    Entity m_nextEntity{};
    std::vector<Entity> m_freeEntities{};

    std::bitset<MAX_ENTITIES> m_alive{};
    // array is a little memory heavy (~4byte per entity at construction) - consider using vector if mem is tight
    std::array<Signature, MAX_ENTITIES> m_signatures{};
};

// --- COMPONENT

class IComponentArray {
    public:
        virtual ~IComponentArray() = default;
        virtual void OnEntityDestroyed( Entity e ) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public:
    ComponentArray() { 
        m_entityToIndex.fill(INVALID_INDEX);
    }

    void InsertData( Entity e, T c ) {
        if (!m_ValidateEntity(e)) {
            LOG_ERROR("ComponentArray: failed to insert (" << typeid(T).name() << ") component for " <<
                       "entity (" << e << "). entity is invalid");
            return;
        }
        if (m_entityToIndex[e] != INVALID_INDEX) {
            LOG_ERROR("ComponentArray: failed to insert (" << typeid(T).name() << ") component for " <<
                       "entity (" << e << "). component already exists for this entity");
            return;
        }

        size_t lastIndex = m_components.size();

        m_entityToIndex[e] = lastIndex;
        m_indexToEntity.push_back(e);
        m_components.push_back(std::move(c));
    }

    void RemoveData( Entity e ) {
        if (!m_ValidateEntity(e)) {
            LOG_ERROR("ComponentArray: failed to remove (" << typeid(T).name() << ") component for " <<
                       "entity (" << e << "). entity is invalid");
            return;
        }
        if (m_entityToIndex[e] == INVALID_INDEX) {
            LOG_ERROR("ComponentArray: failed to remove (" << typeid(T).name() << ") component for " <<
                       "entity (" << e << "). component does not exist for this entity");
            return;
        }

        if (m_entityToIndex[e] != m_components.size() - 1) {
            // overwrite component with last element of array
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

    T* GetData( Entity e ) {
        if (!m_ValidateEntity(e)) {
            LOG_ERROR("ComponentArray: failed to get (" << typeid(T).name() << ") component for " <<
                       "entity (" << e << "). entity is invalid");
            return nullptr;
        }
        if (m_entityToIndex[e] == INVALID_INDEX) {
            LOG_ERROR("ComponentArray: failed to get (" << typeid(T).name() << ") component for " <<
                       "entity (" << e << "). component does not exist for this entity");
            return nullptr;
        }

        return &m_components[m_entityToIndex[e]];
    }

    void OnEntityDestroyed( Entity e ) override {
        if (m_ValidateEntity(e) && m_entityToIndex[e] != INVALID_INDEX) {
            RemoveData(e);
        }
    }

private:
    std::vector<T> m_components{};
    std::array<size_t, MAX_ENTITIES> m_entityToIndex{};
    std::vector<Entity> m_indexToEntity{};

    bool m_ValidateEntity( Entity e ) {
        return (e < MAX_ENTITIES && e != INVALID_ENTITY);
    }
};

class ComponentManager {
public:
    template<typename T>
    void RegisterComponent() {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_componentIndexMap.find(typeIndex) != m_componentIndexMap.end()) {
            LOG_ERROR("ComponentManager: failed to register component array of type (" << typeid(T).name() << 
                      "). already registered");
            return;
        }
        if (m_nextComponentIndex >= MAX_COMPONENTS) {
            LOG_ERROR("ComponentManager: failed to register component array of type (" << typeid(T).name() << 
                      "). maximum components (" << MAX_COMPONENTS << ") reached.");
            return;
        }

        m_componentIndexMap.insert({typeIndex, m_nextComponentIndex});
        m_componentArrayMap.insert({typeIndex, std::make_unique<ComponentArray<T>>()});
        m_nextComponentIndex++;
    }

    template<typename T>
    ComponentIndex GetComponentIndex() {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_componentIndexMap.find(typeIndex) == m_componentIndexMap.end()) {
            LOG_ERROR("ComponentManager: failed to get component index for type (" << typeid(T).name() << 
                      "). not registered");
            return INVALID_COMPONENT;
        }

        return m_componentIndexMap[typeIndex];
    }

    template<typename T>
    void AddComponent( Entity e, T c ) {
        auto componentArray = m_GetComponentArray<T>();

        if (!componentArray) {
            LOG_ERROR("ComponentManager: failed add component of type (" << typeid(T).name() << 
                      "). not registered");
            return;
        }

        componentArray->InsertData(e, std::move(c));
    }

    template <typename T>
    void RemoveComponent( Entity e ) {
        auto componentArray = m_GetComponentArray<T>();

        if (!componentArray) {
            LOG_ERROR("ComponentManager: failed remove component of type (" << typeid(T).name() << 
                      "). not registered");
            return;
        }

        componentArray->RemoveData(e);
    }

    template <typename T>
    T* GetComponent( Entity e ) {
        auto componentArray = m_GetComponentArray<T>();

        if (!componentArray) {
            LOG_ERROR("ComponentManager: failed get component of type (" << typeid(T).name() << 
                      "). not registered");
            return nullptr;
        }

        return componentArray->GetData(e);
    }

    void EntityDestroyed( Entity e ) {
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

        return m_componentArrayMap[typeIndex].get();
    }
};

// --- SYSTEM

class System {
public: 
    virtual ~System() = default;

    void AddEntity( Entity e ) {
        m_entities.insert(e);
    }

    void RemoveEntity( Entity e ) {
        m_entities.erase(e);
    }
protected:
    std::set<Entity> m_entities;
};

class SystemManager {
public:
    template<typename T, typename... Args>
    System* RegisterSystem( Args&&... args ) {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_systemMap.find(typeIndex) != m_systemMap.end()) {
            LOG_ERROR("SystemManager: failed to register system of type (" << typeid(T).name() << 
                      "). already registered");
            return nullptr;
        }

        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* pointer  = system.get();
        m_systemMap.emplace(typeIndex, std::move(system));
        return pointer;
    }

    template<typename T>
    void SetSignature( Signature s ) {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_systemMap.find(typeIndex) == m_systemMap.end()) {
            LOG_ERROR("SystemManager: failed to set signature of system of type (" << typeid(T).name() << 
                      "). not registered");
            return;
        }

        m_signatureMap[typeIndex] = s;
    }

    void EntityDestroyed( Entity e ) {
        for (auto& pair: m_systemMap) {
            auto& system = pair.second;
            system->RemoveEntity(e);
        }
    }

    void EntitySignatureChanged( Entity e, Signature entitySignature ) {
        for (auto& pair : m_systemMap) {
            auto& index = pair.first;
            auto& system = pair.second;
            auto it = m_signatureMap.find(index);
            if (it == m_signatureMap.end()) {
                continue;
            }
            auto& systemSignature = it->second;

            if ((entitySignature & systemSignature) == systemSignature) {
                system->AddEntity(e);
            } else {
                system->RemoveEntity(e);
            }
        }
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<System>> m_systemMap{};
    std::unordered_map<std::type_index, Signature> m_signatureMap{};
};

// --- SCENE (/COORDINATOR)

class Scene {
public:
    Scene() {
        m_entityManager = std::make_unique<EntityManager>();
        m_componentManager = std::make_unique<ComponentManager>();
        m_systemManager = std::make_unique<SystemManager>();
    }

    Entity CreateEntity() {
        return m_entityManager->CreateEntity();
    }

    void DestroyEntity( Entity e ) {
        m_entityManager->DestroyEntity(e);
        m_componentManager->EntityDestroyed(e);
        m_systemManager->EntityDestroyed(e);
    }

    template<typename T>
    void RegisterComponent() {
        m_componentManager->RegisterComponent<T>();
    }

    template<typename T>
    void AddComponent( Entity e, T c ) {
        ComponentIndex ci = m_componentManager->GetComponentIndex<T>();

        if (ci == INVALID_COMPONENT) {
            return;
        }

        m_componentManager->AddComponent(e, c);

        Signature s = m_entityManager->GetSignature(e);
        s.set(ci, true);
        m_entityManager->SetSignature(e, s);

        m_systemManager->EntitySignatureChanged(e, s);
    }

    template<typename T>
    void RemoveComponent( Entity e ) {
        ComponentIndex ci = m_componentManager->GetComponentIndex<T>();

        if (ci == INVALID_COMPONENT) {
            return;
        }

        m_componentManager->RemoveComponent<T>(e);

        Signature s = m_entityManager->GetSignature(e);
        s.set(ci, false);
        m_entityManager->SetSignature(e, s);

        m_systemManager->EntitySignatureChanged(e, s);
    }

    template<typename T>
    T* GetComponent( Entity e ) {
        return m_componentManager->GetComponent<T>(e);
    }

    template<typename T>
    ComponentIndex GetComponentIndex() {
        return m_componentManager->GetComponentIndex<T>();
    }

    template<typename T, typename... Args>
    T* RegisterSystem( Args&&... args ) {
        return m_systemManager->RegisterSystem<T>(std::forward(args)...);
    }

    template<typename T>
    void SetSystemSignature( Signature s ) {
        m_systemManager->SetSignature<T>(s);
    }

private:
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<ComponentManager> m_componentManager;
    std::unique_ptr<SystemManager> m_systemManager;
};