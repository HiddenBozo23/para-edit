#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "para-edit/editor/View.hpp"

namespace para {
class ViewManager {
   public:
    template <typename T, typename... Args>
    T* AddView(Args&&... args) {
        auto view = std::make_unique<T>(std::forward<Args>(args)...);
        T* pointer = view.get();
        m_views.push_back(std::move(view));

        return pointer;
    }

    void RenderAll() {
        for (auto& view : m_views) {
            view->OnRender();
        }
    }

   private:
    std::vector<std::unique_ptr<View>> m_views{};
};
}  // namespace para