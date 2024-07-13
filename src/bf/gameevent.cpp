#include "../pch.h"

static int currentMessagePlayerID = -1;
static int currentMessageSecondaryPlayerID = -1;
static bool dataBaseCompleteEventReceived = false;
static bool setLevelEventReceived = false;

GameEventMakerMaker<CreateStaticObjectEvent> createStaticObjectEventMaker;
GameEventMakerMaker<UpdateStaticObjectEvent> updateStaticObjectEventMaker;

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(push)
#pragma warning(disable: 26495 4100 4410 4409 4740)

// __declspec(naked) GameEvent::~GameEvent()
// {
//     _asm mov eax, 0x004911B0
//     _asm jmp eax
// }

static uintptr_t getNextRcvdEvent_orig = 0x004B34B0;
__declspec(naked) GameEvent* GameEventManager::getNextRcvdEvent()
{
    _asm mov eax,getNextRcvdEvent_orig
    _asm jmp eax
}

__declspec(naked) void* __fastcall GameEvent_allocate(size_t size)
{
    _asm mov eax, 0x004A6290
    _asm jmp eax
}

__declspec(naked) void* __fastcall GameEvent_deallocate(void* ptr)
{
    _asm mov eax, 0x004869D0
    _asm jmp eax
}

__declspec(naked) bool __fastcall GameEvent_registerEventMaker(GameEventID id, GameEventMaker* e)
{
    _asm mov eax, 0x004A7D70
    _asm jmp eax
}
#pragma warning(pop)

void GameEvent::operator delete(void* ptr)
{
    GameEvent_deallocate(ptr);
}

