#include <string>
#include <vector>
#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/translit.h>

using namespace icu;

namespace str {
    std::string strip(const std::string& str);

    std::vector<std::string> split(const std::string& str, const std::string& split_char = " ");
}