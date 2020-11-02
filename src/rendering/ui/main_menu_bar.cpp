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

            std::string str = "Current project: ";
            str += project->isSaved() ? project->getName() : project->getName() + "*";
            ImGui::Text(str.c_str());
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
    if (ImGui::MenuItem("WOW", "Baby")) {
        jobFct job = [=](float &progress, bool &abort) -> std::shared_ptr<JobResult> {
            std::cout << "Executing job" << std::endl;
            return std::make_shared<DummyResult>("lol");
        };
        jobResultFct result_fct = [=] (const std::shared_ptr<JobResult>& result) {
            auto res = std::dynamic_pointer_cast<DummyResult>(result);
            std::cout << "Hello from result " << res->data.str << std::endl;
        };
        JobScheduler::getInstance().addJob("test", job, result_fct);
    }
    if (ImGui::MenuItem("Open project", Shortcuts::open_project_shortcut.description)) {
        open_file();
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        auto recent_projects = Settings::getInstance().getRecentFiles();
        if (recent_projects.empty()) {
            ImGui::MenuItem("No recent projects", NULL, false, false);
        }
        else {
            for(auto &filename : recent_projects) {
                if (ImGui::MenuItem(filename.c_str())) {
                    open_file(filename);
                }
            }
        }
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
            std::string name = prj->isSaved() ? prj->getName() : "*" + prj->getName();
            if (ImGui::MenuItem(name.c_str(), prj->getSaveFile().c_str(), is_active)) {
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
        Modals::getInstance().setModal("Set UI size", [] (bool &show, bool &enter, bool &escape) {
            ImGui::DragInt("Size", &Settings::getInstance().getUIsize(), 1, 50, 200, "%d%%");
            if(ImGui::Button("Reset")) {
                Settings::getInstance().setUIsize(100);
            }
            ImGui::SameLine();
            if(ImGui::Button("Ok") || escape || enter) {
                Settings::getInstance().saveSettings();
                show = false;
            }
            }
            , ImGuiWindowFlags_AlwaysAutoResize);
    }
}

void Rendering::MainMenuBar::open_file(std::string filename) {
    if (filename.empty()) {
        nfdchar_t *outPath;
        nfdfilteritem_t filterItem[1] = { { "Project", STRING(PROJECT_EXTENSION) } };
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
        if (result == NFD_ERROR) {
            show_error_modal("Load project error",
                             "Could not open project");
            return;
        }
        else if (result == NFD_CANCEL) {
            return;
        }
        filename = outPath;
    }
    try {
        project_manager_.openProjectFromFile(filename);
        Settings::getInstance().addRecentFile(filename);
    }
    catch (std::exception &e) {
        show_error_modal("Load project error",
                         "An error occured when loading the project ''\n",
                         e.what());
    }
}

void Rendering::MainMenuBar::init_listeners() {
    auto shortcuts_listener = new Listener;
    shortcuts_listener->filter = "shortcuts/global/*";
    shortcuts_listener->callback = [this] (Event_ptr &event) {
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
        nfdchar_t *outPath = NULL;
        nfdfilteritem_t filterItem[1] = { { "Project", STRING(PROJECT_EXTENSION) } };
        bool proceed = false;
        if (project->getSaveFile().empty()) {
            nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, NULL, NULL);
            if (result == NFD_OKAY) {
                out_path = outPath;
                out_path += ".ml_proj";
                proceed = true;
            }
        }
        else {
            proceed = true;
            out_path = project->getSaveFile();
        }

        if (proceed) {
            save(project, outPath);
        }
    }
}
void Rendering::MainMenuBar::save_project_under() {
    auto project = project_manager_.getCurrentProject();
    if (project != NULL) {
        nfdchar_t *outPath = NULL;
        nfdfilteritem_t filterItem[1] = { { "Project", STRING(PROJECT_EXTENSION) } };
        nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, NULL, NULL);
        if (result == NFD_OKAY) {
            project = project_manager_.duplicateCurrentProject();
            save(project, std::string(outPath) + STRING(PROJECT_EXTENSION));
        }
    }
}

void Rendering::MainMenuBar::save(Project *project, std::string filename) {
    bool result = project_manager_.saveProjectToFile(project, filename);
    if (!result) {
        error_msg = "Error when saving '" + project->getName() + "', please try again";
        Modals::getInstance().setModal("Error",  error_fct, ImGuiWindowFlags_AlwaysAutoResize);
    }
    else {
        Settings::getInstance().addRecentFile(filename);
    }
}
