#include "../pch.h"

static bfs::list<ConsoleObject*> customCommands;

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(push)
#pragma warning(disable: 26495 4100 4410 4409 4740)

__declspec(naked) bool ConsoleObjects::registerConsoleObjects(bfs::list<ConsoleObject*>& list)
{
    _asm mov eax, 0x005AC970
    _asm jmp eax
}

__declspec(naked) bfs::string ConsoleObject::execute(bfs::string* argv, int argc)
{
    _asm mov eax, 0x005ACC30
    _asm jmp eax
}

#pragma warning(pop)

class ConsoleObjectBoolSetting : public ConsoleObject {
protected:
    bool result;
public:
    ConsoleObjectBoolSetting() {
        isdynamic = true;
        type = 1;
        access = 1;
        objectname = "plus";
        minargcount = 0;
        maxargcount = 1;
        argdesc[0] = "bool";
        argtype[0] = -1;
        retdesc = "bool";
        customCommands.push_back(this);
    };
    virtual bool isObjectActive() const { return true; };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            args[0] = atoi(value.c_str()) != 0;
        }
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            return result ? "1" : "0";
        }
        return "";
    };
    void needsRestart() { BfMenu::getSingleton()->outputConsole("This setting only takes effect after the next launch!"); };
};

class ConsoleObjectIntSetting : public ConsoleObject {
protected:
    int result;
public:
    ConsoleObjectIntSetting() {
        isdynamic = true;
        type = 1;
        access = 1;
        objectname = "plus";
        minargcount = 0;
        maxargcount = 1;
        argdesc[0] = "int";
        argtype[0] = -1;
        retdesc = "int";
        customCommands.push_back(this);
    };
    virtual bool isObjectActive() const { return true; };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            args[0] = atoi(value.c_str());
        }
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            char temp[32];
            return _itoa(result, temp, 10);
        }
        return "";
    };
};

class ConsoleObjectFloatSetting : public ConsoleObject {
protected:
    float result;
    float arg;
public:
    ConsoleObjectFloatSetting() {
        isdynamic = true;
        type = 1;
        access = 1;
        objectname = "plus";
        minargcount = 0;
        maxargcount = 1;
        argdesc[0] = "float";
        argtype[0] = -1;
        retdesc = "float";
        customCommands.push_back(this);
    };
    virtual bool isObjectActive() const { return true; };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            this->arg = atof(value.c_str());
        }
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            char temp[32];
            snprintf(temp, 32, "%f", result);
            return temp;
        }
        return "";
    };
};

class ConsoleObjectEnumSetting : public ConsoleObject {
protected:
    bfs::string param;
    bfs::string result;
    std::string enumList;
public:
    ConsoleObjectEnumSetting() {
        isdynamic = true;
        type = 1;
        access = 1;
        objectname = "plus";
        minargcount = 0;
        maxargcount = 1;
        argdesc[0] = enumList.c_str();
        argtype[0] = -1;
        retdesc = "string";
        customCommands.push_back(this);
    };
    virtual bool isObjectActive() const { return true; };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            param = value;
        }
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            return result;
        }
        return "";
    };
    void updatePossibleValues(const std::vector<std::string>& possibleValues) {
        std::stringstream ss;
        for (auto it = possibleValues.begin();;) {
            ss << *it;
            if (++it == possibleValues.end()) break;
            else ss << '|';
        }
        enumList = ss.str();
        argdesc[0] = enumList.c_str();
    }
};

class ConsoleObjectPlusShowConnectsInChat : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusShowConnectsInChat() : ConsoleObjectBoolSetting() {
        functionname = "showConnectsInChat";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.showConnectsInChat.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.showConnectsInChat.value = args[0] != 0;
            g_settings.showConnectsInChat.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusShowConnectsInChat commandPlusShowConnectsInChat;

class ConsoleObjectPlusShowIDInChat : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusShowIDInChat() : ConsoleObjectBoolSetting() {
        functionname = "showIDInChat";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.showIDInChat.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.showIDInChat.value = args[0] != 0;
            g_settings.showIDInChat.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusShowIDInChat commandPlusShowIDInChat;

class ConsoleObjectPlusShowIDInKills : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusShowIDInKills() : ConsoleObjectBoolSetting() {
        functionname = "showIDInKills";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.showIDInKills.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.showIDInKills.value = args[0] != 0;
            g_settings.showIDInKills.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusShowIDInKills commandPlusShowIDInKills;


class ConsoleObjectPlusShowIDInNametags : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusShowIDInNametags() : ConsoleObjectBoolSetting() {
        functionname = "showIDInNametags";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.showIDInNametags;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.showIDInNametags.value = args[0] != 0;
            g_settings.showIDInNametags.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
            needsRestart();
        }
        return 0;
    };
};
ConsoleObjectPlusShowIDInNametags commandPlusShowIDInNametags;


class ConsoleObjectPlusShowVoteInConsole : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusShowVoteInConsole() : ConsoleObjectBoolSetting() {
        functionname = "showVoteInConsole";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.showVoteInConsole;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.showVoteInConsole.value = args[0] != 0;
            g_settings.showVoteInConsole.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusShowVoteInConsole commandPlusShowVoteInConsole;


class ConsoleObjectPlusLowerNametags : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusLowerNametags() : ConsoleObjectBoolSetting() {
        functionname = "lowerNametags";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.lowerNametags;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.lowerNametags.value = args[0] != 0;
            g_settings.lowerNametags.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
            needsRestart();
        }
        return 0;
    };
};
ConsoleObjectPlusLowerNametags commandPlusLowerNametags;


