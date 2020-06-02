#ifndef BM_SEGMENTER_SHORTCUTS_LIST_H
#define BM_SEGMENTER_SHORTCUTS_LIST_H

#include "rendering/keyboard_shortcuts.h"

void init_global_shortcuts() {
    Shortcut new_project({.keys = {KEY_CTRL, KEY_SHIFT, GLFW_KEY_N},
                          .description = "new project"});
    KeyboardShortCut::addShortcut(new_project);

    Shortcut save_project({.keys = {KEY_CTRL, GLFW_KEY_S},
                                 .description = "save project"});
    KeyboardShortCut::addShortcut(save_project);

    Shortcut save_project_under({.keys = {KEY_CTRL, KEY_SHIFT, GLFW_KEY_S},
                                  .description = "save project under"});
    KeyboardShortCut::addShortcut(save_project_under);

    Shortcut open_project({.keys = {KEY_CTRL, GLFW_KEY_O},
                                  .description = "open project"});
    KeyboardShortCut::addShortcut(open_project);

    Shortcut undo({.keys = {KEY_CTRL, GLFW_KEY_Z},
                                  .description = "undo"});
    KeyboardShortCut::addShortcut(undo);

    Shortcut redo({.keys = {KEY_CTRL, GLFW_KEY_Y},
                                  .description = "redo"});
    KeyboardShortCut::addShortcut(open_project);
}

#endif //BM_SEGMENTER_SHORTCUTS_LIST_H
