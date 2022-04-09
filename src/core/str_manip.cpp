#include "str_manip.h"

#include <sstream>

namespace str {
    std::string strip(const std::string& str) {
        UnicodeString source = UnicodeString::fromUTF8(StringPiece(str.c_str()));

        UErrorCode status = U_ZERO_ERROR;
        Transliterator* accentsConverter = Transliterator::createInstance(
            "NFD; [:Nonspacing Mark:] Remove; NFC", UTRANS_FORWARD, status);
        accentsConverter->transliterate(source);
        std::string result;

        source.toLower().toUTF8String(result);
        return result;
    }

    std::vector<std::string> split(const std::string& str, const std::string& split_char) {
        std::stringstream test(str);
        std::vector<std::string> seglist;
        std::string segment;

        // Only works for latin alphabets
        while (std::getline(test, segment, split_char[0])) {
            seglist.push_back(segment);
        }
        return seglist;
    }
}
