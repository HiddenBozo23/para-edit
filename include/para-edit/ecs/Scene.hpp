// header only because of template code
#pragma once

#include <functional>
#include <unordered_map>

#include "para-edit/ecs/ComponentManager.hpp"
#include "para-edit/ecs/ComponentSerialiser.hpp"
#include "para-edit/ecs/EntityManager.hpp"
#include "para-edit/ecs/SystemManager.hpp"
#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/systems/HierarchySystem.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {
class Scene {
   public:
    struct ComponentConstructor {
        std::function<void(Entity, Scene&)> construct;
        std::function<void(Entity, Scene&)> destruct;
    };

    Scene() {
        m_entityManager = std::make_unique<EntityManager>();
        m_componentManager = std::make_unique<ComponentManager>();
        m_systemManager = std::make_unique<SystemManager>();

        RegisterComponent<Name>("name");
        ComponentIndex ci = RegisterComponent<Hierarchy>("hierarchy");
        RegisterComponent<Transform>("transform");

        RegisterSystem<HierarchySystem>();
        Signature signature;
        signature.set(ci);
        SetSystemSignature<HierarchySystem>(signature);
    }

    Entity CreateEntity() {
        Entity entity = m_entityManager->CreateEntity();

        if (entity != INVALID_ENTITY) {
            AddComponent<Name>(entity, {});
            AddComponent<Hierarchy>(entity, {});
        }

        return entity;
    }

    void DestroyEntity(Entity e) {
        if (e == INVALID_ENTITY) {
            return;
        }

        m_systemManager->EntityDestroyed(e);
        m_componentManager->EntityDestroyed(e);
        m_entityManager->DestroyEntity(e);
    }

    template <typename T>
    ComponentIndex RegisterComponent(const std::string& name) {
        ComponentIndex ci = m_componentManager->RegisterComponent<T>();

        if (ci != INVALID_COMPONENT) {
            m_nameToIndex[name] = ci;

            ComponentConstructor cc;
            cc.construct = [](Entity entity, Scene& scene) {
                scene.AddComponent<T>(entity, {});
            };
            cc.destruct = [](Entity entity, Scene& scene) {
                scene.RemoveComponent<T>(entity);
            };
            m_componentConstructors[name] = (std::move(cc));

            m_ComponentSerialiser cs;
            cs.name = name;
            cs.serialise = [](Entity entity, Scene& scene) {
                T* component = scene.GetComponent<T>(entity);
                return ComponentSerialiser::SerialiseComponent<T>(*component);
            };
            cs.deserialise = [](Entity entity, Scene& scene, const std::string& string) {
                scene.AddComponent<T>(entity, ComponentSerialiser::DeserialiseComponent<T>(string));
            };
            m_ComponentSerialisers[ci] = (std::move(cs));
        }

        return ci;
    }

    template <typename T>
    bool HasComponent(Entity e) {
        if (!m_entityManager->GetEntityAliveValid(e)) {
            return false;
        }

        ComponentIndex ci = m_componentManager->GetComponentIndex<T>();
        if (ci == INVALID_COMPONENT) {
            return false;
        }

        Signature s;
        s.set(ci);

        return (s & m_entityManager->GetSignature(e)) != 0;
    }

    template <typename T>
    void AddComponent(Entity e, T c) {
        ComponentIndex ci = m_componentManager->GetComponentIndex<T>();

        if (ci == INVALID_COMPONENT) {
            return;
        }

        // add component data before notifying systems so it can be used in OnEntityAdded
        m_componentManager->AddComponent(e, c);

        Signature s = m_entityManager->GetSignature(e);
        s.set(ci, true);
        m_entityManager->SetSignature(e, s);
        m_systemManager->EntitySignatureChanged(e, s);
    }

    // default component construction
    void AddComponentByName(const std::string& name, Entity entity) {
        auto it = m_componentConstructors.find(name);
        if (it == m_componentConstructors.end()) {
            LOG_ERROR("Scene: failed to add component by unknown name (" << name << ")");
            return;
        }

        it->second.construct(entity, *this);
    }

