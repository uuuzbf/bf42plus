#pragma once
#include <stdint.h>
#include "generic.h"

enum GameEventID {
    BF_HUDTextEvent = 0x01,
    BF_CreatePlayerEvent = 0x08,
    BF_DestroyPlayerEvent = 0x0C,
    BF_VoteEvent = 0x12,
    BF_WelcomeMsgEvent = 0x17,
    BF_CreateStaticObjectEvent = 0x1C,
    BF_UpdateStaticObjectEvent = 0x1D,
    BF_GameStatusEvent = 0x24,
    BF_SpecialGameEvent = 0x27,
    BF_ChatFragmentEvent = 0x28,
    BF_ScoreMsgEvent = 0x2a,
    BF_DataBaseCompleteEvent = 0x34,
    BF_SetLevelEvent = 0x36,
    BF_SetTeamEvent = 0x39,
    BF_RadioMessageEvent = 0x3A,
};

enum ScoreEventID : uint32_t {
    SE_FLAGCAPTURE = 0,
    SE_ATTACK = 1,
    SE_DEFENCE = 2,
    SE_KILL = 3,
    SE_DEATH = 4,
    SE_DEATHNOMSG = 5,
    SE_TK = 6,
    SE_SPAWNED = 7,
    SE_OBJECTIVE = 8,
    SE_OBJECTIVETK = 9,
};

enum VoteAction : uint32_t {
    VA_START = 0,
    VA_FAILED = 1,
    VA_PASSED = 2,
    VA_UPDATE = 3,
};

enum VoteType : uint32_t {
    VT_MAP = 0,
    VT_KICK = 1,
    VT_TEAMKICK = 2,
};

enum GameStatus : uint8_t {
    GS_PLAYING = 1,
    GS_ENDGAME = 2,
    GS_PREGAME = 3,
    GS_PAUSED = 4,
    GS_ENDMAP = 5,
};

class PacketStatus;


class GameEvent {
    unsigned int sequenceNumber;
    GameEvent* nextEvent;
public:
    virtual GameEventID getType() = 0;
    virtual ~GameEvent() {};
    static void operator delete(void* ptr);
    virtual void logInfo(bool, int) {};
    virtual void eventReceivedByRemote(void*) {};
    virtual bool deSerialize(BitStream*) = 0;
    virtual bool serialize(BitStream*) = 0;
};

class IStreamManager : public IBase {
public:
    virtual int getStreamManagerId();
    virtual void setStreamManagerId(int);
    virtual bool processReceivedPacket(BitStream*);
    virtual void handlePacketStatus(PacketStatus&, bool&);
    virtual unsigned int transmit(BitStream*, PacketStatus&, unsigned int);
    virtual void setConnectionId(int);
    virtual int getError();
    virtual ~IStreamManager();
};

class GameEventManager : public IStreamManager {
public:
    GameEvent* getNextRcvdEvent();
    GameEvent* getNextRcvdEvent_hook();
};


#pragma pack(push, 1)

class CreatePlayerEvent : GameEvent {
public:
    uint8_t team;
    uint8_t spawngroup;
    bool isRemote;
    char name[32];
    uint8_t playerID;
    uint16_t playerNetworkID;
    uint16_t vehicleNetworkID;
    uint16_t cameraNetworkID;
    uint16_t kitNetworkID;
    bool isAI;
};

static_assert(sizeof(CreatePlayerEvent) == 0x39);

class ScoreMsgEvent : GameEvent {
public:
    ScoreEventID eventid;
    uint8_t playerid;
    uint8_t victimpid;
    int weapon;
    int bodypart;
};

static_assert(sizeof(ScoreMsgEvent) == 0x1A);

class ChatFragmentEvent : GameEvent {
public:
    bool firstFragment;
    bool consoleMessage;
    bool broadcast; // global chat, not team
    bool serverMessage; // yellow
    int senderID; // player id or -1
    uint8_t totalLength;
    uint8_t textLength;
    char text[16];
};

static_assert(sizeof(ChatFragmentEvent) == 0x26);

class RadioMessageEvent : GameEvent {
public:
    uint16_t messageID;
    bool broadcast; // radio instead of yelling
    uint8_t playerid;
};

static_assert(sizeof(RadioMessageEvent) == 0x10);

class DestroyPlayerEvent : GameEvent {
public:
    uint8_t playerid;
};

static_assert(sizeof(DestroyPlayerEvent) == 0x0D);

