#include "../pch.h"

static uintptr_t getNextRcvdEvent_orig = 0x004B34B0;
__declspec(naked) GameEvent* GameEventManager::getNextRcvdEvent()
{
    _asm mov eax,getNextRcvdEvent_orig
    _asm jmp eax
}

GameEvent* GameEventManager::getNextRcvdEvent_hook()
{
    GameEvent* event = getNextRcvdEvent();
    if (!event) return 0;

    static bool dataBaseCompleteEventReceived = false;

    switch (event->getType()) {
        case BF_CreatePlayerEvent: {
            // do not output messages while downloading the database
            if (dataBaseCompleteEventReceived && g_settings.showConnectsInChat) {
                auto ev = reinterpret_cast<CreatePlayerEvent*>(event);
                auto message = std::string(ev->name) + " connected";
                if (ev->team == 1 || ev->team == 2) {
                    message += ev->team == 1 ? " (axis)" : " (allied)";
                }
                chatMessage(message, true);
            }
            break;
        }
        case BF_DataBaseCompleteEvent: {
            dataBaseCompleteEventReceived = true;
            break;
        }
    }
    return event;
}

// For casting non-virtual method pointers to void*, for hook_function
// example: hook_function(..., method_to_voidptr(&class::method);
// This is needed because C++ doesn't let you cast method pointers, but probably
// there is a better solution
// See also:
// 
__declspec(naked) void* method_to_voidptr(...) {
    _asm mov eax, [esp + 4]
    _asm ret
}
void gameevent_hook_init()
{
    getNextRcvdEvent_orig = (uintptr_t)hook_function(getNextRcvdEvent_orig, 6, method_to_voidptr(&GameEventManager::getNextRcvdEvent_hook));
}
