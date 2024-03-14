#include "stl.h"

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

    __declspec(naked) string& string::replace(size_t pos, size_t len, string const& str)
    {
        __asm {
            mov eax, 0x008C3160
            jmp [eax]
        }
    }

}