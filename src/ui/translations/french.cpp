#pragma once

#include "translate.h"

Translator build_FR() {
    Translator fr;
    fr.texts["hello world %s"] = "bonjour le monde %s";
    return fr;
}