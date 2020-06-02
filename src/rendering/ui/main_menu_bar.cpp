#include "main_menu_bar.h"

#include "rendering/ui/modales/modals.h"

void Rendering::MainMenuBar::ImGuiDraw(GLFWwindow *window, Rect &parent_dimension) {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            file_menu();
            ImGui::EndMenu();
        }

        /*if (ImGui::BeginMenu("Edit")) {
            //edit_menu();
            ImGui::EndMenu();
        }*/

        if (ImGui::BeginMenu("Projects")) {
            projects_menu();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Settings")) {
            settings_menu();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Rendering::MainMenuBar::file_menu()  {
    if (ImGui::MenuItem("New project", "Ctrl+Shift+N")) {
        new_project_modal_.showModal();
    }
    if (ImGui::MenuItem("Open project", "Ctrl+O")) {
        open_file();
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("prj1.c");
        ImGui::MenuItem("prj2.inl");
        ImGui::MenuItem("prj3.h");
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {
        save_project();
    }
    if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S")) {
        save_project();
    }
}

void Rendering::MainMenuBar::projects_menu() {
    if (project_manager_.getNumProjects() > 0) {
        for(auto &prj : project_manager_) {
            bool is_active = project_manager_.getCurrentProject() == prj;
            if (ImGui::MenuItem(prj->getName().c_str(), "", is_active)) {}
        }
    }
    else
        ImGui::MenuItem("No opened project", "", false, false);
}

/*
void Rendering::MainMenuBar::edit_menu() {
    if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}
}*/

void Rendering::MainMenuBar::settings_menu(){
    if (ImGui::BeginMenu("Theme")) {
        bool is_dark_theme = settings_.getCurrentTheme() == Settings::SETTINGS_DARK_THEME;
        if (ImGui::MenuItem("Dark", "", is_dark_theme)) {
            settings_.setTheme(Settings::SETTINGS_DARK_THEME);
        }
        if (ImGui::MenuItem("Light", "", !is_dark_theme)) {
            settings_.setTheme(Settings::SETTINGS_LIGHT_THEME);
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Set UI size")) {
        Modals::getInstance().setModal("Set UI size", [] (bool &show) {
                if(ImGui::Button("TEST")) {
                    show = false;
                }
            }
            , ImGuiWindowFlags_AlwaysAutoResize);
    }
}

void Rendering::MainMenuBar::open_file() {
    nfdresult_t result = NFD_OpenDialog( "ml_prj", NULL, &outPath );
    if (result == NFD_OKAY) {
        // Do something
    }
}

void Rendering::MainMenuBar::init_listeners() {
    auto shortcuts_listener = new Listener{
        .filter = "shortcuts/global/*",
        .callback = [this] (Event_ptr &event) {
            std::string name = event->getName();

            if (name == "shortcuts/global/new project") {
                new_project_modal_.showModal();
            }
            else if (name == "shortcuts/global/open project") {
                open_file();
            }
            else if (name == "shortcuts/global/save project under") {
                save_project();
            }
            else if (name == "shortcuts/global/save project") {
                save_project();
            }
            else if (name == "shortcuts/global/undo") {
                //undo();
            }
            else if (name == "shortcuts/global/redo") {
                //redo();
            }
        }
    };
    event_queue_.subscribe(shortcuts_listener);
    listeners_.push_back(shortcuts_listener);
}

void Rendering::MainMenuBar::destroy_listeners() {
    for (auto & listener : listeners_) {
        event_queue_.unsubscribe(listener);
        delete listener;
    }
}

void Rendering::MainMenuBar::save_project() {

}