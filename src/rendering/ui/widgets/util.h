#pragma once
#include "imgui.h"

namespace Rendering {
    namespace Widgets {
        /**
         * Displays a little helper marker the user can hover on
         * @param fmt c_str format
         * @param ...
         */
        static void HelpMarker(const char* fmt, ...) {
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                va_list args;
                va_start(args, fmt);
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextV(fmt, args);
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
                va_end(args);
            }
        }
    }
}