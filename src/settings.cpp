#include "pch.h"
#include <SimpleIni.h>
#include "settings.h"

Settings g_settings;

static const char* CONFIG_FILE = "bf42plus.ini";

inline void StringSetting::load(const CSimpleIni& ini) {
    value = ini.GetValue(section, name, value.c_str());
}

inline void StringSetting::save(CSimpleIni& ini) {
    if (dirty) {
        ini.SetValue(section, name, value.c_str(), comment);
        dirty = false;
    }
}

inline void BoolSetting::load(const CSimpleIni& ini) {
    value = ini.GetBoolValue(section, name, value);
}

inline void BoolSetting::save(CSimpleIni& ini) {
    if (dirty) {
        // SetBoolValue uses true/false, use SetValue with on/off instead because it's less programmer-y
        // GetBoolValue supports various boolean strings including on/off
        ini.SetValue(section, name, value ? L"on" : L"off", comment);
        dirty = false;
    }
}

Settings::Settings()
{
    settings = {
        &showConnectsInChat,
        &showIDInChat,
        &showIDInKills,
        &showIDInNametags,
        &showVoteInConsole,
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

    ini.SaveFile(CONFIG_FILE);

    return false;
}
