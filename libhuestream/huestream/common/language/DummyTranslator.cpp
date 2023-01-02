/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#include <huestream/config/Config.h>
#include <huestream/common/language/DummyTranslator.h>

#include <string>
#include <map>

namespace huestream {

    std::map<std::string, std::string> DummyTranslator::english = {
        { "ID_PRESS_PUSH_LINK",        " found. Press the push-link button on the Hue Bridge." },
        { "ID_SELECT_GROUP",           "Select an entertainment area to use." },
        { "ID_NO_BRIDGE_FOUND",        "No Hue Bridge was found. Make sure your Hue Bridge is connected to your home Wi-Fi network or enter the IP address manually." },
        { "ID_BRIDGE_NOT_FOUND",       "We couldn't connect to your Hue Bridge. Make sure your Hue Bridge is connected to your home Wi-Fi network or search for another Hue Bridge." },
        { "ID_INVALID_MODEL",          "The entertainment feature is not available on the round shaped Hue Bridge. Please use the square shaped Hue Bridge." },
        { "ID_INVALID_VERSION",        "Your Hue Bridge software is out of date. Go to Settings in the Hue app to update your Hue Bridge." },
        { "ID_NO_GROUP_AVAILABLE",     "Your Hue Bridge is not yet set up for entertainment. To continue, go to settings in the Hue app to create an entertainment area." },
        { "ID_BRIDGE_BUSY",            "Your Hue Bridge is already active in another entertainment session: " },
        { "ID_STREAMING_DISCONNECTED", "Entertainment session stopped." }
    };

    std::map<std::string, std::string> DummyTranslator::dutch = {
        { "ID_PRESS_PUSH_LINK",        " gevonden. Druk op de link knop van je Hue Bridge." },
        { "ID_SELECT_GROUP",           "Kies een Entertainment gebied om te gebruiken." },
        { "ID_NO_BRIDGE_FOUND",        "Geen Hue Bridge gevonden. Controleer of je Hue Bridge verbonden is met je huis Wi-Fi netwerk of voer het IP adres handmatig in." },
        { "ID_BRIDGE_NOT_FOUND",       "We kunnen niet me je Hue Bridge verbinden. Controleer of je hue Bridge verbonden is met je huis Wi-Fi netwerk of zoek naar een andere Hue Bridge." },
        { "ID_INVALID_MODEL",          "De entertainment functionaliteit is niet beschikbaar op de ronde Hue Bridge. Gebruik de vierkante Hue Bridge." },
        { "ID_INVALID_VERSION",        "Je Hue Bridge software is verouderd. Ga naar instellinen in de Hue app om je Hue Bridge te updaten." },
        { "ID_NO_GROUP_AVAILABLE",     "Je Hue Bridge is nog niet opgezet voor Entertainment. Om verder te gaan, ga naar instellingen in de Hue app om een Entertainment gebied aan te maken." },
        { "ID_BRIDGE_BUSY",            "Je Hue Bridge is al actief in een andere entertainment sessie: " },
        { "ID_STREAMING_DISCONNECTED", "Entertainment sessie gestopt." }
    };

    std::map<std::string, std::string> DummyTranslator::german = {
        { "ID_PRESS_PUSH_LINK",        u8" gefunden. Drücken Sie die Push-Link-Taste der Hue Bridge." },
        { "ID_SELECT_GROUP",           u8"Wählen Sie ein zu verwendendes Entertainment-Bereich." },
        { "ID_NO_BRIDGE_FOUND",        "Es wurde keine Hue Bridge gefunden. Vergewissern Sie sich, dass Ihre Hue Bridge mit Ihrem WLAN-Heimnetz verbunden ist oder geben Sie die IP-Adresse manuell ein." },
        { "ID_BRIDGE_NOT_FOUND",       "Wir konnten keine Verbindung zu Ihrer Hue Bridge herstellen. Vergewissern Sie sich, dass Ihre Hue Bridge mit Ihrem WLAN-Heimnetz verbunden ist oder suchen Sie nach einer anderen Hue Bridge." },
        { "ID_INVALID_MODEL",          u8"Die Entertainment-Funktion steht bei der runden Hue Bridge nicht zur Verfügung. Bitte verwenden Sie die rechteckige Hue Bridge." },
        { "ID_INVALID_VERSION",        "Die Software Ihrer Hue Bridge ist veraltet. Gehen Sie auf Einstellungen in der Hue App, um Ihre Hue Bridge zu aktualisieren." },
        { "ID_NO_GROUP_AVAILABLE",     u8"Ihre Hue Bridge noch nicht für Entertainment eingerichtet. Um fortzufahren, gehen Sie zu den Einstellungen in der Hue App und erstellen Sie ein Entertainment-Bereich." },
        { "ID_BRIDGE_BUSY",            "Ihre Hue Bridge ist bereits in einer anderen Entertainment-Sitzung aktiv: " },
        { "ID_STREAMING_DISCONNECTED", "Entertainment-Sitzung gestoppt." }
    };

    std::map<std::string, std::map<std::string, std::string>> DummyTranslator::translations = {
        { "en", english },
        { "nl", dutch },
        { "de", german }
    };

    DummyTranslator::DummyTranslator(std::string language) :
    _language(language) {
    }

    std::string DummyTranslator::Translate(std::string tag) const {
        auto translation = english;

        auto i = translations.find(_language);
        if (i != translations.end()) {
            translation = i->second;
        }

        auto j = translation.find(tag);
        if (j != translation.end()) {
            return j->second;
        }

        return "Internal error";
    }

}  // namespace huestream
