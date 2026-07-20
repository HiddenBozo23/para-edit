#pragma once

#include <cstdint>

#include "para-edit/editor/View.hpp"

namespace para {
class LogView : public View {
   public:
    LogView(Editor& editor)
        : View(editor) {}

    void OnRender() override;

   private:
    bool m_verbose = true;
    uint32_t m_filterMask = ~0;
};
}  // namespace para