class ConsoleObjectPlusSave : public ConsoleObject {
    bfs::string result;
public:
    ConsoleObjectPlusSave() {
        isdynamic = true;
        type = 0;
        access = 1;
        objectname = "plus";
        functionname = "save";
        minargcount = 0;
        maxargcount = 0;
        retdesc = "std::string";
        customCommands.push_back(this);
    };
    virtual bool isObjectActive() const { return true; };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            if (g_settings.save(false)) {
                result = "Settings saved";
            }
            else {
                result = "Failed to save settings";
            }
            hasreturnvalue = true;
            return &result;
        }
        hasreturnvalue = false;
        return 0;
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            return result;
        }
        return "";
    };
};
ConsoleObjectPlusSave commandPlusSave;


class ConsoleObjectPlusBuddyColor : public ConsoleObject {
    std::string buddyname;
public:
    ConsoleObjectPlusBuddyColor() {
        isdynamic = true;
        type = 0;
        access = 1;
        objectname = "plus";
        functionname = "buddyColor";
        minargcount = 2;
        maxargcount = 2;
        argdesc[0] = "std::string";
        argdesc[1] = "color";
        argtype[0] = -1;
        argtype[1] = -1;
        retdesc = "void";
        customCommands.push_back(this);
    };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            buddyname = value;
            args[0] = (intptr_t)&buddyname;
        }
        else if (arg == 2) {
            args[1] = GetColorFromString(value);
        }
    };
    virtual bool isObjectActive() const { return true; };
    virtual void* executeObjectMethod() {
        if (argcount == 2) {
            setBuddyColor(buddyname, args[1], true);
        }
        hasreturnvalue = false;
        return 0;
    };
};
ConsoleObjectPlusBuddyColor commandPlusBuddyColor;


class ConsoleObjectPlusDebugTextColor : public ConsoleObject {
    uint32_t result;
public:
    ConsoleObjectPlusDebugTextColor() {
        isdynamic = true;
        type = 0;
        access = 1;
        objectname = "plus";
        functionname = "debugTextColor";
        minargcount = 0;
        maxargcount = 1;
        argdesc[0] = "color";
        argtype[0] = -1;
        retdesc = "color";
        customCommands.push_back(this);
    };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            args[0] = GetColorFromString(value);
        }
    };
    virtual bool isObjectActive() const { return true; };
    virtual void* executeObjectMethod() {
        if (argcount == 1) {
            g_settings.debugTextColor.value = args[0];
            g_settings.debugTextColor.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        else if (argcount == 0) {
            result = g_settings.debugTextColor;
            hasreturnvalue = true;
            return &result;
        }
        return 0;
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            return GetStringFromColor(result);
        }
        return "";
    };
};
ConsoleObjectPlusDebugTextColor commandPlusDebugTextColor;

class ConsoleObjectPlusSmootherGameplay : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusSmootherGameplay() : ConsoleObjectBoolSetting() {
        functionname = "smootherGameplay";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.smootherGameplay.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.smootherGameplay.value = args[0] != 0;
            g_settings.smootherGameplay.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
            needsRestart();
        }
        return 0;
    };
};
ConsoleObjectPlusSmootherGameplay commandPlusSmootherGameplay;


