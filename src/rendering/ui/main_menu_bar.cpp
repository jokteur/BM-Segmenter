#include "main_menu_bar.h"

#include "rendering/ui/modales/modals.h"
#include "rendering/ui/shortcuts_list.h"

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

        Project* project = project_manager_.getCurrentProject();
        if (project != NULL) {
            // Try to align a Text element to the right
            const float item_spacing = ImGui::GetStyle().ItemSpacing.x;
            static float text_width = 200.0f;
            float pos = text_width + item_spacing + 20.f;
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);

            ImGui::Text((std::string("Current project: ") + project->getName()).c_str());
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                //ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                ImGui::TextUnformatted(project->getDescription().c_str());
                //ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
            text_width = ImGui::GetItemRectSize().x;


        }
        ImGui::EndMainMenuBar();
    }
}

void Rendering::MainMenuBar::file_menu()  {
    bool is_project_active = project_manager_.getCurrentProject() != NULL;

    if (ImGui::MenuItem("New project", Shortcuts::new_project_shortcut.description)) {
        new_project_modal_.showModal();
    }
    if (ImGui::MenuItem("Open project", Shortcuts::open_project_shortcut.description)) {
        open_file();
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("prj1.c");
        ImGui::MenuItem("prj2.inl");
        ImGui::MenuItem("prj3.h");
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", Shortcuts::save_project_shortcut.description, false, is_project_active)) {
        save_project();
    }
    if (ImGui::MenuItem("Save As..", Shortcuts::save_project_under_shortcut.description, false, is_project_active)) {
        save_project_under();
    }
}

void Rendering::MainMenuBar::projects_menu() {
    if (project_manager_.getNumProjects() > 0) {
        for(auto &prj : project_manager_) {
            bool is_active = project_manager_.getCurrentProject() == prj;
            if (ImGui::MenuItem(prj->getName().c_str(), prj->getSaveFile().c_str(), is_active)) {
                project_manager_.setCurrentProject(prj);
            }
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
            settings_.setStyle(Settings::SETTINGS_DARK_THEME);
        }
        if (ImGui::MenuItem("Light", "", !is_dark_theme)) {
            settings_.setStyle(Settings::SETTINGS_LIGHT_THEME);
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Set UI size")) {
        Modals::getInstance().setModal("Set UI size", [] (bool &show) {
            ImGui::DragInt("Size", &Settings::getInstance().getUIsize(), 1, 50, 200, "%d%%");
            if(ImGui::Button("Reset")) {
                Settings::getInstance().setUIsize(100);
            }
            ImGui::SameLine();
            if(ImGui::Button("Ok")) {
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
                save_project_under();
            }
            else if (name == "shortcuts/global/save project") {
                save_project();
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
    auto project = project_manager_.getCurrentProject();
    if (project != NULL) {
        std::string out_path;
        bool save = false;
        if (project->getSaveFile().empty()) {
            nfdresult_t result = NFD_SaveDialog("ml_prj", NULL, &outPath);
            if (result == NFD_OKAY) {
                out_path = outPath;
                save = true;
            }
        }
        else {
            save = true;
            out_path = project->getSaveFile();
        }

        if (save) {
            bool result = project_manager_.saveProjectToFile(project, out_path);
            if (!result) {
                error_msg = "Error when saving '" + project->getName() + "', please try again";
                Modals::getInstance().setModal("Error",  error_fct, ImGuiWindowFlags_AlwaysAutoResize);
            }
        }
    }
}
void Rendering::MainMenuBar::save_project_under() {
    auto project = project_manager_.duplicateCurrentProject();
    if (project != NULL) {
        nfdresult_t result = NFD_SaveDialog("ml_prj", NULL, &outPath);
        if (result == NFD_OKAY) {
            project_manager_.saveProjectToFile(project, outPath);
            error_msg = "Error when saving '" + project->getName() + "', please try again";
            Modals::getInstance().setModal("Error",  error_fct, ImGuiWindowFlags_AlwaysAutoResize);
        }
    }
}