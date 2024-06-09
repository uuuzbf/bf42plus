#include "../pch.h"

__declspec(naked) bfs::map<unsigned int, IObject*>& ObjectManager_getProjectileMap()
{
    _asm {
        mov ecx, 0x0097D764
        mov ecx, [ecx] // pObjectManager
        mov eax, [ecx]
        jmp [eax+0x98] // ->getProjectileMap()
    }
}

__declspec(naked) bfs::map<unsigned int, IObject*>& ObjectManager_getSupplyDepotMap()
{
    _asm {
        mov ecx, 0x0097D764
        mov ecx, [ecx] // pObjectManager
        mov eax, [ecx]
        jmp [eax+0xB0] // ->getSupplyDepotMap()
    }
}

__declspec(naked) bfs::vector<IObject*>& ObjectManager_getControlPointVector()
{
    _asm {
        mov ecx, 0x0097D764
        mov ecx, [ecx] // pObjectManager
        mov eax, [ecx]
        jmp [eax+0x7C] // ->getControlPointVector()
    }
}

__declspec(naked) bool IObject::hasMobilePhysics() const
{
    _asm {
        mov ecx, [ecx+0x60] // physicsNode
        test ecx,ecx
        jz no_physics
        mov eax, [ecx]
        jmp [eax+0xe0] // getIsMobile
    no_physics:
        xor eax, eax
        ret
    }
}
