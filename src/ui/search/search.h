#pragma once

#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include <chrono>
#include <map>
#include <set>
#include <unordered_set>

namespace Search {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    typedef std::chrono::high_resolution_clock pclock;
    typedef std::chrono::time_point<std::chrono::steady_clock> tp;

    struct Node {
        std::string parent;
        std::unordered_set<std::string> childrens;
        std::vector<std::string> search_phrases;
        bool show;
        tp last_time;
    };

    struct Result {
        std::string widget_name;
        std::string description;
    };

    class Universe {
    private:
        std::unordered_map<std::string, Node> m_widget_tree; // id -> Node
        std::string m_show_id;
        std::unordered_map<std::string, std::vector<std::string>> m_search_phrases; // Phrase -> id
        std::set<std::string> m_search_words;
        std::map<std::string, std::vector<std::string>> m_word_to_phrase; // word -> Phrase

        bool m_frame_taken = false;

        std::unordered_set<std::string> m_widgets_to_remove;

        std::string m_split_char = " ";

        void rebuild_words();
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
        void Register(const std::string& id, const std::string& parent = "", const std::vector<std::string>& search_terms = {});

        /**
         * @brief Removes a node from the search Tree (and consequently all the children)
         *
         * @param id name of the node
         */
        void Unregister(const std::string& id, int level = 0);

        /**
         * @brief Clear all search terms (mainly used to change language)
         *
         */
        void Clear();

        /**
         * @brief Signals the specified ID that is must not be highlighted anymore
         *
         * @param id
         */
        void StopHighlight(const std::string& id);

        void StopAllHighlights();

        /**
         * @brief Highlight a widget with its ID
         *
         * @param id
         */
        void Highlight(const std::string& id);

        /**
         * @brief Set split character for separating words in the search
         *
         * @param split
         */
        void SetSplitChar(const std::string& split);

        /**
         * @brief The widgets are shown one by one
         * Each frame, a widget is shown, indicating other widget that they must wait their turn
         */
        void FreeFrame() { m_frame_taken = false; }

        /**
         * @brief Returns the weights of the search result, along with the search term and the widget id's
         */
        std::map<int, std::vector<Result>> getSearchResults(const std::string& query, double score_cutoff = 0.0);
    };
}