#pragma once

#include <memory>
#include <string>
#include <map>
#include <stdexcept>
#include <functional>

enum Language { LANG_FR, LANG_EN };

struct Translator {
    std::map<std::string, std::string> texts;
    // std::map <const std::string&, std::string(const std::string&)> plurals;
};

Translator build_FR();

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    auto size = static_cast<size_t>(size_s);
    auto buf = std::make_unique<char[]>(size);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

template<typename ... Args>
std::string gettext(const std::string& txt, Translator& babel, Args ... args) {
    if (babel.texts.contains(txt)) {
        return string_format(babel.texts[txt], args...);
    }
    return string_format(txt, args ...);
}

// const char* ngettext(const std::string& singular, const std::string& plural, long long int n) {

// }
#define TXT(X, ...)  gettext(X, *m_ui_state->current, __VA_ARGS__).c_str()

