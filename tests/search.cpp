#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <string>
#include <map>
#include <vector>

#include "core/search.h"

std::set<std::string> getResults(const std::vector<std::string>& query_words, const std::vector<std::string>& search_terms) {
    auto scores = Search::ScoreQueryAgainstResults(query_words, search_terms);
    std::set<std::string> results;

    for (auto& pair : scores) {
        for (auto& str : pair.second) {
            results.insert(str);
        }
    }
    return results;
}

TEST_SUITE("search") {
    TEST_CASE("fuzzy_match"
        * doctest::description("shouldn't take more than 100ms")
        * doctest::timeout(0.1)) {

        std::vector<std::string> search_terms = {
            "Import data to project",
            "Import project",
            "Create segmentation",
            "Train model",
            "Test model"
        };

        /* User intends to search for the word import */
        std::vector<std::string> query_words1 = { "import" };

        auto results1 = getResults(query_words1, search_terms);
        CHECK_MESSAGE(results1.contains("Import data to project"), "Search does not return one obvious results");
        CHECK_MESSAGE(results1.contains("Import project"), "Search does not return one obvious results");

        /* User intends to search for creating a new segmentation, but with a typo */
        std::vector<std::string> query_words2 = { "crwate", "seg" };
        auto results2 = getResults(query_words2, search_terms);
        WARN_MESSAGE(results2.contains("Create segmentation"), "Search with typo does not return result");

        /* User probably intends to search something with import and project */
        std::vector<std::string> query_words3 = { "impprj" };
        auto results3 = getResults(query_words3, search_terms);
        WARN_MESSAGE(results3.contains("Import data to project"), "Search with initials does not return result");
        WARN_MESSAGE(results3.contains("Import project"), "Search with initials does not return result");
    }
}