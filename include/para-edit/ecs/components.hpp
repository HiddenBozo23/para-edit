#pragma once

#include "para-edit/core/math.hpp"
#include "para-edit/ecs/types.hpp"

#include <nlohmann/detail/conversions/to_json.hpp>
#include <nlohmann/json.hpp>

namespace para {

struct Name {
    std::string name;
};

inline void to_json(nlohmann::json& j, const Name& n) {
    j = {{"name", n.name}};
}

inline void from_json(const nlohmann::json& j, Name& n) {
    n.name = j.at("name").get<std::string>();
}

// doubly-linked list in component - guarantees fixed-size with no vector heap alloc.
struct Hierarchy {
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity previousSibling = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
};

inline void to_json(nlohmann::json& j, const Hierarchy& h) {
    j = {
        {"parent", h.parent},
        {"firstChild", h.firstChild},
        {"previousSibling", h.previousSibling},
        {"nextSibling", h.nextSibling}};
}

inline void from_json(const nlohmann::json& j, Hierarchy& h) {
    h.parent = j.at("parent");
    h.firstChild = j.at("firstChild");
    h.previousSibling = j.at("previousSibling");
    h.nextSibling = j.at("nextSibling");
}

struct Transform {
    Vec3 position{};
    Vec3 scale{1, 1, 1};
    Quat rotation{};
};

inline void to_json(nlohmann::json& j, const Transform& t) {
    j = {{"position", {t.position.x, t.position.y, t.position.z}},
         {"rotation", {t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w}},
         {"scale", {t.scale.x, t.scale.y, t.scale.z}}};
}

inline void from_json(const nlohmann::json& j, Transform& t) {
    auto p = j.at("position");
    t.position = {p[0], p[1], p[2]};
    auto r = j.at("rotation");
    t.rotation = {r[3], r[0], r[1], r[2]};  // glm quat ctor is w,x,y,z
    auto s = j.at("scale");
    t.scale = {s[0], s[1], s[2]};
}

}  // namespace para