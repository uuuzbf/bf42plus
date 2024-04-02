#include "../pch.h"
#include "generic.h"

__declspec(naked) void __stdcall UIStuff__debugOutputMessage(bfs::string message, int type) noexcept
{
    _asm {
        mov ecx, [0x00973AB8]
        mov ecx, [ecx]
        test ecx,ecx
        jnz obj_valid
fail:
        // destruct message here!
        lea ecx,[esp+4]
        lea eax,bfs::string::~string
        call eax
        ret 20h
obj_valid:
        mov al,[ecx+0x17d]
        test al,al
        jz fail
        mov eax,0x006A91D0
        jmp eax
    }
}

void chatMessage(std::string message, bool status)
{
    UIStuff__debugOutputMessage(message, status ? 2 : 1);
}
