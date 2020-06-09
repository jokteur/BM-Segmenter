#include "error_message.h"

void Rendering::show_error_modal(const char *title, const char *short_descr, const char *detailed, float size_x, float size_y)  {
    static std::string last_title;
    static bool new_error_modal;
    static bool show_details;
    if (last_title != title) {
        last_title = title;
        new_error_modal = true;
        show_details = false;
    }
    else {
        new_error_modal = false;
    }

    const modal_fct error_fct = [title, short_descr, detailed, size_x, size_y] (bool &show, bool &enter, bool &escape) {
        ImGui::TextWrapped(short_descr);

        if (new_error_modal) {
            ImGui::SetWindowSize(ImVec2(size_x, size_y));
            new_error_modal = false;
            show_details = false;
        }

        ImGui::NewLine();
        if(ImGui::Button("Ok") || enter || escape)
            show = false;

        if (detailed[0] != 0) {
            ImGui::SameLine();
            if (ImGui::Button("Show details")) {
                show_details = !show_details;
            }

            if (show_details) {
                ImGui::Text("Error details:");
                ImGui::TextWrapped(detailed);
            }
        }
    };

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 200), ImVec2(FLT_MAX, FLT_MAX));
    Modals::getInstance().stackModal(title, error_fct);
};