#include "stl.h"

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(disable: 26495 4100 4410 4409 4740)

namespace bfs {
    __declspec(naked) string::string(char const* s)
    {
        __asm {
            mov eax, 0x008C3134
            jmp [eax]
        }
    }
    __declspec(naked) string::string(char const* s, size_t count)
    {
        __asm {
            mov eax, 0x008C3220
            jmp [eax]
        }
    }

    __declspec(naked) string::string(string const&)
    {
        __asm {
            mov eax, 0x008C3114
            jmp [eax]
        }
    }

    __declspec(naked) string::string(string const&, size_t pos, size_t count)
    {
        __asm {
            mov eax, 0x008C3048
            jmp [eax]
        }
    }

    __declspec(naked) string::string(size_t count, char c)
    {
        __asm {
            mov eax, 0x008C3048
            jmp[eax]
        }
    }

    __declspec(naked) string& string::replace(size_t pos, size_t len, string const& str)
    {
        __asm {
            mov eax, 0x008C3160
            jmp [eax]
        }
    }

}