    template <typename T>
    void RemoveComponent(Entity e) {
        ComponentIndex ci = m_componentManager->GetComponentIndex<T>();

        if (ci == INVALID_COMPONENT) {
            return;
        }

        Signature s = m_entityManager->GetSignature(e);
        s.set(ci, false);
        m_entityManager->SetSignature(e, s);
        m_systemManager->EntitySignatureChanged(e, s);

        // remove component data after notifying systems so they safely use it in OnEntityRemoved
        m_componentManager->RemoveComponent<T>(e);
    }

    void RemoveComponentByName(const std::string& name, Entity entity) {
        auto it = m_componentConstructors.find(name);
        if (it == m_componentConstructors.end()) {
            LOG_ERROR("Scene: failed to remove component by unknown name (" << name << ")");
            return;
        }

        it->second.destruct(entity, *this);
    }

    template <typename T>
    T* GetComponent(Entity e) {
        return m_componentManager->GetComponent<T>(e);
    }

    template <typename T>
    ComponentIndex GetComponentIndex() {
        return m_componentManager->GetComponentIndex<T>();
    }

    template <typename T, typename... Args>
    T* RegisterSystem(Args&&... args) {
        return m_systemManager->RegisterSystem<T>(*this, std::forward<Args>(args)...);
    }

    template <typename T>
    T* GetSystem() {
        return m_systemManager->GetSystem<T>();
    }

    template <typename T>
    void SetSystemSignature(Signature s) {
        m_systemManager->SetSignature<T>(s);
    }

    const std::unordered_map<std::string, ComponentConstructor>& GetComponentConstructors() {
        return m_componentConstructors;
    }

    std::string SerialiseEntity(Entity e) {
        nlohmann::json j;

        Signature s = m_entityManager->GetSignature(e);

        for (const auto& pair : m_ComponentSerialisers) {
            const auto& ci = pair.first;
            const auto& funcs = pair.second;
            if (s[ci]) {
                j[funcs.name] = funcs.serialise(e, *this);
            }
        }

        return j.dump();
    }

    // deseralising is unsafe - it assumes all entities are free
    // and the same as when they were serialised
    void DeserialiseEntity(Entity entity, const std::string& string) {
        nlohmann::json j = nlohmann::json::parse(string);

        m_entityManager->CreateEntity(entity);

        for (const auto& item : j.items()) {
            std::string componentName = item.key();
            auto it = m_nameToIndex.find(componentName);
            if (it == m_nameToIndex.end()) {
                LOG_ERROR("Scene: failed to deserialise unknown type (" << componentName << "). skipping");
                continue;
            }

            m_ComponentSerialisers.at(it->second).deserialise(entity, *this, item.value());
        }
    }

    std::string SerialiseScene() {
        nlohmann::json j;

        std::vector<Entity> entities = m_entityManager->GetAliveEntities();

        for (Entity e : entities) {
            j[std::to_string(e)] = nlohmann::json::parse(SerialiseEntity(e));
        };

        return j.dump();
    }

   private:
    // deseralising is unsafe - it assumes all entities are free
    // and the same as when they were serialised
    void m_DeserialiseScene(const std::string& string) {
        nlohmann::json j = nlohmann::json::parse(string);

        for (const auto& entry : j.items()) {
            Entity entity = static_cast<Entity>(std::stoul(entry.key()));

            if (entity == INVALID_ENTITY) {
                LOG_ERROR("Scene: failed to create entity (" << entity << ") while deserialising. skipping");
                continue;
            }

            DeserialiseEntity(entity, entry.value());
        }
    }

    struct m_ComponentSerialiser {
        std::string name;
        std::function<std::string(Entity, Scene&)> serialise;
        std::function<void(Entity, Scene&, const std::string&)> deserialise;
    };

    struct m_Prefab {
        std::string name;
        std::function<void()> constructor;
    };

    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<ComponentManager> m_componentManager;
    std::unique_ptr<SystemManager> m_systemManager;

    std::unordered_map<std::string, ComponentIndex> m_nameToIndex{};
    // only handles default component construction
    std::unordered_map<std::string, ComponentConstructor> m_componentConstructors{};
    std::unordered_map<ComponentIndex, m_ComponentSerialiser> m_ComponentSerialisers{};
};
}  // namespace para