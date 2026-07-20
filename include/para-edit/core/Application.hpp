#pragma once

#include <memory>

#include "para-edit/core/GraphicsDevice.hpp"
#include "para-edit/core/Window.hpp"

namespace para {
class Application {
   public:
    explicit Application();
    virtual ~Application();

    virtual void Run() {};

   protected:
    // unique-ptr used here to defer construction until glInit etc is called
    std::unique_ptr<Window> m_window;
    std::unique_ptr<GraphicsDevice> m_graphicsDevice;
};
}  // namespace para