#include "search.h"
#include <tempo.h>

namespace Search {
    Universe::Universe() {}

    void Universe::Register(const std::string& id, const std::string& parent) {
        m_search_tree[id] = Node{ parent, false };
    }

    std::optional<bool> Universe::getShow(const std::string& id) {
        if (!m_search_tree.contains(id))
            return std::optional<bool>();
        auto& node = m_search_tree[id];
        if (node.show) {
            if (m_show_id == id) {
                return std::optional<bool>(true);
            }
            else {
                node.show = false;
                auto now = pclock::now();
                if (duration_cast<milliseconds>(now - node.last_time).count() > TIMEOUT)
                    return std::optional<bool>();

                return std::optional<bool>(false);
            }
        }
        return std::optional<bool>();
    }

    void Universe::StopShow(const std::string& id) {
        if (!m_search_tree.contains(id))
            return;

        m_search_tree[id].show = false;
        m_show_id = "";
    }

    void Universe::StopAllShow() {
        for (auto& pair : m_search_tree) {
            pair.second.show = false;
        }
        m_show_id = "";
    }

    void Universe::Show(const std::string& id) {
        if (!m_search_tree.contains(id))
            return;
        m_show_id = id;
        Node* current_node = &m_search_tree[id];
        current_node->show = true;
        current_node->last_time = pclock::now();

        Tempo::PollUntil(TIMEOUT + 100);

        int internal_counter = 0;
        while (current_node->parent != "") {
            std::string& parent = current_node->parent;
            if (m_search_tree.contains(parent))
                current_node = &m_search_tree[parent];
            else
                break;

            internal_counter++;
            current_node->show = true;
            current_node->last_time = pclock::now();

            // Means a cycle in parenting has been created
            if (internal_counter > 1000)
                break;
        }
    }

    void Universe::Clear() {
        m_search_tree.clear();
    }
}