#ifndef BM_SEGMENTER_KEYBOARD_SHORTCUTS_H
#define BM_SEGMENTER_KEYBOARD_SHORTCUTS_H

#include <vector>
#include <queue>
#include <initializer_list>
#include <GLFW/glfw3.h>
#include <map>
#include <chrono>
#include <mutex>
#include <string>

#include "events.h"
#include "imgui.h"

#ifndef KEYBOARD_SHORTCUT_QUEUE_LENGTH
#define KEYBOARD_SHORTCUT_QUEUE_LENGTH 5
#endif

using keyboard_event = int;
using timepoint = std::chrono::system_clock::time_point;

enum {KEY_CTRL = 1 << 10,
        KEY_ALT = (1 << 10) + 1,
        KEY_SHIFT = (1 << 10) + 2,
        KEY_SUPER = (1 << 10) + 3};

struct Shortcut {
    std::multiset<keyboard_event> keys;
    std::string description;

    // If delay == 0, then the shortcut can only be accomplished if
    // all key in a GLFW_PRESSED state
    // Otherwise, then shortcut can still be accomplished even if
    // some keys are not GLFW_PRESSED
    // delay in [ms]
    float delay = 0;

    std::multiset<keyboard_event> tmp_keys;
};

struct KeyEvent {
    int key;
    int state;
    timepoint time;
};

// Assumptions : can only do one shortcut at a time
// Local, global shortcuts
class KeyboardShortCut {
private:
    static std::mutex mutex;
    static std::deque<KeyEvent> keyboard_events_;
    static std::vector<Shortcut> global_shortcuts_;
    static std::vector<Shortcut> local_shortcuts_;
    static EventQueue& eventQueue_;

    static bool ignore_global_shortcuts_;

    static std::set<int> kp_keys_list_;
    //static std::set<char> authorized_chars_;

    static bool is_shortcut_valid(Shortcut& shortcut);
    static std::multiset<keyboard_event>::iterator find_key(Shortcut& shortcut, const int key);
public:
    static int last_keystroke_;
    // GLFW does not understand keyboard layouts
    // If a user pressed the key "z", depending on the keyboard layout,
    // it could send an GLFW_KEY_Z or GLFW_KEY_Y or else.
    // We try to hack key_callback to do the right thing, but
    // still no support for other than A-Z keys

    static int translate_keycode(int key);

    /**
     * For one dispatchShortcuts() call, the list of shortcuts (i.e. not temporary) is not processed
     * Allows to make temporary shortcuts to take control of the listened shortcuts
     */
    static void ignoreNormalShortcuts() { ignore_global_shortcuts_ = true; }

    /**
     * Adds a shortcut to the local list of shortcuts (i.e. a shortcut that is temporary)
     * Each time dispatchShortcuts() is called, the list of temporary shortcuts is emptied
     * Temporary shortcuts have priority of normal shortcuts
     * @param shortcut
     */
    static void addTempShortcut(Shortcut& shortcut);

    /**
     * Adds a shortcut to the global list of shortcuts (always listening to these shortcut)
     * @param shortcut
     */
    static void addShortcut(Shortcut& shortcut);

    /**
     * Key callback to be defined for any newly created window
     *
     * @note character_callback must be called first in order capture
     * the right key stroke
     */
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * Character callback to be defined for any newly created window
     */
    static void character_callback(GLFWwindow* window, unsigned int codepoint);

    /**
     * Empties the queue of keyboard events
     */
    static void emptyKeyEventsQueue();

    /**
     * Processes the queue for global and local shortcuts
     * Should be called once per loop in the main loop
     */
    static void dispatchShortcuts();
};


#endif //BM_SEGMENTER_KEYBOARD_SHORTCUTS_H
