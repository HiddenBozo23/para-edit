#pragma once

namespace para {
// forward-declared class to break circular dependncy
// don't forget to include real Editor header in subclass cpp files
class Editor;

class View {
   public:
    explicit View(Editor& editor)
        : m_editor(editor) {};
    virtual ~View() = default;

    virtual void OnRender() = 0;

   protected:
    virtual void m_ContextMenu();
    Editor& m_editor;
};
}  // namespace para