#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace para {
    // to match PascalCase in project + for reducing dependence on glm
    using Vec3 = glm::vec3;
    using Quat = glm::quat;
}