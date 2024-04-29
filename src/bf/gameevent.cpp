#include "../pch.h"

static int currentMessagePlayerID = -1;
static int currentMessageSecondaryPlayerID = -1;

static uintptr_t getNextRcvdEvent_orig = 0x004B34B0;
__declspec(naked) GameEvent* GameEventManager::getNextRcvdEvent()
{
    _asm mov eax,getNextRcvdEvent_orig
    _asm jmp eax
}

GameEvent* GameEventManager::getNextRcvdEvent_hook()
{
    // This function is called by the game until there are no more events to process.
    // This means it will always be called one last time when it will return null
    currentMessagePlayerID = -1;
    currentMessageSecondaryPlayerID = -1;

    GameEvent* event = getNextRcvdEvent();
    if (!event) return 0;

    static bool dataBaseCompleteEventReceived = false;

    switch (event->getType()) {
        case BF_CreatePlayerEvent: {
            auto ev = reinterpret_cast<CreatePlayerEvent*>(event);
            currentMessagePlayerID = ev->playerID;
            // do not output messages while downloading the database
            if (dataBaseCompleteEventReceived && g_settings.showConnectsInChat) {
                auto message = std::string(ev->name) + " connected";
                /*if (ev->team == 1 || ev->team == 2) {
                    message += ev->team == 1 ? " (axis)" : " (allied)";
                }*/
                chatMessage(message, true, ev->team);
            }
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
                    if (g_highPrecBlindTest) {
                        if (ev->playerid == BFPlayer::getLocal()->getId()) {
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
        case BF_GameStatusEvent: {
            auto ev = reinterpret_cast<GameStatusEvent*>(event);
            if (ev->newStatus == GS_ENDMAP) {
                if (g_settings.highPrecBlindTest) {
                    chatMessage(g_highPrecBlindTest ? "High precision mode was ENABLED" : "High precision mode was DISABLED");
                    BfMenu::getSingleton()->setCenterKillMessage(g_highPrecBlindTest ? L"High precision mode was ENABLED" : L"High precision mode was DISABLED");
                    auto players = BFPlayer::getPlayers();
                    if (players) {
                        for (auto it = players->begin(); it != players->end(); ++it) {
                            std::string name = (*it)->getName();
                            name.append(g_highPrecBlindTest ? "*ENABLED" : "*DISABLED");
                            (*it)->setName(name);
                        }
                    }
                }
            }
        }
    }
    return event;

ignore_event:
    // ignore this event, process the next one
    delete event;
    return getNextRcvdEvent_hook();
}


void gameevent_hook_init()
{
    getNextRcvdEvent_orig = (uintptr_t)hook_function(getNextRcvdEvent_orig, 6, method_to_voidptr(&GameEventManager::getNextRcvdEvent_hook));
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
