#pragma once
#include <stdint.h>

// gcc specific hook stuff for linux server
#ifndef WIN32
#define __stdcall __attribute__((stdcall))
                
typedef struct {
    const char* name;
    void*   funcaddr;
    int     hooksize;
    void*   hookfn;
    void**  originalfn;
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
void patch_bytes(uintptr_t addr, uint8_t* bytes, size_t length);
void* modify_call(uintptr_t addr, void* newaddr);
void inject_jmp(uintptr_t addr, size_t length, void* target, int need_unprotect);
void* hook_function(uintptr_t addr, size_t numbytes, void* hook);
void* move_code_and_add_bytes(uintptr_t addr, size_t addr_length, uint8_t* bytes, size_t bytes_length, int copy_orig, ...);
void trace_function(uintptr_t funcaddr, size_t injectbytes, void (__stdcall*tracer)(const char* name, uintptr_t* arg), const char* name);
void trace_function_fastcall(uintptr_t funcaddr, size_t injectbytes, void (__stdcall*tracer)(const char* name, uintptr_t* arg), const char* name);
