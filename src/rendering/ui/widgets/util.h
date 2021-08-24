#pragma once
#include "imgui.h"
#include "rendering/drawables.h"

#include <set>
#include <functional>

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

        static void NewLine(float size = 0.f) {
            if (size == 0.f) {
                ImGui::NewLine();
            }
            else {
                ImGui::Dummy(ImVec2(0.f, size));
            }
        }

        /**
         * Checks if the mouse is inside the rectangle
         * @param mouse_position
         * @param dimensions
         * @return
         */
        static bool check_hitbox(const ImVec2 &mouse_position, const Rect &dimensions) {
            return mouse_position.x > dimensions.xpos && mouse_position.x < dimensions.xpos + dimensions.width
                   && mouse_position.y > dimensions.ypos && mouse_position.y < dimensions.ypos + dimensions.height;
        }

        class Selectable {
        private:
            std::function<void(std::string)> str_fct_;
            std::function<void(int idx)> idx_fct_;

            std::vector<std::string> options_;
            std::vector<ImVec4> colors_;
            int which_ = -1;
            int idx_ = 0;
            int prev_idx_ = 0;

            void build(std::vector<std::string> options);
        public:
            Selectable() {}
            Selectable(const std::vector<std::string>& options, std::function<void(std::string)> on_select, const std::vector<ImVec4>& colors = {});
            Selectable(const std::vector<std::string>& options, std::function<void(int)> on_select, const std::vector<ImVec4>& colors = {});

            void setOptions(const std::vector<std::string>& options, std::function<void(std::string)> on_select, const std::vector<ImVec4>& colors = {});
            void setOptions(const std::set<std::string>& options, std::function<void(std::string)> on_select, const std::vector<ImVec4>& colors = {});
            void setOptions(std::vector<std::string> options, std::function<void(int)> on_select, const std::vector<ImVec4>& colors = {});
            void setOptions(std::set<std::string> options, std::function<void(int)> on_select, const std::vector<ImVec4>& colors = {});

            int getCurrentIdx() { return idx_; }
            std::string getCurrentOption() { return options_[idx_]; }

            void setIdx(int idx);

            int size() { return options_.size(); }

            void ImGuiDraw(const std::string& label, float size = 0);
        };
    }
}