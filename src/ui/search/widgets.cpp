#include "widgets.h"

#include <iostream>
#include <tempo.h>
#include <cmath>
#include "imgui_internal.h"

namespace Widgets {

    inline void DrawRect(ImVec2 min, ImVec2 max) {
        float progress = Tempo::GetProgress("highlight_widget");

        const float SIZE = 5.f;
        min = ImVec2(min.x - SIZE * (float)std::sin(progress * 3.14), min.y - SIZE * (float)std::sin(progress * 3.14));
        max = ImVec2(max.x + SIZE * (float)std::sin(progress * 3.14), max.y + SIZE * (float)std::sin(progress * 3.14));
        ImGui::GetForegroundDrawList()->AddRect(min, max, IM_COL32(31, 135, 229, 229), 3.f, 0, 3.f);
    }

    inline void GetAndDrawRect() {
        ImGuiContext* g = ImGui::GetCurrentContext();
        ImRect bb = g->LastItemData.Rect;
        DrawRect(bb.Min, bb.Max);
    }

    bool Begin(Search::Universe& search, const std::string& id, const std::string& name, bool* p_open, ImGuiWindowFlags flags) {
        bool ret = ImGui::Begin(name.c_str(), p_open, flags);
        auto show = search.getShow(id);
        if (show.has_value()) {
            ImGui::SetWindowFocus();
            if (show.value()) {
                ImVec2 pos = ImGui::GetWindowPos();
                ImVec2 min = ImGui::GetWindowContentRegionMin();
                ImVec2 max = ImGui::GetWindowContentRegionMax();
                DrawRect(ImVec2(pos.x + min.x, pos.y + min.y), ImVec2(pos.x + max.x, pos.y + max.y));
            }
        }
        return ret;
    }
    void End() {
        ImGui::End();
    }

    bool BeginMenu(Search::Universe& search, const std::string& id, const std::string& label, bool enabled) {
        bool ret = ImGui::BeginMenu(label.c_str(), enabled);

        auto show = search.getShow(id);
        if (show.has_value()) {
            if (show.value()) {
                GetAndDrawRect();
            }
            else {
                auto im_id = ImGui::GetID(label.c_str());
                ImGui::OpenPopup(im_id, ImGuiPopupFlags_AnyPopup);
            }
        }
        return ret;
    }
    void EndMenu() {
        ImGui::EndMenu();
    }

    bool MenuItem(Search::Universe& search, const std::string& id, const std::string& label, const char* shortcut, bool selected, bool enabled) {
        bool ret = ImGui::MenuItem(label.c_str(), shortcut, selected, enabled);

        auto show = search.getShow(id);
        if (show.has_value() && show.value()) {
            GetAndDrawRect();
        }
        return ret;
    }
}