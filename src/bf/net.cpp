#include "../pch.h"

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(push)
#pragma warning(disable: 26495 4100 4410 4409 4740)

bool __declspec(naked) BitStream::readBits(void* ptr, uint32_t bits)
{
    _asm mov eax, 0x00582610;
    _asm jmp eax;
}

void __declspec(naked) BitStream::writeBool(bool) {
    _asm mov eax, 0x00582AB0;
    _asm jmp eax;
}

bool __declspec(naked) BitStream::readBool() {
    _asm mov eax, 0x00582AF0;
    _asm jmp eax;
}

void __declspec(naked) BitStream::writeUnsigned(uint32_t value, int bits) {
    _asm mov eax, 0x005829C0;
    _asm jmp eax;
}

uint32_t __declspec(naked) BitStream::readUnsigned(int bits) {
    _asm mov eax, 0x005829E0;
    _asm jmp eax;
}

void __declspec(naked) BitStream::writeFullVector(Vec3 const& vector) {
    _asm mov eax, 0x00582C90;
    _asm jmp eax;
}

Vec3 __declspec(naked) BitStream::readFullVector() {
    _asm mov eax, 0x00583180;
    _asm jmp eax;
}

#pragma warning(pop)
