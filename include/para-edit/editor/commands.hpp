#pragma once

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/Scene.hpp"
#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/systems/HierarchySystem.hpp"
#include "para-edit/ecs/types.hpp"

// maybe commands should return a bool - false would mean failure and
// the command manager could remove them from the redo-undo list

namespace para {
class ICommand {
   public:
    ICommand(Scene& scene)
        : m_scene(scene) {}
    virtual ~ICommand() = default;

    virtual void Execute() = 0;
    virtual void Undo() = 0;

   protected:
    Scene& m_scene;
};

// this class assumes the reference to m_target remains stable
// expected usage involves data in
template <typename T>
class ValueUpdateCommand : public ICommand {
   public:
    ValueUpdateCommand(Scene& scene, Entity entity, T oldValue, T newValue)
        : ICommand(scene), m_entity(entity), m_oldValue(oldValue), m_newValue(newValue) {}

    ValueUpdateCommand(Scene& scene, Entity entity, T newValue)
        : ICommand(scene), m_entity(entity), m_newValue(std::move(newValue)) {
        T* component = m_scene.GetComponent<T>(entity);
        if (component) {
            m_oldValue = *component;
        } else {
            LOG_WARNING("ValueUpdatedCommand: failed to get the current component value of type ("
                        << typeid(T).name() << ") for entity (" << entity << ")");
        }
    }

    void Execute() override {
        if (T* c = m_scene.GetComponent<T>(m_entity)) {
            *c = m_newValue;
        }
    }
    void Undo() override {
        if (T* c = m_scene.GetComponent<T>(m_entity)) {
            *c = m_oldValue;
        }
    }

   private:
    Entity m_entity;
    T m_oldValue;
    T m_newValue;
};

class ReparentCommand : public ICommand {
   public:
    ReparentCommand(Scene& scene, Entity child, Entity newParent)
        : ICommand(scene), m_child(child), m_newParent(newParent) {
        auto hc = m_scene.GetComponent<Hierarchy>(child);

        if (hc) {
            m_oldParent = hc->parent;
        } else {
            LOG_ERROR("ReparentCommand: failed to get the hierarchy component for entity ("
                      << child << ")");
            return;
        }

        m_hs = scene.GetSystem<HierarchySystem>();
        if (!m_hs) {
            LOG_ERROR("ReparentCommand: failed to get the hierarchy system from the scene");
        }
    }

    void Execute() override {
        if (m_hs) {
            m_hs->SetParent(m_child, m_newParent);
        }
    }

    void Undo() override {
        if (m_hs) {
            m_hs->SetParent(m_child, m_oldParent);
        }
    }

   private:
    Entity m_child;
    Entity m_oldParent;
    Entity m_newParent;

    HierarchySystem* m_hs;
};

class CreateEntityCommand : public ICommand {
   public:
    CreateEntityCommand(Scene& scene, Entity parent, HierarchySystem& hs)
        : ICommand(scene), m_parent(parent), m_hs(hs) {}

    void Execute() override {
        m_entity = m_scene.CreateEntity();

        if (m_parent != INVALID_ENTITY) {
            m_hs.SetParent(m_entity, m_parent);
        }
    }

    void Undo() override {
        if (m_entity == INVALID_ENTITY) {
            return;
        }
        if (m_parent != INVALID_ENTITY) {
            m_hs.RemoveChild(m_entity, m_parent);
        }

        m_scene.DestroyEntity(m_entity);
    }

   private:
    Entity m_entity = INVALID_ENTITY;
    Entity m_parent;

    HierarchySystem& m_hs;
};

class DestroyEntityCommand : public ICommand {
   public:
    DestroyEntityCommand(Scene& scene, Entity entity, HierarchySystem& hs)
        : ICommand(scene), m_entity(entity), m_hs(hs) {
        m_entitySnapshot = m_scene.SerialiseEntity(entity);

        auto hc = m_scene.GetComponent<Hierarchy>(entity);

        m_parent = hc->parent;
        m_firstChild = hc->firstChild;

        if (m_firstChild != INVALID_ENTITY) {
            m_lastChild = m_firstChild;
            Hierarchy* childH = m_scene.GetComponent<Hierarchy>(m_firstChild);

            while (childH->nextSibling != INVALID_ENTITY) {
                m_lastChild = childH->nextSibling;
                childH = m_scene.GetComponent<Hierarchy>(m_lastChild);
            }
        }
    }

    void Execute() override {
        m_scene.DestroyEntity(m_entity);
    }

    void Undo() override {
        m_scene.DeserialiseEntity(m_entity, m_entitySnapshot);

        // restore hierachy
        auto hc = m_scene.GetComponent<Hierarchy>(m_entity);

        m_hs.SetParent(m_entity, m_parent);
        if (m_firstChild != INVALID_ENTITY) {
            hc->firstChild = m_firstChild;

            hc = m_scene.GetComponent<Hierarchy>(m_firstChild);
            hc->previousSibling = INVALID_ENTITY;
        }
        if (m_lastChild != INVALID_ENTITY) {
            hc = m_scene.GetComponent<Hierarchy>(m_lastChild);
            hc->nextSibling = INVALID_ENTITY;
        }
    }

   private:
    std::string m_entitySnapshot{};

    Entity m_entity;
    Entity m_parent = INVALID_ENTITY;
    Entity m_firstChild = INVALID_ENTITY;
    Entity m_lastChild = INVALID_ENTITY;

    HierarchySystem& m_hs;
};

template <typename T>
class AddComponentCommand : public ICommand {
   public:
    AddComponentCommand(Scene& scene, Entity entity)
        : ICommand(scene), m_entity(entity) {}

    void Execute() override {
        m_scene.AddComponent<T>(m_entity, T{});
    }

    void Undo() override {
        m_scene.RemoveComponent<T>(m_entity);
    }

   private:
    Entity m_entity;
};

class AddComponentByNameCommand : public ICommand {
   public:
    AddComponentByNameCommand(Scene& scene, const std::string& name, Entity entity)
        : ICommand(scene), m_name(name), m_entity(entity) {}

    void Execute() override {
        m_scene.AddComponentByName(m_name, m_entity);
    }

    void Undo() override {
        m_scene.RemoveComponentByName(m_name, m_entity);
    }

   private:
    std::string m_name;
    Entity m_entity;
};

class RemoveComponentByNameCommand : public ICommand {
   public:
    RemoveComponentByNameCommand(Scene& scene, const std::string& name, Entity entity)
        : ICommand(scene), m_name(name), m_entity(entity) {}

    void Execute() override {
        m_scene.RemoveComponentByName(m_name, m_entity);
    }

    void Undo() override {
        m_scene.AddComponentByName(m_name, m_entity);
    }

   private:
    std::string m_name;
    Entity m_entity;
};

template <typename T>
class RemoveComponentCommand : public ICommand {
   public:
    RemoveComponentCommand(Scene& scene, Entity entity)
        : ICommand(scene), m_entity(entity) {
        T* c = scene.GetComponent<T>(entity);
        if (c) {
            m_component = *c;
        } else {
            LOG_WARNING("RemoveComponentCommand: entity (" << entity << ") has no component of type ("
                                                           << typeid(T).name() << ") at construction time");
        }
    }

    void Execute() override {
        m_scene.RemoveComponent<T>(m_entity);
    }

    void Undo() override {
        m_scene.AddComponent(m_entity, m_component);
    }

   private:
    Entity m_entity;
    T m_component{};
};
}  // namespace para