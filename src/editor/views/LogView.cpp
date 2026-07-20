#include "para-edit/editor/views/LogView.hpp"

#include "para-edit/core/Logger.hpp"
#include "para-edit/editor/theme.hpp"

#include <imgui.h>

namespace para {
void LogView::OnRender() {
    ImGui::Begin("Log");

    ImGui::BeginChild("LogScrollRegion",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    auto logs = Logger::GetLogs();
    ImVec4 colour;
    std::string text;

    for (auto log : logs) {
        if (m_filterMask & log.level) {
            switch (log.level) {
                case Logger::LogLevel::debug:
                    colour = theme::debugColor;
                    break;
                case Logger::LogLevel::info:
                    colour = theme::infoColor;
                    break;
                case Logger::LogLevel::warning:
                    colour = theme::warningColor;
                    break;
                case Logger::LogLevel::error:
                    colour = theme::errorColor;
                    break;
                case Logger::LogLevel::fatal:
                    colour = theme::fatalColor;
                    break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, colour);

            text = log.msg;
            if (m_verbose) {
                text = log.timestamp + "::" + text + "::" + log.file + " LINE " + std::to_string(log.line);
            }
            ImGui::TextWrapped("%s", text.c_str());

            ImGui::PopStyleColor(1);
        }
    }
    ImGui::EndChild();

    if (ImGui::Button("Clear")) {
        Logger::ClearLogs();
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(ImGui::CalcTextSize("filter").x + 40.0f);
    if (ImGui::BeginCombo(" ", "Filter")) {
        ImGui::CheckboxFlags("debug", &m_filterMask, Logger::LogLevel::debug);
        ImGui::CheckboxFlags("info", &m_filterMask, Logger::LogLevel::info);
        ImGui::CheckboxFlags("warning", &m_filterMask, Logger::LogLevel::warning);
        ImGui::CheckboxFlags("error", &m_filterMask, Logger::LogLevel::error);
        ImGui::CheckboxFlags("fatal", &m_filterMask, Logger::LogLevel::fatal);
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    ImGui::Checkbox("verbose", &m_verbose);

    ImGui::End();
}
}  // namespace para