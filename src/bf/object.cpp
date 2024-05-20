#include "../pch.h"

__declspec(naked) bfs::map<unsigned int, IObject*> ObjectManager_getProjectileMap()
{
    _asm {
        mov ecx, 0x0097D764
        mov ecx, [ecx] // pObjectManager
        mov eax, [ecx]
        jmp [eax+0x98] // ->getProjectileMap()
    }
}
