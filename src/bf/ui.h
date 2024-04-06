#pragma once
#include "stl.h"

class BfMenu {
public:
    static BfMenu* getSingleton() { return *(BfMenu**)0x00973AB8; };
    static BFPlayer* getLocalPlayer();
    bool isInitialized() { return ((bool*)this)[0x17d]; };
    bfs::wstring* stringToWide(bfs::wstring* out, const bfs::string* in) noexcept;
    void addGameInfoMessage(bfs::wstring message, int team) noexcept;
    void addPlayerChatMessage(bfs::wstring message, BFPlayer* player, int team) noexcept;
    void addPlayerChatMessage_hook(bfs::wstring message, BFPlayer* player, int team);
    void addRadioChatMessage(bfs::wstring message, BFPlayer* player, int team) noexcept;
    void addRadioChatMessage_hook(bfs::wstring message, BFPlayer* player, int team);
    void addChatMessageInternal(bfs::wstring message, int team, int firstLinePos, int* numMessages, int maxLines, int* age, int type, bool isBuddy) noexcept;
    void addChatMessageInternal_hook(bfs::wstring message, int team, int firstLinePos, int* numMessages, int maxLines, int* age, int type, bool isBuddy);
    void setCenterKillMessage(bfs::wstring message) noexcept;
    void setCenterKillMessage_hook(bfs::wstring message);
    void outputConsole(bfs::string message) noexcept;
};


void chatMessage(std::string message, bool status = false, int team = 0);

void ui_hook_init();
