#pragma once

// all GL calls happen within this interface
// to keep engine graphics implementation-agnostic

namespace para {
class GraphicsDevice {
   public:
    GraphicsDevice();

    void SetViewport(int width, int height);

    void Clear();
};
}  // namespace para