class ConsoleObjectPlusPresetBuddyColors : public ConsoleObject {
    std::vector<uint32_t> colors;
    bool invalid = false;
public:
    ConsoleObjectPlusPresetBuddyColors() {
        isdynamic = true;
        type = 0;
        access = 1;
        objectname = "plus";
        functionname = "presetBuddyColors";
        minargcount = 0;
        maxargcount = 10;
        for (int i = 0; i < 10; i++) {
            argdesc[i] = "color";
            argtype[i] = -1;
        }
        retdesc = "colors";
        customCommands.push_back(this);
    };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            invalid = false;
            colors.clear();
        }
        else if (invalid) return;

        uint32_t color = GetColorFromString(value);
        if (color == InvalidColor) invalid = true;
        colors.push_back(color);
    };
    virtual bool isObjectActive() const { return true; };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            colors = g_settings.presetBuddyColors.value;
            hasreturnvalue = true;
            return &colors;
        }
        else {
            if (!invalid) {
                // use whatever was parsed into the colors vector
                g_settings.presetBuddyColors.value = colors;
                g_settings.presetBuddyColors.dirty = true;
                g_settings.save(false);
            }
            else {
                // invalid color specified
                BfMenu::getSingleton()->outputConsole(std::format("color {} is invalid", colors.size()));
            }
            hasreturnvalue = false;
        }
        return 0;
    };
    virtual bfs::string getReturnValueAsString() {
        if (hasreturnvalue) {
            return GetStringFromColors(colors);
        }
        return "";
    };
};
ConsoleObjectPlusPresetBuddyColors commandPlusPresetBuddyColors;

