#include "shortcuts_list.h"


namespace Shortcuts {
    Shortcut new_project_shortcut ({.keys = {CMD_KEY, KEY_SHIFT, GLFW_KEY_N},
                                    .name = "new project",
                                    .description = STRING(CMD_DESCR) "+Shift+N"});

    Shortcut save_project_shortcut({.keys = {CMD_KEY, GLFW_KEY_S},
                                    .name = "save project",
                                    .description = STRING(CMD_DESCR) "+S"});

    Shortcut save_project_under_shortcut({.keys = {CMD_KEY, KEY_SHIFT, GLFW_KEY_S},
                                          .name = "save project under",
                                          .description = STRING(CMD_DESCR) "+Shift+S"});

    Shortcut open_project_shortcut({.keys = {CMD_KEY, GLFW_KEY_O},
                                    .name = "open project",
                                    .description = STRING(CMD_DESCR) "+O"});

    void init_global_shortcuts() {
        KeyboardShortCut::addShortcut(new_project_shortcut);

        KeyboardShortCut::addShortcut(save_project_shortcut);

        KeyboardShortCut::addShortcut(save_project_under_shortcut);

        KeyboardShortCut::addShortcut(open_project_shortcut);
    }
}