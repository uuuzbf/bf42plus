#pragma once
#include <string>
#include "SimpleIni.h"

struct Setting {
    const wchar_t* section;
    const wchar_t* name;
    const wchar_t* comment;
    int resourceid;
    bool dirty = false;
    virtual void load(const CSimpleIni& ini) = 0;
    virtual void save(CSimpleIni& ini) = 0;
protected:
    Setting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid) :
        section(section), name(name), comment(comment), resourceid(resourceid) {};
};

struct StringSetting : public Setting {
    std::wstring value;
    StringSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, std::wstring value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
};

struct BoolSetting : public Setting {
    bool value;
    BoolSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, bool value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
    operator bool() const { return value; };
};

class Settings {
    CSimpleIni ini;
    std::vector<Setting*> settings;
public:
    Settings();
    bool load();
    bool save(bool force);
    // Store a buddy color in the settings file.
    // Buddy colors are stored in a separate [buddycolors] section.
    // If color is set to InvalidColor, the entry is removed for the given player name.
    void setBuddyColor(std::wstring name, uint32_t color) {
        if (color != InvalidColor) ini.SetValue(L"buddycolors", name.c_str(), ASCIIToWideString(GetStringFromColor(color)).c_str());
        else ini.Delete(L"buddycolors", name.c_str(), false);
    };
    BoolSetting showConnectsInChat = {
        L"general", L"showConnectsInChat",
        L"; Show a message in the status chat when a player connects to the server",
        0, false };
    BoolSetting showIDInChat = {
        L"general", L"showIDInChat",
        L"; Show player IDs in chat messages, name changes and joins/disconnects",
        0, false };
    BoolSetting showIDInKills = {
        L"general", L"showIDInKills",
        L"; Show player IDs in kill messages",
        0, false };
    BoolSetting showIDInNametags = {
        L"general", L"showIDInNametags",
        L"; Show player IDs in nametags",
        0, false };
    BoolSetting showVoteInConsole = {
        L"general", L"showVoteInConsole",
        L"; Show who starts a vote or votes in the console",
        0, false };
};

extern Settings g_settings;