class SetTeamEvent : GameEvent {
public:
    uint8_t playerid;
    uint8_t teamid;
};

static_assert(sizeof(SetTeamEvent) == 0x0E);

class VoteEvent : GameEvent {
public:
    uint8_t target; // player ID or maplist index
    uint8_t yesCount;
    uint8_t noCount; // never used
    uint8_t votesRequired;
    uint8_t playerID; // who votes/starts the vote
    float voteTime;
    VoteAction action;
    VoteType type;
};

static_assert(sizeof(VoteEvent) == 0x1D);

class GameStatusEvent : GameEvent {
public:
    GameStatus newStatus;
    char reconnectPassword[5];
    uint8_t nextMapModnameLength;
    char nextMapModname[32];
};

static_assert(sizeof(GameStatusEvent) == 0x33);

class WelcomeMsgEvent : GameEvent {
public:
    int32_t id;
    char message[64];
    uint32_t length;
};

static_assert(sizeof(WelcomeMsgEvent) == 0x54);

class SpecialGameEvent : GameEvent {
public:
    uint8_t action;
};

static_assert(sizeof(SpecialGameEvent) == 0x0D);


class CreateStaticObjectEvent : public GameEvent {
public:
    virtual GameEventID getType() override { return BF_CreateStaticObjectEvent; };
    bool deSerialize(BitStream*) override;
    bool serialize(BitStream*) override;

    uint32_t templateid;
    uint16_t objectid;
    bool hasPositionAndRotation;
    bool hasScale;
    Pos3 position;
    Vec3 rotation;
    Vec3 scale;
};

class UpdateStaticObjectEvent : public GameEvent {
public:
    virtual GameEventID getType() override { return BF_UpdateStaticObjectEvent; };
    bool deSerialize(BitStream*) override;
    bool serialize(BitStream*) override;

    enum UpdateAction {
        USO_MOVE = 0,
        USO_ROTATE = 1,
        USO_SCALE = 2,
        USO_DELETE = 3,
    };
    uint16_t objectid;
    UpdateAction action;
    Vec3 newValue;
};

class HUDTextEvent : public GameEvent {
public:
    virtual GameEventID getType() override { return BF_HUDTextEvent; };
    bool deSerialize(BitStream*) override;
    bool serialize(BitStream*) override;

    std::wstring getTextWide() { return ISO88591ToWideString(std::string(text, length)); };

    enum TextType {
        HTT_CENTERTOP2 = 0,
        HTT_CENTERTOP3 = 1,
        HTT_DEATHMESSAGE = 2,
        HTT_CENTERYELLOW = 3,
    };
    const int TextTypeBits = 3;
    TextType type;
    size_t length;
    char text[128];
};

#pragma pack(pop)

class GameEventMaker {
public:
    // Ordering is important! If overloading is used, the virtual table will be compiled
    // in the wrong order and the game will crash when it tries to use it
    virtual GameEvent* createEventCopy(const GameEvent& event) = 0;
    virtual GameEvent* createEvent() = 0;
};

void* __fastcall GameEvent_allocate(size_t size);
bool __fastcall GameEvent_registerEventMaker(GameEventID id, GameEventMaker* e);

template <class T>
class GameEventMakerMaker : public GameEventMaker {
public:
    virtual GameEvent* createEventCopy(const GameEvent& event) override {
        T* e = reinterpret_cast<T*>(GameEvent_allocate(sizeof(T)));
        memcpy(e, &event, sizeof(T));
        debuglogt("GameEventMakerMaker::createEventCopy() -> %d\n", e->getType());
        return e;
    };
    virtual GameEvent* createEvent() override {
        T* e = reinterpret_cast<T*>(GameEvent_allocate(sizeof(T)));
        new (e) T();
        debuglogt("GameEventMakerMaker::createEvent() -> %d\n", e->getType());
        return e;
    };
};


void gameevent_hook_init();

/// <summary>
/// Returns the player ID for the current chat message.
/// This can be set at various places so when a chat message is output the player ID
/// can be prepended to the chat message.
/// If no event is processed or it has no player associated with it, it returns -1
/// </summary>
/// <returns>Player ID or -1</returns>
int getCurrentMessagePID();

/// <summary>
/// Same as getCurrentMessagePID, but used for victims in kill messages
/// </summary>
/// <returns></returns>
int getCurrentMessageSecondaryPID();

void setCurrentMessagePID(int playerid);
