// header only because of template code
#pragma once

#include "para-edit/ecs/ComponentManager.hpp"
#include "para-edit/ecs/EntityManager.hpp"
#include "para-edit/ecs/SystemManager.hpp"

namespace para {
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

    void DestroyEntity(Entity e) {
        m_systemManager->EntityDestroyed(e);
        m_componentManager->EntityDestroyed(e);
        m_entityManager->DestroyEntity(e);
    }

    template <typename T>
    void RegisterComponent() {
        m_componentManager->RegisterComponent<T>();
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
        return m_systemManager->RegisterSystem<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    T* GetSystem() {
        return m_systemManager->GetSystem<T>();
    }

    template <typename T>
    void SetSystemSignature(Signature s) {
        m_systemManager->SetSignature<T>(s);
    }

   private:
    std::unique_ptr<EntityManager> m_entityManager;
    std::unique_ptr<ComponentManager> m_componentManager;
    std::unique_ptr<SystemManager> m_systemManager;
};
}  // namespace para