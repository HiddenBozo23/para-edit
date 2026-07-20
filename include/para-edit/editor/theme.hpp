#pragma once

#include <imgui.h>

namespace para::theme {
constexpr ImVec4 windowColor = ImVec4(0, 0, 0, 1);
constexpr ImVec4 textColor = ImVec4(1, 1, 1, 1);
constexpr ImVec4 accentColor = ImVec4(1, 1, 0, 1);

constexpr ImVec4 debugColor = ImVec4(1, 1, 1, 1);
constexpr ImVec4 infoColor = ImVec4(0.5, 0.5, 0.5, 1);
constexpr ImVec4 warningColor = ImVec4(1, 1, 0, 1);
constexpr ImVec4 errorColor = ImVec4(1, 0.5, 0.5, 1);
constexpr ImVec4 fatalColor = ImVec4(1, 0, 0, 1);
}  // namespace para::theme