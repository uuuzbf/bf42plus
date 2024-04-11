#include "../pch.h"

#pragma warning(push)
#pragma warning(disable: 4100)

__declspec(naked) BFPlayer* BfMenu::getLocalPlayer()
{
    _asm mov eax, 0x0045CE60
    _asm jmp eax
}

__declspec(naked) bfs::wstring* BfMenu::stringToWide(bfs::wstring* out, const bfs::string* in) noexcept
{
    _asm mov eax, 0x0045D1A0
    _asm jmp eax
}

__declspec(naked) void BfMenu::addGameInfoMessage(bfs::wstring message, int team) noexcept
{
    _asm mov eax, 0x006A8F10
    _asm jmp eax
}

static uintptr_t addPlayerChatMessage_orig = 0x006A8E10;
__declspec(naked) void BfMenu::addPlayerChatMessage(bfs::wstring message, BFPlayer* player, int team) noexcept
{
    _asm mov eax, addPlayerChatMessage_orig
    _asm jmp eax
}

static uintptr_t addRadioChatMessage_orig = 0x006A8F80;
__declspec(naked) void BfMenu::addRadioChatMessage(bfs::wstring message, BFPlayer* player, int team) noexcept
{
    _asm mov eax, addRadioChatMessage_orig
    _asm jmp eax
}

static uintptr_t addChatMessageInternal_orig = 0x006A89C0;
__declspec(naked) void BfMenu::addChatMessageInternal(bfs::wstring message, int team, int firstLinePos, int* numMessages, int maxLines, int* age, int type, bool isBuddy) noexcept
{
    _asm mov eax, addChatMessageInternal_orig
    _asm jmp eax
}

static uintptr_t setCenterKillMessage_orig = 0x006A88E0;
__declspec(naked) void BfMenu::setCenterKillMessage(bfs::wstring message) noexcept
{
    _asm mov eax, setCenterKillMessage_orig
    _asm jmp eax
}

__declspec(naked) void BfMenu::outputConsole(bfs::string message) noexcept
{
    _asm mov eax, 0x006A7D00
    _asm jmp eax
}

#pragma warning(pop)

void BfMenu::addPlayerChatMessage_hook(bfs::wstring message, BFPlayer* player, int team)
{
    bool reset = false;
    uintptr_t RA = (uintptr_t)_ReturnAddress();
    // called by functions triggered by chat input?
    if (RA == 0x006AC21B || RA == 0x006AC488) {
        BFPlayer* localPlayer = getLocalPlayer();
        if (localPlayer) {
            setCurrentMessagePID(localPlayer->getId());
            reset = true;
        }
    }
    else if (RA == 0x00491C4D) { // called by GameClient::handleChatMessage?
        if (player) {
            setCurrentMessagePID(player->getId());
            reset = true;
        }
    }
    addPlayerChatMessage(message, player, team);
    if (reset) {
        setCurrentMessagePID(-1);
    }
}

void BfMenu::addRadioChatMessage_hook(bfs::wstring message, BFPlayer* player, int team)
{
    bool reset = false;
    if (player) {
        setCurrentMessagePID(player->getId());
        reset = true;
    }
    addRadioChatMessage(message, player, team);
    if (reset) {
        setCurrentMessagePID(-1);
    }
}

void BfMenu::addChatMessageInternal_hook(bfs::wstring message, int team, int firstLinePos, int* numMessages, int maxLines, int* age, int type, bool isBuddy)
{
    // if oldChatListStyle is set, all messages will be treated as kill messages
    if ((g_settings.showIDInChat && type != 2) || (g_settings.showIDInKills && type == 2)) {
        int pid = getCurrentMessagePID();
        if (pid != -1) {
            // prepend pid to playerid
            wchar_t pidprefix[64];
            _snwprintf(pidprefix, 64, L".%d ", pid);
            message.replace(0, 0, pidprefix);
        }
        pid = getCurrentMessageSecondaryPID();
        if (pid != -1) {
            // append pid to playerid
            wchar_t pidsuffix[64];
            _snwprintf(pidsuffix, 64, L" .%d", pid);
            message.append(pidsuffix);
        }
    }
    addChatMessageInternal(message, team, firstLinePos, numMessages, maxLines, age, type, isBuddy);
}

