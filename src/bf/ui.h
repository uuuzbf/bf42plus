#pragma once
#include "stl.h"

const uint32_t DefaultBuddyColor = 0x00ff00;

// represents an RGB color value, each value 0 .. 1
struct RGBF32 { float b, r, g; };
struct RGBAF32 { float r, g, b, a; };

// meme is Bf's menu system, it contains the various objects used by it.
namespace meme {
    // parent of Node is Object?
    class Node {
        Node* next;
    public:
        virtual ~Node() = 0;
        virtual void* getFactory() = 0;
        virtual size_t getSize() = 0;
        virtual void doDelete() = 0;
        virtual bool isOk() = 0;
        virtual bool canBeDeleted() = 0;
        // .. there are many more

    };

    class TransformNode : public Node {
        Node* transformedNode;
        Vec2 position;
        Vec2 size;
        Vec2 unknown;
    };


    class BfMap : public TransformNode {
    public:
        RGBAF32& alliedBuddyColor() { return *(RGBAF32*)((intptr_t)this + 0x164); };
        RGBAF32& axisBuddyColor() { return *(RGBAF32*)((intptr_t)this + 0x154); };
        bool isPlayerIDInBuddyList(int playerid) noexcept;
        bool isPlayerIDInBuddyList_hook(int playerid);
        bool addPlayerToBuddyListByID(int playerid) noexcept;
        void onAddBuddyButtonClicked(int selectedPlayerid);
    };
}

class BfMenu {
public:
    static BfMenu* getSingleton() { return *(BfMenu**)0x00973AB8; };
    static BFPlayer* getLocalPlayer();
    bool isInitialized() { return ((bool*)this)[0x17d]; };
    meme::BfMap* getMap() { return *(meme::BfMap**)((intptr_t)this + 0x6E4); };
    bfs::list<int>& getIgnoreList() { return *(bfs::list<int>*)((uintptr_t)this + 0x834); };

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
    void showDisconnectMessage(bfs::wstring message) noexcept; // needs testing
    void hideDisconnectMessage() noexcept;
    void setInfoMessage(bfs::string message) noexcept; // accepts a locale key, if not found the string is displayed
    void setServerMessage_orig(bfs::string message) noexcept;
    void setServerMessage(bfs::string message);
    void addToIgnoreList_orig(int playerid) noexcept;
    void addToIgnoreList(int playerid);
    void removeFromIgnoreList_orig(int playerid) noexcept;
    void removeFromIgnoreList(int playerid);
    void setStatusMessage(bfs::wstring message) noexcept;
    bool clearStatusMessage() noexcept;
};

// Set a buddy's color. The player already has to be on the game's buddy list.
// Set color to InvalidColor to remove/set to default.
void setBuddyColor(std::string name, uint32_t color, bool save);
void chatMessage(std::string message, bool status = false, int team = 0);

void __stdcall SpawnScreen_setSpawnMessage(const bfs::wstring message) noexcept;

void forceSpawnTextToShow(bool force);
void forceDisconnectMessageToShow(bool force);

void ui_hook_init();
