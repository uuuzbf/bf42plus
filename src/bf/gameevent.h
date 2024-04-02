#pragma once
#include <stdint.h>
#include "generic.h"

enum GameEventID {
    BF_CreatePlayerEvent = 0x08,
    BF_DataBaseCompleteEvent = 0x34,
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

#pragma pack(pop)


void gameevent_hook_init();
