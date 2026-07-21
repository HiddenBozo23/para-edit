// header-only because of template code

#pragma once

#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {
// forward declare system class to break circluar dependancies
// remember that systems that include this file must also fully include Scene.hpp
class Scene;

class System {
   public:
    explicit System(Scene& s) : m_scene(s) {}
    virtual ~System() = default;

    void AddEntity(Entity e) {
        if (m_entities.find(e) != m_entities.end()) {
            return;
        }

        m_entities.insert(e);
        OnEntityAdded(e);
    }

    void RemoveEntity(Entity e) {
        if (m_entities.find(e) == m_entities.end()) {
            return;
        }

        OnEntityRemoved(e);
        m_entities.erase(e);
    }

    const std::set<Entity>& GetEntities() {
        return m_entities;
    }

   protected:
    Scene& m_scene;

    std::set<Entity> m_entities{};

    // these separate virtual methods guarantee that the function
    // overrides can never accidentally forget to remove e from m_entities
    void virtual OnEntityAdded(Entity e) {}
    void virtual OnEntityRemoved(Entity e) {}
};

class SystemManager {
   public:
    template <typename T, typename... Args>
    T* RegisterSystem(Scene& scene, Args&&... args) {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_systemMap.find(typeIndex) != m_systemMap.end()) {
            LOG_ERROR("SystemManager: failed to register system of type ("
                      << typeid(T).name() << "). already registered");
            return nullptr;
        }

        auto system = std::make_unique<T>(scene, std::forward<Args>(args)...);
        T* pointer = system.get();
        m_systemMap.emplace(typeIndex, std::move(system));

        return pointer;
    }

    template <typename T>
    T* GetSystem() {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_systemMap.find(typeIndex) == m_systemMap.end()) {
            LOG_ERROR("SystemManager: failed to get system of type ("
                      << typeid(T).name() << "). not registered");
            return nullptr;
        }

        auto pointer = static_cast<T*>(m_systemMap.at(typeIndex).get());
        return pointer;
    }

    template <typename T>
    void SetSignature(Signature s) {
        std::type_index typeIndex = std::type_index(typeid(T));

        if (m_systemMap.find(typeIndex) == m_systemMap.end()) {
            LOG_ERROR("SystemManager: failed to set signature of system of type ("
                      << typeid(T).name() << "). not registered");
            return;
        }

        m_signatureMap[typeIndex] = s;
    }

    void EntityDestroyed(Entity e) {
        for (auto& pair : m_systemMap) {
            auto& system = pair.second;
            system->RemoveEntity(e);
        }
    }

    void EntitySignatureChanged(Entity e, Signature entitySignature) {
        for (auto& pair : m_systemMap) {
            auto& index = pair.first;
            auto& system = pair.second;

            auto it = m_signatureMap.find(index);
            if (it == m_signatureMap.end()) {
                continue;
            }

            auto& systemSignature = it->second;

            // calls even if the entity is already tracked by the system,
            // a check is done inside Add/Remove entity respectively
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
}  // namespace para