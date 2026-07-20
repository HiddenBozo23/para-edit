#include "para-edit/core/Application.hpp"

#include <GLFW/glfw3.h>

#include <memory>
#include <stdexcept>

#include "para-edit/core/Logger.hpp"
#include "para-edit/core/Window.hpp"

namespace para {
Application::Application() {
    if (!glfwInit()) {
        LOG_ERROR("Application: failed to initialise GLFW");
        throw std::runtime_error("failed to initialise GLFW");
    }
    glfwWindowHint(GLFW_SAMPLES, 4);

    Window::Config config{};
    Window::ResizeCallback resizeCallback = [this](int width, int height) -> void { m_graphicsDevice->SetViewport(width, height); };

    m_window = std::make_unique<Window>(config, resizeCallback);
    m_graphicsDevice = std::make_unique<GraphicsDevice>();
}

Application::~Application() {
    glfwTerminate();
}
}  // namespace para