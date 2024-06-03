#include "../pch.h"

static bfs::list<ConsoleObject*> customCommands;

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

void register_custom_console_commands()
{
    ConsoleObjects::getSingleton()->registerConsoleObjects(customCommands);
    customCommands.clear();
}
