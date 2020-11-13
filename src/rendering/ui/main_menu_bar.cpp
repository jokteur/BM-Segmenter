#include "main_menu_bar.h"

#include "rendering/ui/modales/modals.h"
#include "rendering/ui/shortcuts_list.h"
#include "gui.h"
#include "views/project_view.h"
#include "views/default_view.h"

namespace project = ::core::project;

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
        if (!page_title_.empty()) {
            ImGui::SameLine((ImGui::GetWindowWidth() - 200.f) / 2);
            ImGui::TextDisabled(page_title_.c_str());
        }

        std::shared_ptr<project::Project> project = project_manager_.getCurrentProject();
        if (project != nullptr) {
            // Try to align a Text element to the right
            const float item_spacing = ImGui::GetStyle().ItemSpacing.x;
            static float text_width = 200.0f;
            float pos = text_width + item_spacing + 20.f;
            ImGui::SameLine(ImGui::GetWindowWidth() - pos);

            std::string str = "Current project: ";
            str += project->isSaved() ? project->getName() : project->getName() + "*";
            ImGui::Text("%s", str.c_str());
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
    if (close_projects_ && project_manager_.getNumProjects() == 0) {
        EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<DefaultView>())));
        close_projects_ = false;
        num_projects = 0;
    }
    else if (close_projects_) {
        if (num_projects != project_manager_.getNumProjects())  {
            for (auto& prj : project_manager_) {
                close_project_modal_.setProject(prj);
                break;
            }
            num_projects = project_manager_.getNumProjects();
        }
        if (close_projects_) {
            close_project_modal_.showModal();
        }
    }
}

void Rendering::MainMenuBar::file_menu()  {
    bool is_project_active = project_manager_.getCurrentProject() != nullptr;

    if (ImGui::MenuItem("New project", Shortcuts::new_project_shortcut.description)) {
        new_project_modal_.showModal();
    }
    if (ImGui::MenuItem("Open project", Shortcuts::open_project_shortcut.description)) {
        open_file();
    }
    if (ImGui::BeginMenu("Open Recent"))
    {
        auto recent_projects = Settings::getInstance().getRecentFiles();
        if (recent_projects.empty()) {
            ImGui::MenuItem("No recent projects", nullptr, false, false);
        }
        else {
            for(auto &filename : recent_projects) {
                if (ImGui::MenuItem(filename.c_str())) {
                    open_file(filename);
                    EventQueue::getInstance().post(Event_ptr(new SetViewEvent(std::make_unique<ProjectView>())));
                }
            }
        }
        ImGui::EndMenu();
    }
    if (project_manager_.getNumProjects() > 0) {
        if (ImGui::MenuItem("Close all projects")) {
            close_projects_ = true;
        }
    }

    if (ImGui::MenuItem("Save", Shortcuts::save_project_shortcut.description, false, is_project_active)) {
        save_project();
    }
    //if (ImGui::MenuItem("Save As..", Shortcuts::save_project_under_shortcut.description, false, is_project_active)) {
    //    save_project_under();
    //}
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
        },
        ImGuiWindowFlags_AlwaysAutoResize);
    }
}

void Rendering::MainMenuBar::open_file(std::string filename) {
    if (filename.empty()) {
        nfdchar_t *outPath;
        nfdfilteritem_t filterItem[1] = { { "Project", STRING(PROJECT_EXTENSION) } };
        nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, nullptr);
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
        auto project = project_manager_.openProjectFromFile(filename);
        project_manager_.setCurrentProject(project);
        Settings::getInstance().addRecentFile(filename);

    }
    catch (std::exception &e) {
        show_error_modal("Load project error",
                         "An error occured when loading the project ''\n",
                         e.what());
    }
}

Rendering::MainMenuBar::MainMenuBar()
    : project_manager_(::core::project::ProjectManager::getInstance()),
    event_queue_(EventQueue::getInstance()),
    settings_(Settings::getInstance()) {

    error_fct = [this](bool& show, bool& escape, bool& enter) {
        ImGui::Text("%s", error_msg.c_str());
        if (ImGui::Button("Ok") || escape || enter)
            show = false;
    };
    init_listeners();
}

void Rendering::MainMenuBar::init_listeners() {
    shortcuts_listener_.filter = "shortcuts/global/*";
    shortcuts_listener_.callback = [this] (Event_ptr &event) {
        std::string name = event->getName();

        if (name == "shortcuts/global/new project") {
            new_project_modal_.showModal();
        }
        else if (name == "shortcuts/global/open project") {
            open_file();
        }
        else if (name == "shortcuts/global/save project under") {
            //save_project_under();
        }
        else if (name == "shortcuts/global/save project") {
            save_project();
        }
    };

    page_title_change_.filter = "menu/change_title/*";
    page_title_change_.callback = [this](Event_ptr& event) {
        page_title_ = event->getName().substr(18);
    };

    event_queue_.subscribe(&shortcuts_listener_);
    event_queue_.subscribe(&page_title_change_);
}

void Rendering::MainMenuBar::destroy_listeners() {
    event_queue_.unsubscribe(&shortcuts_listener_);
    event_queue_.unsubscribe(&page_title_change_);
}

void Rendering::MainMenuBar::save_project() {
    auto project = project_manager_.getCurrentProject();
    if (project != nullptr) {
        std::string out_path, str_err;
        NFD_Init();
        nfdchar_t* outPath;
        bool proceed = false;
        if (project->getSaveFile().empty()) {
            nfdresult_t result = NFD_PickFolder(&outPath, nullptr);
            if (result == NFD_OKAY) {
                out_path = outPath;
                proceed = project->setUpWorkspace(out_path, project->getName(), STRING(PROJECT_EXTENSION), out_path);
            }
        }
        else {
            proceed = true;
            out_path = project->getSaveFile();
        }

        if (proceed) {
            save(project, out_path);
        }
        else {
            show_error_modal("Could not save project",
                "The program failed to set up the workspace at the given path",
                str_err);
        }
        NFD_Quit();
    }
}
void Rendering::MainMenuBar::save_project_under() {
    auto project = project_manager_.getCurrentProject();
    if (project != nullptr) {
        nfdchar_t *outPath = nullptr;
        nfdfilteritem_t filterItem[1] = { { "Project"} };
        nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 1, nullptr, nullptr);
        if (result == NFD_OKAY) {
            project = project_manager_.duplicateCurrentProject();
            save(project, std::string(outPath));
        }
    }
}

void Rendering::MainMenuBar::save(const std::shared_ptr<project::Project>& project, const std::string& filename) {
    bool result = project_manager_.saveProjectToFile(project, filename);
    if (!result) {
        error_msg = "Error when saving '" + project->getName() + "', please try again";
        Modals::getInstance().setModal("Error",  error_fct, ImGuiWindowFlags_AlwaysAutoResize);
    }
    else {
        Settings::getInstance().addRecentFile(filename);
    }
}
