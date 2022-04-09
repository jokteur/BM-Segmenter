/*
 * A lot of the code source for the string search
 * is inspired from Blender's string search source code
 * https://github.com/blender/blender/blob/594f47ecd2d5367ca936cf6fc6ec8168c2b360d0/source/blender/blenlib/intern/string_search.cc
 */

#include "search.h"
#include "str_manip.h"

#include <climits>

namespace Search {
    typedef std::unordered_map<std::string, std::unordered_set<std::string>> UsedWords;

    int get_shortest_word_index_that_starts_with(const std::string& query_word, const std::string& result, const std::vector<std::string>& result_words, UsedWords& used_words) {
        int best_word_size = INT32_MAX;
        int word_index = -1;

        int i = 0;
        for (auto word : result_words) {
            if (!used_words[result].contains(word) && word.starts_with(query_word)) {
                if (word.size() < best_word_size) {
                    best_word_size = (int)word.size();
                    word_index = i;
                }
            }
            i++;
        }
        return word_index;
    }

    bool match_word_initials(const std::string& query_word, const std::string& result, const std::vector<std::string>& result_words, UsedWords& used_words, std::unordered_set<std::string>& matched_words, int start = 0) {
        int word_index = start;

        int num_matched_chars = 0;

        int query_index = 0;
        int r_char_index = 0;

        /* Nothing has been fully matched */
        if (word_index >= result_words.size()) {
            return false;
        }

        /* For now, this is not UTF-8 friendly */
        while (query_index < query_word.size()) {
            // We are at the end of all available words
            if (word_index >= result_words.size()) {
                break;
            }
            /* Skip words already matched */
            if (used_words[result].contains(result_words[word_index])) {
                word_index++;
                continue;
            }
            if (r_char_index < result_words[word_index].size()) {
                if (query_word[query_index] == result_words[word_index][r_char_index]) {
                    num_matched_chars++;
                    query_index++;
                    r_char_index++;
                    matched_words.insert(result_words[word_index]);
                    continue;
                }
            }

            /* If we arrive here, we must go the next word */
            word_index++;
            r_char_index = 0;

        }

        /* Try again by starting at the next word */
        if (num_matched_chars == query_word.size()) {
            return true;
        }

        return false;
    }

    using namespace rapidfuzz;

    std::map<int, std::vector<std::string>> ScoreQueryAgainstResults(const std::vector<std::string>& query_words, const std::vector<std::string>& results, const std::string& split_char) {
        UsedWords used_words;
        std::unordered_set<std::string> dismiss;
        std::unordered_map<std::string, int> scores;

        for (auto& q_word : query_words) {
            std::string query_word = str::strip(q_word);
            fuzz::CachedRatio scorer(query_word);

            for (auto& result : results) {
                auto result_words = str::split(result, split_char);

                {
                    int word_index = get_shortest_word_index_that_starts_with(query_word, result, result_words, used_words);
                    if (word_index >= 0) {
                        used_words[result].insert(result_words[word_index]);
                        scores[result] += 300;
                        continue;
                    }
                }
                {
                    std::unordered_set<std::string> matched_words;
                    bool success = match_word_initials(query_word, result, result_words, used_words, matched_words);
                    if (success) {
                        for (const auto& word : matched_words) {
                            used_words[result].insert(word);
                        }
                        scores[result] += 100 * (int)matched_words.size();
                        continue;
                    }
                }
                {
                    int best_word_index = -1;
                    double best_score = -1.;
                    for (int i = 0;i < result_words.size();i++) {
                        if (used_words[result].contains(result_words[i])) {
                            continue;
                        }
                        double score = scorer.similarity(result_words[i]);
                        if (best_score < score) {
                            best_score = score;
                            best_word_index = i;
                        }
                    }
                    if (best_word_index >= 0 && best_score > 50.) {
                        used_words[result].insert(result_words[best_word_index]);
                        scores[result] += (int)best_score;
                        continue;
                    }
                }

                /* If we arrive here, the query word did not matched the result
                To clean the results, we want to dismiss the word */
                dismiss.insert(result);
            }
        }

        std::map<int, std::vector<std::string>> result_scores;

        /* Remove unwanted results and order the results */
        for (auto& pair : scores) {
            if (dismiss.contains(pair.first))
                continue;
            result_scores[pair.second].push_back(pair.first);
        }

        return result_scores;
    }
}