class ConsoleObjectPlusCorrectedLookSensitivity : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusCorrectedLookSensitivity() : ConsoleObjectBoolSetting() {
        functionname = "correctedLookSensitivity";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.correctedLookSensitivity.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.correctedLookSensitivity.value = args[0] != 0;
            g_settings.correctedLookSensitivity.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusCorrectedLookSensitivity commandPlusCorrectedLookSensitivity;

class ConsoleObjectPlusStationaryMGInfSensitivity : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusStationaryMGInfSensitivity() : ConsoleObjectBoolSetting() {
        functionname = "stationaryMGInfSensitivity";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.stationaryMGInfSensitivity.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.stationaryMGInfSensitivity.value = args[0] != 0;
            g_settings.stationaryMGInfSensitivity.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusStationaryMGInfSensitivity commandPlusStationaryMGInfSensitivity;

class ConsoleObjectPlusEnable3DMineMap : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusEnable3DMineMap() : ConsoleObjectBoolSetting() {
        functionname = "enable3DMineMap";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.enable3DMineMap.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.enable3DMineMap.value = args[0] != 0;
            g_settings.enable3DMineMap.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusEnable3DMineMap commandPlusEnable3DMineMap;

class ConsoleObjectPlusEnable3DSupplyMap : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusEnable3DSupplyMap() : ConsoleObjectBoolSetting() {
        functionname = "enable3DSupplyMap";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.enable3DSupplyMap.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.enable3DSupplyMap.value = args[0] != 0;
            g_settings.enable3DSupplyMap.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusEnable3DSupplyMap commandPlusEnable3DSupplyMap;

class ConsoleObjectPlusFasterMapchange : public ConsoleObjectBoolSetting {
public:
    ConsoleObjectPlusFasterMapchange() : ConsoleObjectBoolSetting() {
        functionname = "fasterMapchange";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.fasterMapchange.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.fasterMapchange.value = args[0] != 0;
            g_settings.fasterMapchange.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
            needsRestart();
        }
        return 0;
    };
};
ConsoleObjectPlusFasterMapchange commandPlusFasterMapchange;

class ConsoleObjectPlusWrapChat : public ConsoleObjectIntSetting {
public:
    ConsoleObjectPlusWrapChat() : ConsoleObjectIntSetting() {
        functionname = "wrapChat";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.wrapChat.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.wrapChat.value = args[0];
            g_settings.wrapChat.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusWrapChat commandPlusWrapChat;

class ConsoleObjectPlusScreenshotFormat : public ConsoleObjectEnumSetting {
public:
    ConsoleObjectPlusScreenshotFormat() : ConsoleObjectEnumSetting() {
        functionname = "screenshotFormat";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.screenshotFormat.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            if (g_settings.screenshotFormat.isValidValue(param)) {
                g_settings.screenshotFormat.value = param;
                g_settings.screenshotFormat.dirty = true;
                g_settings.save(false);
            }
            else {
                BfMenu::getSingleton()->outputConsole("Invalid value!");
            }
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusScreenshotFormat commandPlusScreenshotFormat;

class ConsoleObjectPlusHitIndicatorTime : public ConsoleObjectFloatSetting {
public:
    ConsoleObjectPlusHitIndicatorTime() : ConsoleObjectFloatSetting() {
        functionname = "hitIndicatorTime";
    };
    virtual void* executeObjectMethod() {
        if (argcount == 0) {
            result = g_settings.hitIndicatorTime.value;
            hasreturnvalue = true;
            return &result;
        }
        else if (argcount == 1) {
            g_settings.hitIndicatorTime.value = arg;
            g_settings.hitIndicatorTime.dirty = true;
            g_settings.save(false);
            hasreturnvalue = false;
        }
        return 0;
    };
};
ConsoleObjectPlusHitIndicatorTime commandPlusHitIndicatorTime;

#ifdef _DEBUG
class ConsoleObjectPlusCrash : public ConsoleObject {
    bfs::string param;
public:
    ConsoleObjectPlusCrash() {
        isdynamic = true;
        type = 0;
        access = 1;
        objectname = "plus";
        functionname = "crash";
        minargcount = 1;
        maxargcount = 1;
        argdesc[0] = "crash";
        argtype[0] = -1;
        retdesc = "void";
        customCommands.push_back(this);
    };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            param = value;
        }
    };
    virtual bool isObjectActive() const { return true; };
    virtual void* executeObjectMethod() {
        if (argcount == 1) {
            if (param == "crash") {
                // patch mov ecx,[0] into Setup__mainLoop
                patchBytes(0x0044ABC0, { 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00 });
            }
            else if (param == "hang") {
                // patch jmp -2 (infinite loop) into Setup__mainLoop
                patchBytes(0x0044ABC0, { 0xeb, 0xfe });
            }
        }
        hasreturnvalue = false;
        return 0;
    };
};
ConsoleObjectPlusCrash commandPlusCrash;
#endif

class ConsoleObjectPlusPoke : public ConsoleObject {
    int param1;
    bfs::string param2;
public:
    ConsoleObjectPlusPoke() {
        isdynamic = true;
        type = 0;
        access = 1;
        objectname = "plus";
        functionname = "poke";
        minargcount = 2;
        maxargcount = 2;
        argdesc[0] = "int";
        argtype[0] = -1;
        argdesc[1] = "std::string";
        argtype[1] = -1;
        retdesc = "void";
        customCommands.push_back(this);
    };
    virtual void setArgFromString(int arg, bfs::string const& value) {
        if (arg == 1) {
            param1 = atoi(value.c_str());
        }
        else if (arg == 2) {
            param2 = value;
        }
    };
    virtual bool isObjectActive() const { return true; };
    virtual void* executeObjectMethod() {
        if (argcount == 2) {
            bfs::wstring wparam2 = ISO88591ToWideString(param2);
            char temp[64];
            snprintf(temp, 64, "%i -> '%s' - '%ls'", param1, param2.c_str(), wparam2.c_str());
            BfMenu::getSingleton()->outputConsole(temp);
            if (param1 == 1) {
                BfMenu::getSingleton()->setInfoMessage(param2);
            }
            else if (param1 == 2) { // spawn point selected, spawn point not selected select one
                if (param2.length > 0) {
                    SpawnScreen_setSpawnMessage(wparam2);
                    forceSpawnTextToShow(true);
                }
                else {
                    forceSpawnTextToShow(false);
                }
            }
            else if (param1 == 3) { // game is restarting, min number of players
                if (param2.length > 0) {
                    BfMenu::getSingleton()->setStatusMessage(wparam2);
                }
                else {
                    BfMenu::getSingleton()->clearStatusMessage();
                }
            }
            else if (param1 == 4) {
                BfMenu::getSingleton()->setCenterKillMessage(wparam2);
                //BfMenu::getSingleton()->setCenterKillMessage(L"baaaaaa ba ba ba");
            }
            else if (param1 == 5) { // Warning! Connection problems detected!, chat flood, OVERRIDES setStatusMessage if visible
                if (param2.length > 0) {
                    BfMenu::getSingleton()->showDisconnectMessage(wparam2);
                    forceDisconnectMessageToShow(true);
                }
                else {
                    BfMenu::getSingleton()->hideDisconnectMessage();
                    forceDisconnectMessageToShow(false);
                }
            }
        }
        hasreturnvalue = false;
        return 0;
    };
};
ConsoleObjectPlusPoke commandPlusPoke;

void register_custom_console_commands()
{
    commandPlusScreenshotFormat.updatePossibleValues(g_settings.screenshotFormat.getPossibleValues());

    ConsoleObjects::getSingleton()->registerConsoleObjects(customCommands);
    customCommands.clear();
}
