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

__declspec(naked) bfs::map<unsigned int, IObject*>& ObjectManager_getAllRegisteredObjects()
{
    _asm {
        mov ecx, 0x0097D764
        mov ecx, [ecx] // pObjectManager
        mov eax, [ecx]
        jmp [eax+0xC4] // ->getAllRegisteredObjects()
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

int IObject::getTeam() const
{
    auto IPCO = (IPlayerControlObject*)queryInterface(IID_IPlayerControlObject);
    if (IPCO) {
        return IPCO->getTeam();
    }
    if (tmpl->getClassID() == CID_ProjectileTemplate) {
        return *(int*)((uintptr_t)this + 0x144);
    }
    return -1;
};

__declspec(naked) ObjectTemplate* __stdcall ObjectTemplate::getTemplateByName(const bfs::string& name)
{
    _asm {
        mov ecx, 0x0097D768
        mov ecx, [ecx] // pObjectTemplateManager
        mov eax, [ecx]
        jmp [eax+0x2C] // ->getTemplate(const bfs::string& name)
    }
}
