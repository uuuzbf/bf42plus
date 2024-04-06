#pragma once
#include <stdint.h>
#include "generic.h"

enum GameEventID {
    BF_CreatePlayerEvent = 0x08,
    BF_DestroyPlayerEvent = 0x0C,
    BF_ChatFragmentEvent = 0x28,
    BF_ScoreMsgEvent = 0x2a,
    BF_DataBaseCompleteEvent = 0x34,
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

class PacketStatus;


class GameEvent {
    unsigned int sequenceNumber;
    GameEvent* nextEvent;
public:
    virtual GameEventID getType() = 0;
    virtual ~GameEvent();
    virtual void logInfo();
    virtual void eventReceivedByRemote(void*);
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

#pragma pack(pop)


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
