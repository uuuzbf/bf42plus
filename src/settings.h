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

struct ColorSetting : public Setting {
    uint32_t value;
    ColorSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, uint32_t value) :
        Setting(section, name, comment, resourceid), value(value) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
    operator uint32_t() const { return value; };
};

// space separated list of colors
struct ColorListSetting : public Setting {
    std::vector<uint32_t> value;
    ColorListSetting(const wchar_t* section, const wchar_t* name, const wchar_t* comment, int resourceid, std::initializer_list<uint32_t> values) :
        Setting(section, name, comment, resourceid), value(values) {};
    virtual void load(const CSimpleIni& ini);
    virtual void save(CSimpleIni& ini);
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
        if (color != InvalidColor) ini.SetValue(L"buddycolors", name.c_str(), ISO88591ToWideString(GetStringFromColor(color)).c_str());
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
    BoolSetting lowerNametags = {
        L"general", L"lowerNametags",
        L"; Gradually lower nametags when u get closer to other players",
        0, true };
    ColorSetting debugTextColor = {
        L"general", L"debugTextColor",
        L"; Sets the color of console.showStats, console.showFPS, etc",
        0, 0xffff00 /*yellow*/ };
    BoolSetting highPrecBlindTest = {
        L"general", L"highPrecBlindTest",
        L"; Enable blind testing of high precision FPU mode.\n"
        L"; At startup the precision patch may be applied, and on map end\n"
        L"; it is revealed if it was applied or not. Please report your experiences\n"
        L"; on the SiMPLE forum or on discord (username uuuzbf).\n"
        L"; THIS SETTING WILL BE REMOVED IN NEXT VERSIONS",
        0, false };
    ColorListSetting presetBuddyColors = {
        L"general", L"presetBuddyColors",
        L"; Sets the colors chosen when the ADD BUDDY button is repeatedly clicked on the scoreboard",
        0, { DefaultBuddyColor /*lime*/, 0x1e90ff /*dodgerblue*/, 0xffff00 /*yellow*/, 0x8a2be2 /*blueviolet*/} };
};

extern Settings g_settings;
