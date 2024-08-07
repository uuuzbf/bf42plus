#include "../pch.h"

bool spawnTextForced = false;
bool disconnectMessageForced = false;

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(push)
#pragma warning(disable: 26495 4100 4410 4409 4740)

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

static uintptr_t isPlayerIDInBuddyList_orig = 0x00468680;
__declspec(naked) bool meme::BfMap::isPlayerIDInBuddyList(int playerid) noexcept
{
    _asm mov eax, isPlayerIDInBuddyList_orig
    _asm jmp eax
}

__declspec(naked) bool meme::BfMap::addPlayerToBuddyListByID(int playerid) noexcept
{
    _asm mov eax, 0x0046A2A0
    _asm jmp eax
}

__declspec(naked) void BfMenu::showDisconnectMessage(bfs::wstring message) noexcept
{
    _asm mov eax, 0x006A7E90
    _asm jmp eax
}

__declspec(naked) void BfMenu::hideDisconnectMessage() noexcept
{
    _asm mov eax, 0x006A7EE0
    _asm jmp eax
}

__declspec(naked) void BfMenu::setInfoMessage(bfs::string message) noexcept
{
    _asm mov eax, 0x006A76B0
    _asm jmp eax
}

static uintptr_t setServerMessage_addr = 0x006A87D0;
__declspec(naked) void BfMenu::setServerMessage_orig(bfs::string message) noexcept
{
    _asm mov eax, setServerMessage_addr
    _asm jmp eax
}

static uintptr_t addToIgnoreList_addr = 0x006A9340;
__declspec(naked) void BfMenu::addToIgnoreList_orig(int playerid) noexcept
{
    _asm mov eax, addToIgnoreList_addr
    _asm jmp eax
}

static uintptr_t removeFromIgnoreList_addr = 0x006A90B0;
__declspec(naked) void BfMenu::removeFromIgnoreList_orig(int playerid) noexcept
{
    _asm mov eax, removeFromIgnoreList_addr
    _asm jmp eax
}

__declspec(naked) void BfMenu::setStatusMessage(bfs::wstring message) noexcept
{
    _asm mov eax, 0x006A7D80
    _asm jmp eax
}

__declspec(naked) bool BfMenu::clearStatusMessage() noexcept
{
    _asm mov eax, 0x006A7DD0
    _asm jmp eax
}

