#include "str_manip.h"

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
}