GameEvent* GameEventManager::getNextRcvdEvent_hook()
{
    // This function is called by the game until there are no more events to process.
    // This means it will always be called one last time when it will return null
    currentMessagePlayerID = -1;
    currentMessageSecondaryPlayerID = -1;

    GameEvent* event = getNextRcvdEvent();
    if (!event) return 0;


    switch (event->getType()) {
        case BF_CreatePlayerEvent: {
            auto ev = reinterpret_cast<CreatePlayerEvent*>(event);
            currentMessagePlayerID = ev->playerID;
            break;
        }
        case BF_DataBaseCompleteEvent: {
            dataBaseCompleteEventReceived = true;
            break;
        }
        case BF_ScoreMsgEvent: {
            auto ev = reinterpret_cast<ScoreMsgEvent*>(event);
            currentMessagePlayerID = ev->playerid;
            switch (ev->eventid) {
                case SE_KILL:
                    currentMessageSecondaryPlayerID = ev->victimpid;
                    break;
                case SE_SPAWNED:
                    if (!dataBaseCompleteEventReceived) {
                        // Fix for dead players showing as alive snipers after joining a server.
                        // This happens because when the server sends the database, it sends a ScoreMsgEvent/spawned
                        // after each CreatePlayerEvent, thus marking all players as alive.
                        // This fix ignores the SPAWNED event if it is sent during connecting and the
                        // player's vehicle is a MultiPlayerFreeCamera.
                        BFPlayer* player = BFPlayer::getFromID(ev->playerid);
                        IObject* vehicle;
                        if (player && (vehicle = player->getVehicle()) && vehicle->getTemplate()->getName() == "MultiPlayerFreeCamera") {
                            goto ignore_event;
                        }
                    }
                    break;
                case SE_DEATH:
                case SE_DEATHNOMSG:
                    if (g_settings.smootherGameplay) {
                        if (ev->playerid == BFPlayer::getLocal()->getId()) {
                            // local player just died, drop some actions instead of sending it to the server
                            // see also patch_drop_actions()
                            g_actionsToDrop = 3;
                        }
                    }
                    break;
            }
            break;
        }
        case BF_DestroyPlayerEvent: {
            auto ev = reinterpret_cast<DestroyPlayerEvent*>(event);
            currentMessagePlayerID = ev->playerid;
            break;
        }
        case BF_SetTeamEvent: {
            auto ev = reinterpret_cast<SetTeamEvent*>(event);
            currentMessagePlayerID = ev->playerid;
            break;
        }
        case BF_VoteEvent: {
            auto ev = reinterpret_cast<VoteEvent*>(event);
            if (g_settings.showVoteInConsole) {
                if (ev->action == VA_START || ev->action == VA_UPDATE) {
                    BFPlayer* voter = BFPlayer::getFromID(ev->playerID);
                    if (voter) {
                        char message[64];
                        static const char* voteTypes[8] = {"map", "kick", "teamkick", 0, 0, 0, 0, 0};
                        if (ev->action == VA_START) _snprintf(message, 64, "%s started a %s vote", voter->getName().c_str(), voteTypes[ev->type]);
                        else _snprintf(message, 64, "%s voted", voter->getName().c_str());
                        BfMenu::getSingleton()->outputConsole(message);
                    }
                }
            }
            break;
        }
        case BF_WelcomeMsgEvent: {
            auto ev = reinterpret_cast<WelcomeMsgEvent*>(event);
            g_serverSettings.parseFromText(ev->message);
            break;
        }
        case BF_CreateStaticObjectEvent: {
            auto ev = reinterpret_cast<CreateStaticObjectEvent*>(event);
            auto tmpl = ObjectTemplateManager_getTemplate(ev->templateid);
            if (tmpl) {
                auto obj = tmpl->createObject();
                if (obj) {
                    if (ev->hasPositionAndRotation) {
                        // This has to be called before the rotation is set via setAbsoluteTransformation,
                        // if both the position and the rotation is set with the matrix the object will be
                        // invisible.
                        obj->setAbsolutePosition(ev->position);

                        Mat4 m = obj->getAbsoluteTransformation();
                        setRotation(m, ev->rotation);
                        //m.position = ev->position;
                        obj->setAbsoluteTransformation(m);
                    }
                    obj->updateFlags(0x40000, 0);
                    obj->init();
                    auto& pos = obj->getAbsolutePosition();
                    debuglogt("created static %u - %s at (%f, %f, %f)\n", ev->objectid, tmpl->getName().c_str(), pos.x, pos.y, pos.z);

                    addStaticObject(ev->objectid, obj);
                }
                else {
                    debuglogt("failed to create object for CreateStaticObjectEvent (%s)\n", tmpl->getName().c_str());
                }
            }
            else {
                debuglogt("received CreateStaticObjectEvent with unknown template id %u\n", ev->templateid);
            }
            
            goto ignore_event;
        }
        case BF_UpdateStaticObjectEvent: {
            auto ev = reinterpret_cast<UpdateStaticObjectEvent*>(event);
            switch (ev->action) {
                case UpdateStaticObjectEvent::USO_DELETE:{
                    removeStaticObject(ev->objectid);
                    break;
                }
                case UpdateStaticObjectEvent::USO_MOVE: {
                    if (auto obj = getStaticObject(ev->objectid); obj) {
                        obj->setAbsolutePosition(ev->newValue);
                    }
                    else debuglogt("UpdateStaticObjectEvent %s unknown id %u\n", "MOVE", ev->objectid);
                    break;
                }
                case UpdateStaticObjectEvent::USO_ROTATE: {
                    if (auto obj = getStaticObject(ev->objectid); obj) {
                        Mat4 m = obj->getAbsoluteTransformation();
                        setRotation(m, ev->newValue);
                        obj->setAbsoluteTransformation(m);
                    }
                    else debuglogt("UpdateStaticObjectEvent %s unknown id %u\n", "ROTATE", ev->objectid);
                    break;
                }
                case UpdateStaticObjectEvent::USO_SCALE: {
                    if (auto obj = getStaticObject(ev->objectid); obj) {
                        // not implemented
                    }
                    else debuglogt("UpdateStaticObjectEvent %s unknown id %u\n", "SCALE", ev->objectid);
                    break;
                }
            }
            goto ignore_event;
        }
        case BF_SpecialGameEvent: {
            auto ev = reinterpret_cast<SpecialGameEvent*>(event);
            if (!setLevelEventReceived && ev->action == 2) {
                // If we receive a SpecialGameEvent(2) before SetLevelEvent during connecting
                // then skip loading of the StaticObjects.con for the map.
                g_skipLoadingStaticObjects = true;
                debuglogt("received SpecialGameEvent(2) before SetLevelEvent, not loading StaticObjects.con\n");

                goto ignore_event;
            }
            break;
        }
        case BF_SetLevelEvent: {
            setLevelEventReceived = true;
            break;
        }
    }
    return event;

ignore_event:
    // ignore this event, process the next one
    delete event;

    return getNextRcvdEvent_hook();
}

