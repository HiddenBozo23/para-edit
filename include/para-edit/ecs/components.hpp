#pragma once

#include "para-edit/core/math.hpp"
#include "para-edit/ecs/types.hpp"

namespace para {

struct Name {
    char name[256];
};

// doubly-linked list in component - guarantees fixed-size with no vector heap alloc.
struct Hierarchy {
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity previousSibling = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
};

struct Transform {
    Vec3 position{};
    Vec3 scale{1, 1, 1};
    Quat rotation{};
};

}  // namespace para