#include "pch.h"
#include <SimpleIni.h>
#include "settings.h"

Settings g_settings;

static const char* CONFIG_FILE = "bf42plus.ini";

inline void StringSetting::load(const CSimpleIni& ini)
{
    value = ini.GetValue(section, name, value.c_str());
}

inline void StringSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        ini.SetValue(section, name, value.c_str(), comment, true);
        dirty = false;
    }
}

inline void BoolSetting::load(const CSimpleIni& ini)
{
    value = ini.GetBoolValue(section, name, value);
}

inline void BoolSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        // SetBoolValue uses true/false, use SetValue with on/off instead because it's less programmer-y
        // GetBoolValue supports various boolean strings including on/off
        ini.SetValue(section, name, value ? L"on" : L"off", comment, true);
        dirty = false;
    }
}

inline void IntSetting::load(const CSimpleIni& ini)
{
    value = ini.GetLongValue(section, name, value);
}

inline void IntSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        ini.SetLongValue(section, name, value, comment, false, true);
        dirty = false;
    }
}

void FloatSetting::load(const CSimpleIni& ini)
{
    value = ini.GetDoubleValue(section, name, value);
}

void FloatSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        ini.SetDoubleValue(section, name, value, comment, true);
        dirty = false;
    }
}


void ColorSetting::load(const CSimpleIni& ini)
{
    auto colorString = ini.GetValue(section, name);
    if (colorString) {
        uint32_t color = GetColorFromString(WideStringToISO88591(colorString));
        if (color != InvalidColor) {
            value = color;
        }
        else {
            // color code in file was invalid, overwrite it with default value
            dirty = true;
        }
    }
}

void ColorSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        ini.SetValue(section, name, ISO88591ToWideString(GetStringFromColor(value)).c_str(), comment, true);
    }
}

void ColorListSetting::load(const CSimpleIni& ini)
{
    auto rawValue = ini.GetValue(section, name);
    if (rawValue) {
        auto s = std::stringstream(WideStringToISO88591(rawValue));
        std::vector<uint32_t> newColors;
        for (std::string colorstring; std::getline(s, colorstring, ' ');) {
            uint32_t color = GetColorFromString(colorstring);
            if (color != InvalidColor) {
                newColors.push_back(color);
            }
            else {
                // had to skip some invalid color, rewrite setting
                dirty = true;
            }
        }
        if (newColors.size() > 0) {
            value = newColors;
        }
        else {
            // did not parse any colors from file, reset to defaults
            dirty = true;
        }
    }
}

void ColorListSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        ini.SetValue(section, name, ISO88591ToWideString(GetStringFromColors(value)).c_str(), comment, true);
    }
}

void EnumSetting::load(const CSimpleIni& ini)
{
    std::string value = WideStringToISO88591(ini.GetValue(section, name));
    if (isValidValue(value)) {
        this->value = value;
    }
    else {
        // invalid enum value, overwrite with default value
        dirty = true;
    }
}

void EnumSetting::save(CSimpleIni& ini)
{
    if (dirty) {
        ini.SetValue(section, name, ISO88591ToWideString(value).c_str(), comment, true);
    }
}

Settings::Settings()
{
    ini.SetQuotes(true);
    ini.SetMultiKey(true);

    settings = {
        &showConnectsInChat,
        &showIDInChat,
        &showIDInKills,
        &showIDInNametags,
        &showVoteInConsole,
        &lowerNametags,
        &debugTextColor,
        &smootherGameplay,
        &presetBuddyColors,
        &correctedLookSensitivity,
        &stationaryMGInfSensitivity,
        &enable3DMineMap,
        &enable3DSupplyMap,
        &enable3DControlPointMap,
        &crashCreateFullDump,
        &crashDumpsToKeep,
        &fasterMapchange,
        &wrapChat,
        &screenshotFormat,
        &hitIndicatorTime,
    };
}

bool Settings::load()
{
    SI_Error rc = ini.LoadFile(CONFIG_FILE);
    if (rc < 0) {
        if (rc == SI_FILE && errno == ENOENT) {
            // config doesn't exist, force a save to create it
            save(true);
            return true;
        }
    }

    bool needToSaveNewSettings = false;

    // remove deleted options
    if (ini.KeyExists(L"general", L"unlockConsole")) {
        ini.Delete(L"general", L"unlockConsole", false);
        needToSaveNewSettings = true;
    }
    if (ini.KeyExists(L"general", L"highPrecBlindTest")) {
        ini.Delete(L"general", L"highPrecBlindTest", false);
        needToSaveNewSettings = true;
    }

    for (auto& setting : settings) {
        // if the setting exists in the config file, load it
        if (ini.KeyExists(setting->section, setting->name)) {
            setting->load(ini);
        }
        else {
            // add missing setting with default value by marking it for saving
            setting->dirty = true;
            needToSaveNewSettings = true;
        }
    }

    // Load buddy colors from config file
    if (!ini.SectionExists(L"buddycolors")) {
        // Section missing, create an empty one with some comments
        ini.SetValue(L"buddycolors", 0, 0,
            L"; This section contains the custom buddy colors\n"
            L"; Keys are player names, values are color codes\n"
            L"; The color code may be a webcolor name or in #rgb or #rrggbb format\n"
            L"; playername = colorcode");
        needToSaveNewSettings = true;
    }
    else {
        CSimpleIni::TNamesDepend names;
        if (ini.GetAllKeys(L"buddycolors", names)) {
            for (auto& name : names) {
                uint32_t color = GetColorFromString(WideStringToISO88591(ini.GetValue(L"buddycolors", name.pItem, L"")));
                if (color != InvalidColor) {
                    ::setBuddyColor(WideStringToISO88591(name.pItem), color, false);
                }
                else {
                    debuglog("buddycolor: '%ls' invalid in config, ignoring\n", name.pItem);
                }
            }
        }
    }
    if (!ini.SectionExists(L"ignorelist")) {
        // Section missing, create an empty one with some comments
        ini.SetValue(L"ignorelist", 0, 0,
            L"; This section contains the list of players you ignored");
        needToSaveNewSettings = true;
    }

    if (needToSaveNewSettings) {
        // save all settings marked
        save(false);
    }

    return true;
}

