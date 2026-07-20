
#include "para-edit/core/GraphicsDevice.hpp"

#include <glad/glad.h>

#include <GLFW/glfw3.h>

namespace para {
GraphicsDevice::GraphicsDevice() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
}

void GraphicsDevice::SetViewport(int width, int height) {
    glViewport(0, 0, width, height);
}

void GraphicsDevice::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
}  // namespace para