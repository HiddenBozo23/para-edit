#include "para-edit/ecs/systems/HierarchySystem.hpp"

#include "para-edit/core/Logger.hpp"
#include "para-edit/ecs/components.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {
void HierarchySystem::SetParent(Entity child, Entity parent) {
    Hierarchy* childH = m_scene.GetComponent<Hierarchy>(child);

    if (!childH) {
        LOG_ERROR("HierarchySystem: failed to get Hierachy of entity (" << child << ")");
        return;
    }

    // remove relations between potential old parents and siblings
    m_UnlinkFromParent(childH);

    // set parent of child's hierarchy component
    childH->parent = parent;

    if (parent == INVALID_ENTITY) {
        return;
    }

    Hierarchy* parentH = m_scene.GetComponent<Hierarchy>(parent);
    Entity sibling = parentH->firstChild;

    // if there are no siblings insert child as it's parent's first child with no siblings
    if (sibling == INVALID_ENTITY) {
        parentH->firstChild = child;
        childH->previousSibling = INVALID_ENTITY;
        childH->nextSibling = INVALID_ENTITY;
    } else {
        // if siblings exist, add the child to the end of the linked list
        Hierarchy* siblingH = m_scene.GetComponent<Hierarchy>(sibling);

        while (siblingH->nextSibling != INVALID_ENTITY) {
            sibling = siblingH->nextSibling;
            siblingH = m_scene.GetComponent<Hierarchy>(sibling);
        }

        siblingH->nextSibling = child;
        childH->previousSibling = sibling;
        childH->nextSibling = INVALID_ENTITY;
    }
}

void HierarchySystem::OnEntityRemoved(Entity e) {
    Hierarchy* eH = m_scene.GetComponent<Hierarchy>(e);

    Entity child = eH->firstChild;
    Hierarchy* childH{};

    if (m_cascadeDelete) {
        m_UnlinkFromParent(eH);

        // delete all children
        while (child != INVALID_ENTITY) {
            childH = m_scene.GetComponent<Hierarchy>(child);
            Entity next = childH->nextSibling;

            m_scene.DestroyEntity(child);

            child = next;
        }
    } else {
        Entity parent = eH->parent;

        if (parent == INVALID_ENTITY) {
            // reset all children's hierarchy data (not recursive)
            while (child != INVALID_ENTITY) {
                childH = m_scene.GetComponent<Hierarchy>(child);

                childH->parent = INVALID_ENTITY;
                childH->previousSibling = INVALID_ENTITY;
                childH->nextSibling = INVALID_ENTITY;

                child = childH->nextSibling;
            }
        } else {
            if (eH->firstChild == INVALID_ENTITY) {
                m_UnlinkFromParent(eH);
            } else {
                Hierarchy* parentH = m_scene.GetComponent<Hierarchy>(parent);
                Hierarchy* siblingH{};

                // insert e's children to e's position
                if (parentH->firstChild == e) {
                    childH = m_scene.GetComponent<Hierarchy>(eH->firstChild);

                    parentH->firstChild = eH->firstChild;
                } else {
                    siblingH = m_scene.GetComponent<Hierarchy>(eH->previousSibling);
                    childH = m_scene.GetComponent<Hierarchy>(eH->firstChild);

                    siblingH->nextSibling = eH->firstChild;
                    childH->previousSibling = eH->previousSibling;
                }

                // find e's last child - also set parent pointers while iterating through list
                childH = m_scene.GetComponent<Hierarchy>(eH->firstChild);
                childH->parent = parent;

                while (childH->nextSibling != INVALID_ENTITY) {
                    child = childH->nextSibling;
                    childH = m_scene.GetComponent<Hierarchy>(child);
                    childH->parent = parent;
                }

                // clean up end of inserted list
                if (eH->nextSibling != INVALID_ENTITY) {
                    siblingH = m_scene.GetComponent<Hierarchy>(eH->nextSibling);
                    siblingH->previousSibling = child;
                }

                childH->nextSibling = eH->nextSibling;
            }
        }
    }
}

bool HierarchySystem::IsAncestor(Entity entity, Entity ancestor) {
    auto hc = m_scene.GetComponent<Hierarchy>(entity);

    while (hc && hc->parent != INVALID_ENTITY) {
        if (hc->parent == ancestor) {
            return true;
        } else {
            hc = m_scene.GetComponent<Hierarchy>(hc->parent);
        }
    }

    return false;
}

void HierarchySystem::m_UnlinkFromParent(Hierarchy* eH) {
    if (eH->parent == INVALID_ENTITY) {
        return;
    }

    eH->nextSibling = INVALID_ENTITY;
    eH->previousSibling = INVALID_ENTITY;

    if (eH->previousSibling != INVALID_ENTITY) {
        m_scene.GetComponent<Hierarchy>(eH->previousSibling)->nextSibling = eH->nextSibling;
    } else {
        m_scene.GetComponent<Hierarchy>(eH->parent)->firstChild = eH->nextSibling;
    }

    if (eH->nextSibling != INVALID_ENTITY) {
        m_scene.GetComponent<Hierarchy>(eH->nextSibling)->previousSibling = eH->previousSibling;
    }
}
}  // namespace para