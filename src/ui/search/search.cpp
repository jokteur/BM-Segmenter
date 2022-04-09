#include "search.h"
#include "core/str_manip.h"
#include "core/search.h"

#include <tempo.h>

namespace Search {
    Universe::Universe() {}

    const int TIMEOUT = 1000;

    void Universe::rebuild_words() {
        m_search_words.clear();
        m_word_to_phrase.clear();

        for (auto& pair : m_search_phrases) {
            std::string search_term = pair.first;
            for (auto& str : str::split(search_term)) {
                m_search_words.insert(str);
                m_word_to_phrase[str].push_back(search_term);
            }
        }
    }

    void Universe::Register(const std::string& id, const std::string& parent, const std::vector<std::string>& terms) {
        /* In order to overwrite an existing widget id, one must first remove previous
           search phrases in the map such that they are not searchable anymore */
        if (m_widget_tree.contains(id)) {
            for (auto& term : m_widget_tree[id].search_phrases) {
                m_search_phrases.erase(term);
            }
            rebuild_words();
        }

        std::vector<std::string> search_terms;
        for (auto& term : terms) {
            search_terms.push_back(str::strip(term));
        }
        m_widget_tree[id] = Node{ parent, std::unordered_set<std::string>(), search_terms, false };
        std::string search_term;
        for (auto& str : search_terms) {
            search_term = str::strip(str);
            m_search_phrases[search_term].push_back(id);
        }
        for (auto& str : str::split(search_term)) {
            m_search_words.insert(str);
            m_word_to_phrase[str].push_back(search_term);
        }
        if (m_widget_tree.contains(parent)) {
            m_widget_tree[parent].childrens.insert(id);
        }
    }

    void Universe::Unregister(const std::string& id, int level) {
        if (!m_widget_tree.contains(id))
            return;

        if (level == 0) {
            m_widgets_to_remove.clear();

            // Remove reference in parent node if any
            std::string parent = m_widget_tree[id].parent;
            if (m_widget_tree.contains(parent)) {
                m_widget_tree[parent].childrens.erase(id);
            }
        }

        m_widgets_to_remove.insert(id);

        for (auto child : m_widget_tree[id].childrens) {
            Unregister(child, level + 1);
        }

        // Can only modify the map once we are back to the top of the call stack
        if (level == 0) {
            for (auto el : m_widgets_to_remove) {
                for (auto& phrase : m_widget_tree[el].search_phrases) {
                    m_search_phrases.erase(phrase);
                }
                m_widget_tree.erase(el);
            }
            rebuild_words();
        }
    }

    std::optional<bool> Universe::getShow(const std::string& id) {
        if (!m_widget_tree.contains(id))
            return std::optional<bool>();
        auto& node = m_widget_tree[id];
        if (node.show) {
            if (m_show_id == id) {
                return std::optional<bool>(true);
            }
            else if (!m_frame_taken) {
                m_frame_taken = true;
                node.show = false;
                // auto now = pclock::now();
                // if (duration_cast<milliseconds>(now - node.last_time).count() > TIMEOUT)
                //     return std::optional<bool>();

                return std::optional<bool>(false);
            }
        }
        return std::optional<bool>();
    }

    void Universe::StopHighlight(const std::string& id) {
        if (!m_widget_tree.contains(id))
            return;

        m_widget_tree[id].show = false;
        m_show_id = "";
    }

    void Universe::StopAllHighlights() {
        for (auto& pair : m_widget_tree) {
            pair.second.show = false;
        }
        m_show_id = "";
    }

    void Universe::Highlight(const std::string& id) {
        if (!m_widget_tree.contains(id))
            return;
        m_show_id = id;
        Node* current_node = &m_widget_tree[id];
        current_node->show = true;
        current_node->last_time = pclock::now();

        Tempo::PollUntil(TIMEOUT + 100);

        Tempo::PushAnimation("highlight_widget", 1000);

        int internal_counter = 0;
        while (current_node->parent != "") {
            std::string& parent = current_node->parent;
            if (m_widget_tree.contains(parent))
                current_node = &m_widget_tree[parent];
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

    void Universe::SetSplitChar(const std::string& split) {
        m_split_char = split;
    }

    std::map<int, std::vector<Result>> Universe::getSearchResults(const std::string& query, double) {
        std::map<int, std::vector<Result>> results;

        auto query_words = str::split(query, m_split_char);
        std::vector<std::string> results_phrases;
        for (auto& pair : m_search_phrases) {
            results_phrases.push_back(pair.first);
        }
        auto result_scores = ScoreQueryAgainstResults(query_words, results_phrases, m_split_char);

        for (auto pair : result_scores) {
            for (auto result : pair.second) {
                for (auto widget_id : m_search_phrases[result]) {
                    results[pair.first].push_back(Result{ widget_id, result });
                }
            }
        }

        return results;
    }

    void Universe::Clear() {
        m_widget_tree.clear();
        m_search_phrases.clear();
    }
}