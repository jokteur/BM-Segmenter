#include "error_message.h"

void Rendering::show_error_modal(std::string title, std::string short_descr, std::string detailed)  {
    static std::string last_title;
    static bool redraw_modal = true;
    static bool show_details;
    if (last_title != title) {
        last_title = title;
        show_details = false;
    }

    const modal_fct error_fct = [title, short_descr, detailed] (bool &show, bool &enter, bool &escape) {
        ImGui::Text(short_descr.c_str());

        float width = ImGui::GetItemRectSize().x;
        float height = ImGui::GetItemRectSize().y;

        ImGuiStyle& style = ImGui::GetStyle();


        width += style.ScrollbarSize + style.ItemSpacing.x + style.WindowPadding.x;
        height += style.WindowPadding.y;

        if (!detailed.empty()) {
            ImGui::NewLine();
        }

        height += ImGui::GetItemRectSize().y;
        if(ImGui::Button("Ok") || enter || escape)
            show = false;

        // Show detailed description
        height += ImGui::GetItemRectSize().y;

        if (!detailed.empty()) {
            ImGui::SameLine();
            height += ImGui::GetItemRectSize().y;
            if (ImGui::Button("Show details")) {
                redraw_modal = true;
                show_details = !show_details;
            }

            if (show_details) {
                ImGui::Text("Error details:");
                //ImGui::PushFont();
                height += ImGui::GetItemRectSize().y;
                ImGui::TextWrapped(detailed.c_str());
                height += ImGui::GetItemRectSize().y;
            }
        }

        // Set the window size if the modal was just opened
        if (redraw_modal) {
            width = (width > 1000) ? 1000 : width;
            height = (height > 1000) ? 1000 : height;
            ImGui::SetWindowSize(ImVec2(width, height));
            redraw_modal = false;
        }

    };

    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 200), ImVec2(FLT_MAX, FLT_MAX));
    Modals::getInstance().stackModal(title, error_fct);
};