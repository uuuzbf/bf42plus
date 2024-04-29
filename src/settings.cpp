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
        ini.SetValue(section, name, value.c_str(), comment);
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
        ini.SetValue(section, name, value ? L"on" : L"off", comment);
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
        ini.SetValue(section, name, ISO88591ToWideString(GetStringFromColor(value)).c_str(), comment);
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
        ini.SetValue(section, name, ISO88591ToWideString(GetStringFromColors(value)).c_str(), comment);
    }
}

Settings::Settings()
{
    ini.SetQuotes(true);

    settings = {
        &showConnectsInChat,
        &showIDInChat,
        &showIDInKills,
        &showIDInNametags,
        &showVoteInConsole,
        &lowerNametags,
        &debugTextColor,
        &highPrecBlindTest,
        &presetBuddyColors,
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
                    debuglog("buddycolor: loaded %ls color %06X\n", name.pItem, color);
                    ::setBuddyColor(WideStringToISO88591(name.pItem), color);
                }
                else {
                    debuglog("buddycolor: '%ls' invalid in config, ignoring\n", name.pItem);
                }
            }
        }
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
