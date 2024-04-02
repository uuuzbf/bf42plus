#pragma once

#ifdef __cplusplus
#include <initializer_list>
extern "C" {
#endif

#include <stdint.h>

// gcc specific hook stuff for linux server
#ifndef WIN32
#define __stdcall __attribute__((stdcall))

typedef struct {
    const char* name;
    void* funcaddr;
    int     hooksize;
    void* hookfn;
    void** originalfn;
} function_hook_t;

// register a new function hook
// parameters:
//  fnaddr: integer, address of the original function
//  size: number of bytes to copy from fnaddr when hooking
//  hname: name of hook, a function named as NAME_hook will be called instead of the function
//      and the variable NAME should be called to call the original function
//
// #define EXAMPLE(n) void (n)(void)
// typedef EXAMPLE(*example_t);
// example_t example = 0;

// EXAMPLE(example_hook)
// {
    // example();
// }
// ADD_FUNC_HOOK(0x12345678, 5, example);



#define ADD_FUNC_HOOK(fnaddr, size, hname) \
static function_hook_t  bfhook_fn_hooks_ ## hname  __attribute((used, section("bfhook_fn_hooks"))) = { \
    .name = #hname,                                         \
    .funcaddr = (void*)(fnaddr),                            \
    .hooksize = (size),                                     \
    .hookfn = hname ## _hook,                               \
    .originalfn = (void**)&hname                            \
}
#endif

enum {
    HOOK_DISCARD_ORIGINAL = 0,
    HOOK_ADD_ORIGINAL_BEFORE = 1,
    HOOK_ADD_ORIGINAL_AFTER = 2,
};

void init_hooksystem(FILE* out);
void dynbuffer_make_nonwritable();
int protect(void* addr, size_t size);
int unprotect(void* addr, size_t size);
void nop_bytes(uintptr_t addr, size_t count);
void inject_ret(uintptr_t addr, uint16_t n);
void inject_ret_value(uintptr_t addr, uint16_t n, uint32_t value);
void patch_bytes(uintptr_t addr, const uint8_t* bytes, size_t length);
void* modify_call(uintptr_t addr, void* newaddr);
void inject_jmp(uintptr_t addr, size_t length, void* target, int need_unprotect);
void* hook_function(uintptr_t addr, size_t numbytes, void* hook);
void* move_code_and_add_bytes(uintptr_t addr, size_t addr_length, const uint8_t* bytes, size_t bytes_length, int copy_orig, ...);
void trace_function(uintptr_t funcaddr, size_t injectbytes, void(__stdcall* tracer)(const char* name, uintptr_t* arg), const char* name);
void trace_function_fastcall(uintptr_t funcaddr, size_t injectbytes, void(__stdcall* tracer)(const char* name, uintptr_t* arg), const char* name);


#ifdef __cplusplus
}

template <typename... Args>
inline void* moveCodeAndAddBytes(uintptr_t addr, size_t addr_length, std::initializer_list<uint8_t> bytes, int copy_orig, Args... args)
{
    return move_code_and_add_bytes(addr, addr_length, bytes.begin(), bytes.size(), copy_orig, args...);
}

inline void patchBytes(uintptr_t addr, std::initializer_list<uint8_t> bytes)
{
    return patch_bytes(addr, bytes.begin(), bytes.size());
}

template <class T>
inline void patchBytes(uintptr_t addr, const T& value)
{
    return patch_bytes(addr, (uint8_t*)&value, sizeof(value));
}

// helper macros for casting lambda functions to various function pointers
// only stateless lambdas are supported
// example: auto ptr = LAMBDA_STDCALL(int, (), { return 4; })
// ptr is now an int(__stdcall*)() which can be used in __asm blocks too
#define LAMBDA_CDECL(rettype, args, body)    static_cast<rettype (__cdecl*)args>([]args body)
#define LAMBDA_STDCALL(rettype, args, body)    static_cast<rettype (__stdcall*) args >([] args body)
#define LAMBDA_FASTCALL(rettype, args, body)    static_cast<rettype (__fastcall*) args >([] args body)

#endif // #ifdef __cplusplus

#ifdef _MSC_VER
// special wrapper macros for MSVC for __asm blocks

// for arbitrary asm blocks:
// ASMCODE_VARS() only needs to be defined once per scope
// after that
// ASMCODE_START(name)
// <assembly code lines go here>
// ASMCODE_END(name)
// then ASMCODE_PTR_AND_SIZE() returns the start address as uint8_t* and the size of the code, comma separated
#define ASMCODE_VARS()  uint8_t* asm_start,* asm_end
#define ASMCODE_START(name) \
    goto name ## _end; \
    name ## _start: \
    _asm {

#define ASMCODE_END(name) \
    } \
    name ## _end: \
    _asm mov [asm_start], offset name ## _start; \
    _asm mov [asm_end], offset name ## _end

#define ASMCODE_PTR_AND_SIZE()  asm_start, (asm_end - asm_start)

// wrapper for move_code_and_add_bytes():
// BEGIN_ASM_CODE(uniquekey)
// <assembly code lines go here>
// MOVE_CODE_AND_ADD_CODE(uniquekey, address to modify, bytes to move at address, copy mode)
#define BEGIN_ASM_CODE(key) \
    do { \
        goto asmblock_ ## key ## _end; \
        asmblock_ ## key ## _start: \
        __asm { \

#define MOVE_CODE_AND_ADD_CODE(key, addr, addr_length, copy_orig) \
        } \
        asmblock_ ## key ## _end: \
        uint8_t* start,* end; \
        __asm { mov [start], offset asmblock_ ## key ## _start } \
        __asm { mov [end], offset asmblock_ ## key ## _end } \
        move_code_and_add_bytes((addr), (addr_length), start, end - start, (copy_orig), -1); \
    } while(0)

#endif // MSC_VER
