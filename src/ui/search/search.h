#pragma once

#include <unordered_map>
#include <optional>
#include <string>
#include <chrono>
#include <set>

namespace Search {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    typedef std::chrono::high_resolution_clock pclock;
    typedef std::chrono::time_point<std::chrono::steady_clock> tp;

    const int TIMEOUT = 200;
    enum Type {
        WINDOW,
        MENU,
        MENUITEM,
        HEADER,
        BUTTON,
        TREE,
        TREENODE
    };
    struct Node {
        std::string parent;
        Type type;
        bool show;
        tp last_time;
    };

    class Universe {
    private:
        std::unordered_map<std::string, Node> m_search_tree;
        std::string m_show_id;
    public:
        Universe();

        /**
         * @brief Returns if the object must be shown or not
         *
         * @param id id of the object
         * @return std::optional<bool> empty if must not be shown
         * #return if bool, then true indicates that the node is the last node to be shown
         */
        std::optional<bool> getShow(const std::string& id);

        /**
         * @brief Registers a node in the search Tree
         *
         * @param id name of the node (must be unique)
         * @param type type of the node (window, menu, menu item, ...)
         * @param parent parent of the node if there is one
         */
        void Register(const std::string& id, Type type, const std::string& parent = "");

        void Clear();

        void StopShow(const std::string& id);

        void StopAllShow();

        void Show(const std::string& id);
    };
}