bool CreateStaticObjectEvent::deSerialize(BitStream* bs)
{
    debuglogt("CreateStaticObjectEvent::deSerialize()\n");
    templateid = bs->readUnsigned(32);
    debuglog("templateid = %u\n", templateid);
    objectid = bs->readUnsigned(16);
    debuglog("objectid = %u\n", objectid);
    hasPositionAndRotation = bs->readBool();
    debuglog("hasPositionAndRotation = %u\n", hasPositionAndRotation);
    if (hasPositionAndRotation) {
        position = bs->readFullVector();
        rotation = bs->readFullVector();
    }
    hasScale = bs->readBool();
    debuglog("hasScale = %u\n", hasScale);
    if (hasScale) {
        scale = bs->readFullVector();
    }
    return !bs->getAndResetError();
}

bool CreateStaticObjectEvent::serialize(BitStream* bs)
{
    return false; // not implemented
}

bool UpdateStaticObjectEvent::deSerialize(BitStream* bs)
{
    objectid = bs->readUnsigned(16);
    action = (UpdateAction)bs->readUnsigned(2);
    if (action != USO_DELETE) {
        newValue = bs->readFullVector();
    }
    return !bs->getAndResetError();
}

bool UpdateStaticObjectEvent::serialize(BitStream* bs)
{
    return false; // not implemented
}

// This callback is called when the client finished
// processing a CreatePlayerEvent from the server.
static void __stdcall GameClientOnPlayerCreated(BFPlayer* player)
{
    bool isIgnored = false;

    if (g_settings.isPlayerNameIgnored(ISO88591ToWideString(player->getName()))) {
        BfMenu::getSingleton()->addToIgnoreList_orig(player->getId());
        isIgnored = true;
    }

    // do not output messages while downloading the database
    if (dataBaseCompleteEventReceived && g_settings.showConnectsInChat) {
        auto message = std::string(player->getName()) + " connecting";
        if (isIgnored) message += " [ignored]";
        chatMessage(message, true, player->getTeam());
    }
    else {
        if (isIgnored) {
            auto message = "Player " + std::string(player->getName()) + " auto-ignored";
            if (dataBaseCompleteEventReceived) chatMessage(message, true, 0);
            else BfMenu::getSingleton()->outputConsole(message);
        }
    }
}

static void patch_GameClient_player_created_callback()
{
    BEGIN_ASM_CODE(a)
        push ebp
        mov eax, GameClientOnPlayerCreated
        call eax
    MOVE_CODE_AND_ADD_CODE(a, 0x00493B07, 5, HOOK_ADD_ORIGINAL_AFTER);
}

void gameevent_hook_init()
{
    getNextRcvdEvent_orig = (uintptr_t)hook_function(getNextRcvdEvent_orig, 6, method_to_voidptr(&GameEventManager::getNextRcvdEvent_hook));

    patch_GameClient_player_created_callback();

    patchBytes(0x004B403F, {0x74, 0x0B}); // Fix crash if an invalid GameEvent id received

    GameEvent_registerEventMaker(BF_CreateStaticObjectEvent, &createStaticObjectEventMaker);
    GameEvent_registerEventMaker(BF_UpdateStaticObjectEvent, &updateStaticObjectEventMaker);

    trace_function_fastcall(0x004A7BD0, 7, function_tracer_fastcall, "?iC:GameEvent__getEventMaker");
}

int getCurrentMessagePID()
{
    return currentMessagePlayerID;
}

int getCurrentMessageSecondaryPID()
{
    return currentMessageSecondaryPlayerID;
}

void setCurrentMessagePID(int playerid)
{
    currentMessagePlayerID = playerid;
}
