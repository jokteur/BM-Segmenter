#include "translate.h"
#include "IconsMaterialDesign.h"

Translator build_FR() {
    Translator fr;
    fr.texts["Files"] = "Fichiers";
    fr.texts[ICON_MD_NOTE_ADD " New project"] = ICON_MD_NOTE_ADD " Nouveau projet";
    fr.texts[ICON_MD_FOLDER_OPEN
        " Open project"] = ICON_MD_FOLDER_OPEN" Ouvrir projet";
    fr.texts[ICON_MD_LANGUAGE " Language"] = ICON_MD_LANGUAGE " Langue";

    fr.texts["Settings"] = "Réglages";

    fr.texts["Search for something to do"] = "Tapez pour trouver une action";
    fr.texts[ICON_MD_DOWNLOAD " Import"] = ICON_MD_DOWNLOAD " Importation";
    fr.texts[ICON_MD_APPS " Dataset"] = ICON_MD_APPS" Données";
    fr.texts[ICON_MD_APP_REGISTRATION " Segmentation"] = ICON_MD_APP_REGISTRATION " Segmentation";
    fr.texts[ICON_MD_STRAIGHTEN " Measurements"] = ICON_MD_STRAIGHTEN " Mesures";
    return fr;
}