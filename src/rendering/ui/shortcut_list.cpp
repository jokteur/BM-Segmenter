#include "shortcuts_list.h"

namespace Shortcuts {
    Shortcut new_project_shortcut;

    Shortcut save_project_under_shortcut;

    Shortcut save_project_shortcut;

    Shortcut open_project_shortcut;

    void init_global_shortcuts() {
        new_project_shortcut.keys = {CMD_KEY, KEY_SHIFT, GLFW_KEY_N};
        new_project_shortcut.name = "new project";
        new_project_shortcut.description = STRING(CMD_DESCR) "+Shift+N";
        KeyboardShortCut::addShortcut(new_project_shortcut);

        save_project_under_shortcut.keys = {CMD_KEY, KEY_SHIFT, GLFW_KEY_S};
        save_project_under_shortcut.name = "save project under";
        save_project_under_shortcut.description = STRING(CMD_DESCR) "+Shift+S";
        KeyboardShortCut::addShortcut(save_project_under_shortcut);

        save_project_shortcut.keys = {CMD_KEY, GLFW_KEY_S};
        save_project_shortcut.name = "save project";
        save_project_shortcut.description = STRING(CMD_DESCR) "+S";
        KeyboardShortCut::addShortcut(save_project_shortcut);

        open_project_shortcut.keys = {CMD_KEY, GLFW_KEY_O};
        open_project_shortcut.name = "open project";
        open_project_shortcut.description = STRING(CMD_DESCR) "+O";
        KeyboardShortCut::addShortcut(open_project_shortcut);
    }
}