void BfMenu::setCenterKillMessage_hook(bfs::wstring message)
{
    // Hide "xy is no more" from center kill message display. This allows
    // the "xy killed a teammate" message to be displayed, otherwise it
    // is immediatedly overwritten.
    if ((uintptr_t)_ReturnAddress() == 0x0049469E) return;

    // clear non printables to fix some players bugging the message
    {
        wchar_t* data = message.data();
        for (size_t i = 0, size = message.size(); i < size; i++) {
            wchar_t c = data[i];
            if (c < ' ' || c == 0x7F || c == 0xA0) data[i] = ' ';
        }
    }

    if (g_settings.showIDInKills) {
        int pid = getCurrentMessagePID();
        if (pid != -1) {
            // prepend pid to playerid
            wchar_t pidprefix[64];
            _snwprintf(pidprefix, 64, L".%d ", pid);
            message.replace(0, 0, pidprefix);
        }
        pid = getCurrentMessageSecondaryPID();
        if (pid != -1) {
            // append pid to playerid
            wchar_t pidsuffix[64];
            _snwprintf(pidsuffix, 64, L" .%d", pid);
            message.append(pidsuffix);
        }
    }

    setCenterKillMessage(message);
}

void chatMessage(std::string message, bool status, int team)
{
    BfMenu* menu = BfMenu::getSingleton();
    if (!menu || !menu->isInitialized()) return;
    bfs::string temp = message;
    bfs::wstring wmessage;
    menu->stringToWide(&wmessage, &temp);
    if (!status) {
        menu->addPlayerChatMessage(wmessage, 0, team);
    }
    else {
        menu->addGameInfoMessage(wmessage, team);
    }
}


void patch_add_id_to_nametag()
{
    static const char* fmt_without_hp = "#%d [%s]";
    static const char* fmt_with_hp = "#%d [%s] (%d%%)";
    // add id to nametag without hp
    BEGIN_ASM_CODE(a)
        pop ecx
        push ecx
        push ecx
        mov eax, fmt_without_hp
        mov [esp+4], eax
        mov eax, [ebp+0x70]
        mov [esp+8], eax
        mov eax, 0x008C3520 // sprintf
        mov eax,[eax] // msvc assembler cant do this in one step
        call eax
        add esp,0x10
    MOVE_CODE_AND_ADD_CODE(a, 0x004416CB, 9, HOOK_DISCARD_ORIGINAL);
    // add id to nametag with hp
    BEGIN_ASM_CODE(b)
        push ecx
        push eax
        mov eax,[ebp+0x70]
        push eax
        lea edx,[esp+0x1B8]
        push fmt_with_hp
    MOVE_CODE_AND_ADD_CODE(b, 0x0044153a, 16, HOOK_DISCARD_ORIGINAL);
}

void ui_hook_init()
{
    addChatMessageInternal_orig = (uintptr_t)hook_function(addChatMessageInternal_orig, 5, method_to_voidptr(&BfMenu::addChatMessageInternal_hook));
    addPlayerChatMessage_orig = (uintptr_t)hook_function(addPlayerChatMessage_orig, 5, method_to_voidptr(&BfMenu::addPlayerChatMessage_hook));
    setCenterKillMessage_orig = (uintptr_t)hook_function(setCenterKillMessage_orig, 5, method_to_voidptr(&BfMenu::setCenterKillMessage_hook));
    addRadioChatMessage_orig = (uintptr_t)hook_function(addRadioChatMessage_orig, 8, method_to_voidptr(&BfMenu::addRadioChatMessage_hook));

    if (g_settings.showIDInNametags) patch_add_id_to_nametag();
}
