#pragma once

// must be included before glfw
#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <functional>
#include <string>

#include "para-edit/core/Logger.hpp"

namespace para {
class Window {
   public:
    using ResizeCallback = std::function<void(int width, int height)>;
    struct Config {
        int width = 1200;
        int height = 700;
        std::string title = "para-edit";
    };

    explicit Window(const Config& config, ResizeCallback resizeCallback)
        : m_resizeCallback(std::move(resizeCallback)) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(config.width, config.height,
                                    config.title.c_str(), nullptr, nullptr);

        if (!m_window) {
            LOG_ERROR("Window: failed to create glfw window");
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(m_window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG_ERROR("Window: failed to initialise GLAD");
            throw std::runtime_error("Failed to initialise GLAD");
        }

        glfwSetWindowUserPointer(m_window, this);
        glfwSetFramebufferSizeCallback(m_window, &Window::m_OnFramebufferResize);
    }

    ~Window() {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
    }

    void SwapBuffer() {
        glfwSwapBuffers(m_window);
    }

    bool ShouldClose() {
        return glfwWindowShouldClose(m_window);
    }

    void Poll() {
        glfwPollEvents();
    }

    float GetTime() {
        return glfwGetTime();
    }

    // exposes the implementation with return type unfortunately.
    // friend class privilleges would not be inherited
    GLFWwindow* GetHandle() {
        return m_window;
    }

   private:
    static void m_OnFramebufferResize(GLFWwindow* window, int width, int height) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_resizeCallback) {
            self->m_resizeCallback(width, height);
        }
    }
    GLFWwindow* m_window = nullptr;
    ResizeCallback m_resizeCallback;
};
}  // namespace para