__declspec(naked) void __stdcall SpawnScreen_setSpawnMessage(const bfs::wstring message) noexcept
{
    _asm {
        mov ecx, 0x00A5F1A8
        mov ecx, [ecx]
        mov ecx, [ecx+0x30]
        mov eax, [esp+4+0x14] // message.length
        test eax,eax
        setnz al
        mov byte ptr [ecx+0x25], al
        mov eax, 0x006CCBC0
        jmp eax
    }
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

    if (type == 0 && g_settings.wrapChat.value != 0) {
        std::wstring msg(message.data(), message.size());
        while (msg.size() > g_settings.wrapChat.value) {
            // bfsmd uses \xFF, moongamers uses \x80 as spaces
            auto spaceToWrapAt = msg.find_last_of(L" \x80\xff", g_settings.wrapChat.value, 3);
            if (spaceToWrapAt == std::wstring::npos || spaceToWrapAt == 0) spaceToWrapAt = g_settings.wrapChat.value;
            addChatMessageInternal(msg.substr(0, spaceToWrapAt).c_str(), team, firstLinePos, numMessages, maxLines, age, type, isBuddy);
            msg.erase(0, spaceToWrapAt); // leave the space as first character
        }
        message = msg;
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
            if (c < ' ' || (c >= 0x7F && c < 0xA0)) data[i] = ' ';
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

void BfMenu::setServerMessage(bfs::string message)
{
    // Originally the server message is output only after it times out.
    // If another message overwrites it before that, the first message is lost.
    // Outputting on timeout is disabled with a patch, and the message is
    // outputted here immediatedly when displayed.
    outputConsole(message);

    setServerMessage_orig(message);
}

void BfMenu::addToIgnoreList(int playerid)
{
    auto& list = getIgnoreList();
    auto oldsize = list.size();

    addToIgnoreList_orig(playerid);

    // If list was modified
    if (oldsize != list.size()) {
        auto player = BFPlayer::getFromID(playerid);
        if (player) {
            auto name = ISO88591ToWideString(player->getName());
            if (name.find('"') == std::wstring::npos) {
                g_settings.ignorePlayerName(name);
            }
        }
    }
}

void BfMenu::removeFromIgnoreList(int playerid)
{
    // If not called from chat.removeFromIgnoreList, just execute the original function
    if ((uintptr_t)_ReturnAddress() != 0x00418DE4) {
        removeFromIgnoreList_orig(playerid);
    }
    else {
        // Called from chat.removeFromIgnoreList
        auto& list = getIgnoreList();
        auto oldsize = list.size();

        removeFromIgnoreList_orig(playerid);

        // If list was modified
        if (oldsize != list.size()) {
            auto player = BFPlayer::getFromID(playerid);
            if (player) {
                auto name = ISO88591ToWideString(player->getName());
                if (name.find('"') == std::wstring::npos) {
                    g_settings.unignorePlayerName(name);
                }
            }
        }
    }
}

std::map<std::string, uint32_t, StringCompareNoCase> customBuddyColors;

void setBuddyColor(std::string name, uint32_t color, bool save)
{
    // SimpleIni can't handle quotes in keys
    if (name.find('"') != std::string::npos) {
        chatMessage("Cannot set color for players with \" in their name :(");
        return;
    }

    if (color != InvalidColor && color != DefaultBuddyColor) customBuddyColors[name] = color;
    else {
        customBuddyColors.erase(name);
        color = InvalidColor;
    }

    if (save) {
        g_settings.setBuddyColor(ISO88591ToWideString(name), color);
        g_settings.save(false);
    }
}

// Checks if a playerid is a buddy and returns its buddy color and a boolean.
// returns bool | (color << 8)
uint32_t __fastcall isPlayerBuddyAndGetColor(meme::BfMap* bfMap, int playerid)
{
    if (!bfMap->isPlayerIDInBuddyList(playerid)) return 0;
    BFPlayer* player = BFPlayer::getFromID(playerid);
    uint32_t color = 0x00FF00;
    if (player) {
        auto& name = player->getName();
        auto customColor = customBuddyColors.find(name);
        if (customColor == customBuddyColors.end()) color = 0x00FF00; // default green
        else color = customColor->second;
        //crypto_generichash(reinterpret_cast<unsigned char*>(&color), sizeof(color), reinterpret_cast<const unsigned char*>(name.c_str()), name.size(), 0, 0);
    }
    return 1 | (color << 8);
}

// Checks if a playerid is a buddy and gets the buddy color into 3 floats.
bool __fastcall isPlayerBuddyAndGetColorFloat(meme::BfMap* bfMap, int playerid, float* r, float* g, float* b)
{
    uint32_t res = isPlayerBuddyAndGetColor(bfMap, playerid);
    if (!res) return false;
    *r = (float)(res >> 24) / 255.0;
    *g = (float)(res >> 16) / 255.0;
    *b = (float)(res >> 8) / 255.0;
    return true;
}

bool meme::BfMap::isPlayerIDInBuddyList_hook(int playerid)
{
    if (!isPlayerIDInBuddyList(playerid)) return false;
    auto RA = (uintptr_t)_ReturnAddress();
    if (RA == 0x0046B130 || RA == 0x0046B802) { // call from BfMap::update, allied buddy color needs patching
        auto& color = alliedBuddyColor();
        isPlayerBuddyAndGetColorFloat(this, playerid, &color.r, &color.g, &color.b);
    }
    else if (RA == 0x0046B165 || RA == 0x0046B852) {
        auto& color = axisBuddyColor();
        isPlayerBuddyAndGetColorFloat(this, playerid, &color.r, &color.g, &color.b);
    }
    return true;
}

// Scoreboard "Add Buddy" button callback is redirected here
void meme::BfMap::onAddBuddyButtonClicked(int selectedPlayerid)
{
    uint32_t res = isPlayerBuddyAndGetColor(this, selectedPlayerid);
    if (!res) {
        // player not yet in buddy list, just do what the add buddy button does originally
        addPlayerToBuddyListByID(selectedPlayerid);
    }
    else {
        // player is already a buddy
        uint32_t newcolor;
        if (GetAsyncKeyState(VK_LSHIFT) & 0x8000 || g_settings.presetBuddyColors.value.empty()) {
            // if shift is pressed while add buddy button is clicked, assign a random color to the selected buddy
            newcolor = randombytes_random() & 0xffffff;
        }
        else {
            // get the color from the higher bits of res
            uint32_t color = res >> 8;

            // cycle between some predefined colors
            const auto& predefined_colors = g_settings.presetBuddyColors.value;

            // pick the next color in the predefined colors
            // if no color is found, use the first in the list
            auto current = std::find(predefined_colors.begin(), predefined_colors.end(), color);
            if (current == predefined_colors.end()) newcolor = predefined_colors[0];
            else newcolor = predefined_colors[(std::distance(predefined_colors.begin(), current) + 1) % predefined_colors.size()];
        }
        BFPlayer* player = BFPlayer::getFromID(selectedPlayerid);
        if (player) {
            auto& name = player->getName();
            setBuddyColor(name, newcolor, true);
        }

    }
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

void patch_change_nametag_buddy_color()
{
    // patch to change buddy nametag colors
    // 004411D0 22C 50              push    eax ; playerid
    // 004411D1 230 8B CE           mov     ecx, esi ; this
    // 004411D3 230 E8 A8 74 02 00  call    meme_BfMap__isPlayerIDInBuddyList
    // 004411D8 22C 84 C0           test    al, al; Logical Compare
    // 004411DA 22C BB 00 FF 00 00  mov     ebx, 00FF00h ; green! 0xRRGGBB
    // 004411DF 22C 75 04           jnz     short loc_4411E5 ; player is buddy
    BEGIN_ASM_CODE(a)
        mov ecx, esi // meme::BfMap*
        mov edx, eax // playerid
        mov eax, isPlayerBuddyAndGetColor
        call eax
        test al,al
        jz not_buddy
        shr eax,8
        mov ebx, eax
        mov eax, 0x004411E5
        jmp eax
not_buddy:
        mov ebx, 0x00FF00
    MOVE_CODE_AND_ADD_CODE(a, 0x004411D0, 17, HOOK_DISCARD_ORIGINAL);
}

void patch_change_scoreboard_buddy_color()
{
    // patch to change buddy colors on scoreboard, color is stored in RGBF32
    // 006E081A 0D4 8B 44 24 30              mov     eax, [esp+0D4h+playerID_]
    // 006E081E 0D4 8B 0D B8 3A 97 00        mov     ecx, g_pBfMenu
    // 006E0824 0D4 8B 89 E4 06 00 00        mov     ecx, [ecx+6E4h] ; this
    // 006E082A 0D4 50                       push    eax             ; playerid
    // 006E082B 0D8 E8 50 7E D8 FF           call    meme_BfMap__isPlayerIDInBuddyList
    // 006E0830 0D4 84 C0                    test    al, al
    // 006E0832 0D4 74 18                    jz      short loc_6E084C ; Jump if Zero (ZF=1)
    // 006E0834 0D4 C7 44 24 18 00 00 00 00  mov     [esp+0D4h+rowtextcolor.r], 0
    // 006E083C 0D4 C7 44 24 1C 00 00 80 3F  mov     [esp+0D4h+rowtextcolor.g], 1.0
    // 006E0844 0D4 C7 44 24 14 00 00 00 00  mov     [esp+0D4h+rowtextcolor.b], 0
    BEGIN_ASM_CODE(a)
        mov edx,eax // playerid
        lea eax,[esp+0x14] // blue
        push eax
        add eax,8  // green
        push eax
        sub eax,4 // blue
        push eax
        mov eax, isPlayerBuddyAndGetColorFloat
        call eax
not_buddy:
    MOVE_CODE_AND_ADD_CODE(a, 0x006E082A, 34, HOOK_DISCARD_ORIGINAL);
}

void __stdcall getBuddyColorForChat(float* r, float* g, float* b)
{
    if (!isPlayerBuddyAndGetColorFloat(BfMenu::getSingleton()->getMap(), getCurrentMessagePID(), r, g, b)) {
        // something broke, not a buddy, set default buddy color
        *r = 0.0;
        *g = 1.0;
        *b = 0.0;
    }
}

void patch_change_chat_buddy_color()
{
    // patch to change buddy colors when text is added to the chat
    // 006A8A54 02C C7 44 24 58 00 00 00 00  mov     [esp+58h], 0 // red
    // 006A8A5C 02C C7 44 24 54 00 00 80 3F  mov     [esp+54h], 3F800000h // green
    // 006A8A64 02C C7 44 24 64 00 00 00 00  mov     [esp+64h], 0 // blue
    BEGIN_ASM_CODE(a)
        push eax // eax needs to be saved
        lea eax,[esp+0x68] // blue
        push eax
        sub eax,0x10 // green
        push eax
        add eax,4 // red
        push eax
        mov eax, getBuddyColorForChat
        call eax
        pop eax // restore eax
    MOVE_CODE_AND_ADD_CODE(a, 0x006A8A54, 24, HOOK_DISCARD_ORIGINAL);
}

void patch_hook_scoreboard_add_buddy_button()
{
    // redirect call from BfMap::addPlayerToBuddyListByID to our custom function
    inject_call(0x006DFF74, 5, method_to_voidptr(&meme::BfMap::onAddBuddyButtonClicked), 1);
}

void patch_optionally_disable_ui_elements()
{
    // Skip the spawnscreen setVisible call in cbDeActivateInfoMenu if disabled by server
    // This disables the opening of the spawn screen when joining a server
    BEGIN_ASM_CODE(a)
        mov al, g_serverSettings.UI.openSpawnScreenOnJoin
        test al,al
        jnz cont
        mov edx, 0x006D846B
        jmp edx
    cont:
    MOVE_CODE_AND_ADD_CODE(a, 0x006D8461, 5, HOOK_ADD_ORIGINAL_AFTER);

    // Skip the spawnscreen setVisible call when we get a DEATH/DEATHNOMSG ScoreMsgEvent for the local player
    // This disables the opening of the spawn screen when the player dies
    BEGIN_ASM_CODE(b)
        mov cl, g_serverSettings.UI.openSpawnScreenOnDeath
        test cl, cl
        jnz cont2
        mov edx, 0x004946D4
        jmp edx
    cont2:
    MOVE_CODE_AND_ADD_CODE(b, 0x004946CA, 5, HOOK_ADD_ORIGINAL_AFTER);

    // Skip calling the function that updates the spawn status text in the top middle, and opens
    // the spawn screen when the selected spawn is lost, or after a certain timeout
    BEGIN_ASM_CODE(c)
        mov al, g_serverSettings.UI.openSpawnScreenOnDeath
        test al,al
        jnz cont3
        mov al, spawnTextForced
        mov byte ptr [ecx+0x25], al // ShowSpawnText
        mov byte ptr [ecx+0x80], 0 // ShowLostSpawnPoint = 0
        retn 8
    cont3:
    MOVE_CODE_AND_ADD_CODE(c, 0x006CD610, 6, HOOK_ADD_ORIGINAL_AFTER);

    // Skip briefing window
    BEGIN_ASM_CODE(d)
        mov al, g_serverSettings.UI.skipBriefingWindow
        test al,al
        jz do_not_skip
        // Call cbDeActivateInfoMenu instead of transitioning to the briefing window
        mov eax, 0x006D8420
        jmp eax
    do_not_skip:
    MOVE_CODE_AND_ADD_CODE(d, 0x006D8740, 6, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_optionally_disable_enemy_nametags()
{
    // Check for BFSoldiers
    BEGIN_ASM_CODE(a)
        // ecx is BFPlayer* (preserve), eax, edx is free
        mov al, g_serverSettings.UI.showEnemyNametags
        test al,al
        jnz cont

        mov eax, 0x0097D76C
        mov eax, [eax] // PlayerManager*
        mov eax, [eax+0x54] // BFPlayer* localHumanPlayer
        mov eax, [eax+0xAC] // BFPlayer->team
        cmp [ecx+0xAC], eax // compare found player's team to local player's team
        jz cont
        // different team, skip player
        mov eax, 0x00441B57
        jmp eax
    cont:
    MOVE_CODE_AND_ADD_CODE(a, 0x00441AA7, 9, HOOK_ADD_ORIGINAL_AFTER);
    
    // Check for PlayerControlObjects
    BEGIN_ASM_CODE(b)
        // eax is IPlayerControlObject* (+11C, preserve), ecx, edx is free
        mov cl, g_serverSettings.UI.showEnemyNametags
        test cl, cl
        jnz cont2

        mov ecx, 0x0097D76C
        mov ecx, [ecx] // PlayerManager*
        mov ecx, [ecx+0x54] // BFPlayer* localHumanPlayer
        mov ecx, [ecx+0xAC] // BFPlayer->team
        cmp [eax+0x80], ecx // compare PlayerControlObject and local player team
        jz cont2

        // different team, skip PCO
        mov ecx, 0x00441B57
        jmp ecx

    cont2:
    MOVE_CODE_AND_ADD_CODE(b, 0x00441AF0, 7, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_custom_hit_indicator_time()
{
    // Change the value BFPlayer.HitIndicationTime is set to. Original is 1.0
    BEGIN_ASM_CODE(a)
        // Store hitIndicationTime into eax+0x1CC
        fld qword ptr [g_settings.hitIndicatorTime.value]
        mov ecx, 0x8d5a7c // address of 0.0333f
        fld dword ptr [ecx]
        fcomip st, st(1) // compare 0.03333 and hitIndicatorTime, pop 0.03333
        fldz // push 0.0
        fcmovbe st, st(1) // st = st(1) if less or equal [0.0333 <= hitIndicatorTime ? st = hitIndicatorTime]
        fstp st(1) // pop st(0), move value to st(1)
        fstp [eax+0x1CC] // store value into BFPlayer.HitIndicationTime
    MOVE_CODE_AND_ADD_CODE(a, 0x00495A24, 10, HOOK_DISCARD_ORIGINAL);

    // Divide hit indication time before moving the value to BfMenu. Originally
    // BFPlayer.HitIndicationTime is both used as a timer and for the alpha value of
    // the hit indicator. 1.0 both means 1 second and fully visible. If the initial timer
    // value is not 1, it needs to be scaled to be a correct alpha value.
    BEGIN_ASM_CODE(b)
        // BFPlayer.HitIndicationTime is in st(0), can be modified before it is stored to BfMenu.HitIndicationTime
        //fdiv qword ptr [g_settings.hitIndicatorTime.value]
        fld qword ptr [g_settings.hitIndicatorTime.value]
        mov eax, 0x8d5a7c // address of 0.0333f
        fld dword ptr [eax]
        fcomip st, st(1)
        ja set_zero // If hitIndicatorTime is < 0.0333, set BfMenu.HitIndicationTime to zero, this prevents division by zero
        fdivp st(1), st
        jmp end_b
    set_zero:
        fstp st
        fstp st
        fldz
    end_b:
    MOVE_CODE_AND_ADD_CODE(b, 0x006AEBA8, 6, HOOK_ADD_ORIGINAL_BEFORE);
}

void forceSpawnTextToShow(bool force)
{
    spawnTextForced = force;
}

void forceDisconnectMessageToShow(bool force)
{
    disconnectMessageForced = force;
}

void patch_force_texts_to_show()
{
    BEGIN_ASM_CODE(a)
        mov eax, [edx+0x30]
        mov cl, spawnTextForced
        mov byte ptr [eax+0x25], cl
    MOVE_CODE_AND_ADD_CODE(a, 0x006ADF69, 7, HOOK_DISCARD_ORIGINAL);

    BEGIN_ASM_CODE(b)
        mov al, disconnectMessageForced
        test al,al
        jnz cont
        mov eax, 0x006A7EE0
        call eax
    cont:
    MOVE_CODE_AND_ADD_CODE(b, 0x00495FD2, 5, HOOK_DISCARD_ORIGINAL);
}

void ui_hook_init()
{
    addChatMessageInternal_orig = (uintptr_t)hook_function(addChatMessageInternal_orig, 5, method_to_voidptr(&BfMenu::addChatMessageInternal_hook));
    addPlayerChatMessage_orig = (uintptr_t)hook_function(addPlayerChatMessage_orig, 5, method_to_voidptr(&BfMenu::addPlayerChatMessage_hook));
    setCenterKillMessage_orig = (uintptr_t)hook_function(setCenterKillMessage_orig, 5, method_to_voidptr(&BfMenu::setCenterKillMessage_hook));
    addRadioChatMessage_orig = (uintptr_t)hook_function(addRadioChatMessage_orig, 8, method_to_voidptr(&BfMenu::addRadioChatMessage_hook));
    setServerMessage_addr = (uintptr_t)hook_function(setServerMessage_addr, 5, method_to_voidptr(&BfMenu::setServerMessage));
    addToIgnoreList_addr = (uintptr_t)hook_function(addToIgnoreList_addr, 10, method_to_voidptr(&BfMenu::addToIgnoreList));
    removeFromIgnoreList_addr = (uintptr_t)hook_function(removeFromIgnoreList_addr, 9, method_to_voidptr(&BfMenu::removeFromIgnoreList));

    isPlayerIDInBuddyList_orig = (uintptr_t)hook_function(isPlayerIDInBuddyList_orig, 6, method_to_voidptr(&meme::BfMap::isPlayerIDInBuddyList_hook));

    if (g_settings.showIDInNametags) patch_add_id_to_nametag();

    patch_change_nametag_buddy_color();
    patch_change_scoreboard_buddy_color();
    patch_change_chat_buddy_color();
    patch_hook_scoreboard_add_buddy_button();
    patch_optionally_disable_ui_elements();
    patch_optionally_disable_enemy_nametags();
    patch_custom_hit_indicator_time();
    patch_force_texts_to_show();

    //trace_function_fastcall(0x006CCE20, 5, function_tracer_fastcall, "SpawnScreenStuff__setVisible");

    // Disable original server message outputting to console
    inject_jmp(0x006A88B1, 2, (void*)0x006A88CE, 1);
}
