#pragma once

#include <memory>
#include <vector>

#include "para-edit/editor/commands.hpp"

namespace para {
class CommandManager {
   public:
    CommandManager(Scene& scene)
        : m_scene(scene) {}

    template <typename T, typename... Args>
    void Execute(Args&&... args) {
        auto command = std::make_unique<T>(m_scene, std::forward<Args>(args)...);

        command->Execute();
        m_undoList.push_back(std::move(command));

        m_redoList.clear();
    }

    void Execute(std::unique_ptr<ICommand> command) {
        command->Execute();
        m_undoList.push_back(std::move(command));

        m_redoList.clear();
    }

    void Undo() {
        if (!m_undoList.empty()) {
            auto command = std::move(m_undoList[m_undoList.size()]);
            m_undoList.pop_back();

            command->Undo();

            m_redoList.push_back(std::move(command));
        }
    }

    void Redo() {
        if (!m_redoList.empty()) {
            auto command = std::move(m_redoList[m_redoList.size()]);
            m_redoList.pop_back();

            command->Execute();

            m_undoList.push_back(std::move(command));
        }
    }

   private:
    Scene& m_scene;

    std::vector<std::unique_ptr<ICommand>> m_undoList{};
    std::vector<std::unique_ptr<ICommand>> m_redoList{};
};
}  // namespace para