bool Settings::save(bool force)
{
    for (auto& setting : settings) {
        if (force) setting->dirty = true;
        setting->save(ini);
    }

    if(ini.SaveFile(CONFIG_FILE) == SI_OK) return true;
    return false;
}

ServerSettings g_serverSettings;

// Parses a single parameter from a string_view pointing to "&key=value&key=value..."
// Returns false if the string view does not contain a parameter (does not start with '&')
static bool parseNextParameter(std::string_view& view, std::string_view& key, std::string_view& value)
{
    if (view.starts_with("&")) {
        view.remove_prefix(1);

        // Find end of key
        auto end = view.find_first_of("=&~");
        //auto end = view.begin();
        //while (end != view.end() && *end != '=' && *end != '&' && *end != '~') end++;

        key = std::string_view(view.begin(), end == std::string_view::npos ? view.end() : (view.begin() + end));

        view.remove_prefix(key.length());
        
        // Has value?
        if (view.starts_with("=")) {
            view.remove_prefix(1);
            // Find end of value
            auto startOfNext = view.find('&');

            value = std::string_view(view.begin(), startOfNext == std::string_view::npos ? view.end() : (view.begin() + startOfNext));

            view.remove_prefix(value.length());
        }
        else {
            value = std::string_view();
        }
        return true;
    }
    return false;
}

void ServerSettings::parseFromText(const char* text)
{
    std::string_view str(text);
    for (size_t pos; (pos = str.find_first_of('%')) != std::string::npos; ) {
        str.remove_prefix(pos + 1);

        // Find closing character of setting
        size_t end = str.find_first_of('~');

        // If no closing character, stop processing
        if (end == std::string::npos) break;

        std::string_view setting(str.substr(0, end));

        // Setting names consist of uppercase letters and numbers [A-Z0-9]
        auto nameEnd = setting.begin();
        for (; nameEnd != setting.end() && ((*nameEnd >= 'A' && *nameEnd <= 'Z') || (*nameEnd >= '0' && *nameEnd <= '9')); nameEnd++);

        // nameEnd should point to the first parameter separator or the setting closing character
        if (nameEnd != setting.end() && *nameEnd != '&') break;

        std::string_view settingName(setting.begin(), nameEnd);
        std::string_view params(nameEnd, setting.end());

        // Max setting length is 4 for now
        if (settingName.size() > 1 && settingName.size() <= 4) {
            debuglogt("server setting: '%.*s'\n", settingName.size(), settingName.data());
            std::string_view key, value;
            //while (parseNextParameter(params, key, value)) debuglog("  '%.*s' -> '%.*s'\n", key.size(), key.data(), value.size(), value.data());
            
            if (settingName == "3DM") {
                mine3DMap.allow = true;
                while (parseNextParameter(params, key, value)) {
                    if (key == "D") mine3DMap.distance = strtol(value.data(), 0, 10);
                    else if (key == "C") mine3DMap.color = GetColorFromString(std::string(value.begin(), value.end()));
                    else if (key == "T") mine3DMap.text = value;
                }
            }
            else if (settingName == "3DS") {
                supplyDepot3DMap.allow = true;
                while (parseNextParameter(params, key, value)) {
                    if (key == "D") mine3DMap.distance = strtol(value.data(), 0, 10);
                }
            }
            else if (settingName == "3DCP") {
                controlPoint3DMap.allow = true;
                while (parseNextParameter(params, key, value)) {
                    if (key == "D") mine3DMap.distance = strtol(value.data(), 0, 10);
                }
            }
            else if (settingName == "3DO") {
                Custom3DMap c;
                ObjectTemplate* tmpl{};
                while (parseNextParameter(params, key, value)) {
                    if (key == "t") {
                        c.templateName = value;
                        tmpl = ObjectTemplate::getTemplateByName(bfs::string(value.data(), value.size()));
                        if (!tmpl) {
                            debuglogt("server settings 3DO - invalid template specified: %.*s\n", value.size(), value.data());
                            break;
                        }
                    }
                    else if (key == "T") c.text = value;
                    else if (key == "D") c.distance = strtol(value.data(), 0, 10);
                    else if (key == "C") c.color = GetColorFromString(std::string(value.begin(), value.end()));
                    else if (key == "A") c.onlySameTeam = false;
                    else if (key == "sD") c.showDistance = true;
                }
                if (tmpl) {
                    if (c.text.empty()) c.text = c.templateName;
                    custom3DMaps.emplace(std::make_pair(tmpl, c));
                }
                else {
                    debuglogt("ignoring 3DO server setting because no valid template\n");
                }
            }
            else if (settingName == "UI") {
                while (parseNextParameter(params, key, value)) {
                    if (key == "hssdeath") UI.openSpawnScreenOnDeath = false;
                    else if (key == "hssjoin") UI.openSpawnScreenOnJoin = false;
                    else if (key == "nfr") UI.allowFasterRestart = false;
                    else if (key == "sbw") UI.skipBriefingWindow = true;
                    else if (key == "hen") UI.showEnemyNametags = false;
                }
            }
            
        }

        str.remove_prefix(setting.length());
    }
}
