#pragma once

#include <unordered_map>
#include <map>
#include <set>
#include <vector>
#include <string>

#include <rapidfuzz/fuzz.hpp>

namespace Search {
    std::map<int, std::vector<std::string>> ScoreQueryAgainstResults(const std::vector<std::string>& query_words, const std::vector<std::string>& results, const std::string& split_char = " ");
}