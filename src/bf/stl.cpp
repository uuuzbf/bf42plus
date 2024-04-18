#include "../pch.h"

// disable warnings about unreferenced parameters, uninitialized object variables, __asm blocks, ...
#pragma warning(disable: 26495 4100 4410 4409 4740)

namespace bfs {
    __declspec(naked) void* operator_new(size_t)
    {
        __asm {
            mov eax, 0x0045BAF0
            jmp eax
        }
    }

    __declspec(naked) void operator_delete(void*)
    {
        __asm {
            mov eax, 0x0045BB60
            jmp eax
        }
    }

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
            mov eax, 0x008C30BC
            jmp[eax]
        }
    }

    __declspec(naked) string::string()
    {
        __asm {
            mov eax, 0x008C3244
            jmp[eax]
        }
    }

    __declspec(naked) string::~string()
    {
        __asm {
            mov eax, 0x008C311C
            jmp[eax]
        }
    }

    __declspec(naked) const char* string::c_str() const
    {
        __asm {
            mov eax, 0x008C30DC
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

    __declspec(naked) wstring::wstring(wstring const&)
    {
        __asm {
            mov eax, 0x008C3170
            jmp [eax]
        }
    }

    __declspec(naked) wstring::wstring(wchar_t const*)
    {
        __asm {
            mov eax, 0x008C30D8
            jmp [eax]
        }
    }

    __declspec(naked) wstring::wstring()
    {
        __asm {
            mov eax, 0x008C3184
            jmp [eax]
        }
    }

    __declspec(naked) wstring::~wstring()
    {
        __asm {
            // make sure the destructor is not called on dumb objects
            mov eax,[ecx]
            cmp eax,0xff748751 // DUMB_OBJECT
            jnz dtor
            ret
            dtor:
            mov eax, 0x008C3178
            jmp [eax]
        }
    }

    const wchar_t* wstring::c_str() const
    {
        __asm {
            mov eax, 0x008C3028
            jmp [eax]
        }
    }

    __declspec(naked) wstring& wstring::replace(size_t pos, size_t length, wstring const& str)
    {
        __asm {
            mov eax, 0x008C329C
            jmp [eax]
        }
    }

    __declspec(naked) wstring& wstring::replace(size_t pos, size_t length, wchar_t const* str)
    {
        __asm {
            mov eax, 0x008C323C
            jmp [eax]
        }
    }

    __declspec(naked) wstring& wstring::append(wchar_t const* str)
    {
        __asm {
            mov eax, 0x008C30C4
            jmp [eax]
        }
    }

    __declspec(naked) wstring& wstring::operator=(wstring const& str)
    {
        __asm {
            mov eax, 0x008C30CC
            jmp [eax]
        }
    }

}