#include "../pch.h"

__declspec(naked) BFPlayer* __stdcall BFPlayer::getFromID(int id)
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov ecx, [ecx]
        test ecx,ecx
        jnz cont
        xor eax,eax // player manager is null, return 0
        ret 4
    cont:
        mov eax, [ecx]
        jmp dword ptr [eax+0x18] // tailcall to pPlayerManager->getPlayerFromId()
    }
}