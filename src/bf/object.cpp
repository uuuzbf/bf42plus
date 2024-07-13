#include "../pch.h"

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(push)
#pragma warning(disable: 26495 4100 4410 4409 4740)

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

__declspec(naked) ObjectTemplate* __stdcall ObjectTemplateManager_getTemplate(uint32_t)
{
    _asm {
        mov ecx, 0x0097D768
        mov ecx, [ecx] // pObjectTemplateManager
        mov eax, [ecx]
        jmp [eax+0x1C] // ->getTemplate(unsigned int)
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

__declspec(naked) ObjectTemplate* __stdcall ObjectTemplate::getTemplateByName(const bfs::string& name)
{
    _asm {
        mov ecx, 0x0097D768
        mov ecx, [ecx] // pObjectTemplateManager
        mov eax, [ecx]
        jmp [eax+0x2C] // ->getTemplate(const bfs::string& name)
    }
}

#pragma warning(pop)

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

std::map<uint16_t, IObject*> staticObjects;

void addStaticObject(uint16_t id, IObject* obj)
{
    if (auto it = staticObjects.find(id); it != staticObjects.end()) {
        debuglogt("addStaticObject: id %u already exists\n", id);
        it->second->destroy();
        it->second = obj;
    }
    else {
        staticObjects.insert({ id, obj });
    }
}

bool removeStaticObject(uint16_t id)
{
    if (auto it = staticObjects.find(id); it != staticObjects.end()) {
        it->second->destroy();
        staticObjects.erase(it);
        return true;
    }
    debuglogt("removeStaticObject: id %u does not exist\n", id);
    return false;
}

IObject* getStaticObject(uint16_t id)
{
    if (auto it = staticObjects.find(id); it != staticObjects.end()) {
        return it->second;
    }
    return nullptr;
}
