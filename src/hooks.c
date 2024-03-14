#ifndef WIN32
#include <sys/mman.h>
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "hooks.h"

#define DEBUGOUTPUT 1

static void* dynbuffer = 0;
static int dynbuffer_used = 0;
static int pagesize;
static FILE* logger = 0;

#ifndef WIN32
#define section_foreach_entry(section_name, type_t, elem)    \
    for (type_t *elem =                                      \
           ({                                                \
               extern type_t __start_##section_name;         \
               &__start_##section_name;                      \
           });                                               \
           elem !=                                           \
           ({                                                \
               extern type_t __stop_##section_name;          \
               &__stop_##section_name;                       \
           });                                               \
           ++elem)
#endif


void init_hooksystem(FILE* out)
{
    logger = out;
#ifdef WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    pagesize = si.dwAllocationGranularity;
#else
    pagesize = sysconf(_SC_PAGE_SIZE);
    
    section_foreach_entry(bfhook_fn_hooks, function_hook_t, fhook)
    {
        void* origcall = hook_function((uintptr_t)fhook->funcaddr, fhook->hooksize, fhook->hookfn);
        if(origcall)
        {
            *fhook->originalfn = origcall;
        }
        else
        {
            if(logger)fprintf(logger, "%s function hook failed\n", fhook->name);
        }
    }
#endif
}

int protect(void* addr, size_t size)
{
#ifdef WIN32
    DWORD oldprot;
    if(VirtualProtect(addr, size, PAGE_EXECUTE_READ, &oldprot)) {
        return 1;
    }
    return 0;
#else
    uintptr_t start = (uintptr_t)addr;
    uintptr_t pageaddr = start & -pagesize;
    int protsize = (start + size) - pageaddr;
    if(mprotect((void*)pageaddr, protsize, PROT_READ|PROT_EXEC))
    {
        if(logger)fprintf(logger, "failed to protect %p size %d\n", addr, size);
        perror("protect: mprotect");
        return 0;
    }
    return 1;
#endif
}

static void* get_rwxbytes(int size)
{
    if(!dynbuffer || dynbuffer_used + size > pagesize)
    {
        void* previous = dynbuffer;
#ifdef WIN32
        dynbuffer = VirtualAlloc(0, pagesize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if(!dynbuffer)
        {
            if(logger)fprintf(logger, "get_rwxbytes failed\n");
            return 0;
        }
#else
        dynbuffer = mmap(NULL, pagesize, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if(!dynbuffer)
        {
            perror("get_rwxbytes: mmap");
            return 0;
        }
#endif
        *(void**)dynbuffer = previous;
        dynbuffer_used = sizeof(void*);
    }
    
    void* addr = (char*)dynbuffer + dynbuffer_used;
    dynbuffer_used += size;
    return addr;
}

void dynbuffer_make_nonwritable(){
    for(void* buffer = dynbuffer; buffer != 0; buffer = *(void**)buffer){
    #ifdef WIN32
        DWORD oldprot;
        if(!VirtualProtect(buffer, pagesize, PAGE_EXECUTE_READ, &oldprot)) {
    #else
        if(mprotect(buffer, pagesize, PROT_READ|PROT_EXEC)) {
    #endif
            if(logger)fprintf(logger, "failed to change protection flag on dynbuffer %p\n", buffer);
        }
    }
}

int unprotect(void* addr, size_t size)
{
#ifdef WIN32
    DWORD oldprot;
    if(VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &oldprot)) {
        return 1;
    }
    return 0;
#else
    uintptr_t start = (uintptr_t)addr;
    uintptr_t pageaddr = start & -pagesize;
    int protsize = (start + size) - pageaddr;
    if(mprotect((void*)pageaddr, protsize, PROT_READ|PROT_WRITE|PROT_EXEC))
    {
        if(logger)fprintf(logger, "failed to unprotect %p size %d\n", addr, size);
        perror("unprotect: mprotect");
        return 0;
    }
    return 1;
#endif
}

void nop_bytes(uintptr_t addr, size_t count)
{
    if(!unprotect((void*)addr, count))
    {
        if(logger)fprintf(logger, "nop_bytes: unprotect failed\n");
        return;
    }
    memset((void*)addr, 0x90, count);
}

// inject a ret instruction (1 byte)
// if n != 0 then inject a "ret n" (3 bytes)
void inject_ret(uintptr_t addr, uint16_t n){
    if(!unprotect((void*)addr, 3))
    {
        if(logger)fprintf(logger, "inject_ret: unprotect failed\n");
        return;
    }
    if(n == 0)
    {
        *(uint8_t*)addr = 0xC3; // ret
    }
    else
    {
        *(uint8_t*)addr = 0xC2; // ret
        *(uint16_t*)(addr + 1) = n;
    }
}

// inject a mov to eax and a ret (3 .. 8 bytes)
//  value == 0 -> 2 bytes
//  value == 1 -> 3 bytes
//  else 5 bytes
//  plus the ret instruction (1 or 3 bytes)
void inject_ret_value(uintptr_t addr, uint16_t n, uint32_t value)
{
    if(!unprotect((void*)addr, 8))
    {
        if(logger)fprintf(logger, "inject_ret_value: unprotect failed\n");
        return;
    }
    if(value == 0 || value == 1)
    {
        *(uint16_t*)addr = 0xC031; // xor eax,eax
        addr = addr + 2;
        if(value == 1)
        {
            *(uint8_t*)addr = 0x40; // inc eax
            addr = addr + 1;
        }
    }
    else
    {
        *(uint8_t*)addr = 0xB8; // mov eax, value
        addr = addr + 1;
        *(uint32_t*)addr = value;
        addr = addr + 4;
    }
    inject_ret(addr, n);
}

void patch_bytes(uintptr_t addr, const uint8_t* bytes, size_t length)
{
    if(!unprotect((void*)addr, length))
    {
        if(logger)fprintf(logger, "patch_bytes: unprotect failed\n");
        return;
    }
    memcpy((void*)addr, bytes, length);
} 

// change the target of a call instruction
// return the old target address
void* modify_call(uintptr_t addr, void* newaddr)
{
    for(uint8_t b = *(uint8_t*)addr; b != 0xE8;) // this feels wrong
    {
        if(logger)fprintf(logger, "modify_call: unknown byte %02X at %08X, ignoring\n", b, addr);
        return 0;
    }
    uintptr_t* aoffs = (void*)(addr + 1);
    if(!unprotect(aoffs, 4))
    {
        if(logger)fprintf(logger, "modify_call: unprotect failed\n");
        return 0;
    }
    uintptr_t base = addr + 5;
    void* oldaddr = (void*)(base + *aoffs);
    *aoffs = (uintptr_t)newaddr - base;
    if(DEBUGOUTPUT && logger)fprintf(logger, "modify_call: %08X modified from %p to offset %08X\n", addr, oldaddr, *aoffs);
    return oldaddr;
}

// add a jmp at addr to target
// length must be atleast 5
// if length is larger than 5, the remaining bytes are replaced with nop
void inject_jmp(uintptr_t addr, size_t length, void* target, int need_unprotect)
{
    if(need_unprotect)
    {
        if(!unprotect((void*)addr, length))
        {
            if(logger)fprintf(logger, "inject_jmp: unprotect failed\n");
            return;
        }
    }
    
    // try short jump
    uintptr_t after_jmp = addr + 2;
    intptr_t offset = (intptr_t)((uintptr_t)target - after_jmp);
    size_t jumplen;
    if(offset < 128 && offset > -129) { // short jmp possible
        if(length < 2){
            if(logger)fprintf(logger, "inject_jmp at %08X: length must be atleast %d\n", addr, 2);
            return;
        }
        jumplen = 2;
        *(uint8_t*)addr = 0xEB; // short jump
        *(int8_t*)(addr + 1) = (int8_t)offset;
    }
    else {
        if(length < 5){
            if(logger)fprintf(logger, "inject_jmp at %08X: length must be atleast %d\n", addr, 5);
            return;
        }
        jumplen = 5;
        after_jmp = (intptr_t)addr + 5;
        *(uint8_t*)addr = 0xE9; // relative long jump
        // need to recalculate offset here because different target!
        *(intptr_t*)((intptr_t)addr + 1) = (intptr_t)((uintptr_t)target - after_jmp);
    }
    nop_bytes(after_jmp, length - jumplen);
}



/*
    longjump: E9 XX XX XX XX (offset)
    jump to pointer: FF 25 XX XX XX XX (jmp [addr])
*/

/*
    redirect a function to a hook function, call the returned address to call the original
*/
void* hook_function(uintptr_t addr, size_t numbytes, void* hook)
{
    if(DEBUGOUTPUT && logger)fprintf(logger, "hook_function(%08X, %zu, %p)\n", addr, numbytes, hook);
    if(numbytes < 5)
    {
        if(logger)fprintf(logger, "hook_function failed, numbytes too small, addr %08X numbytes %zu hook %p\n", addr, numbytes, hook);
        return 0;
    }
    // set up a small buffer where the function start will be copied
    // calling this will call the original function
    void* f = get_rwxbytes(numbytes + 5); // + longjump
    if(!f)
    {
        if(logger)fprintf(logger, "hook_function: get_rwxbytes failed\n");
        return 0;
    }
    memcpy(f, (void*)addr, numbytes);
    // add a jmp to the end of the buffer, to the rest of the original function
    inject_jmp((uintptr_t)f + numbytes, 5, (void*)(addr + numbytes), 0);

    // replace start of the original function with a jump to the hook function, nop the remaining bytes
    inject_jmp(addr, numbytes, hook, 1);

    
    return f;
}

// if copy_orig == 0, ignore original code
// if copy_orig == 1, copy original code before injected code
// if copy_orig == 2, copy original code after injected code
// originaladdress:
// jmp newmemory
// newmemory:
// <bytes>
// <content of original address>
// jmp <address after addr+addr_length>
// vararg: int offset, void* value, patch new code at offset to value, -1 offset terminates patch list
void* move_code_and_add_bytes(uintptr_t addr, size_t addr_length, const uint8_t* bytes, size_t bytes_length, int copy_orig, ...)
{
    // get a buffer
    size_t buffer_size = addr_length + bytes_length + 5;
    void* buffer = get_rwxbytes(buffer_size);
    size_t offset = 0;
    // maybe copy the original code
    if(copy_orig == HOOK_ADD_ORIGINAL_BEFORE) {
        memcpy((uint8_t*)buffer + offset, (void*)addr, addr_length);
        offset += addr_length;
    }
    // copy the new code
    memcpy((uint8_t*)buffer + offset, bytes, bytes_length);
    offset += bytes_length;
    // maybe copy the original code
    if(copy_orig == HOOK_ADD_ORIGINAL_AFTER) {
        memcpy((uint8_t*)buffer + offset, (void*)addr, addr_length);
        offset += addr_length;
    }
    // after the original code, add a jmp back to the end of the original location
    inject_jmp((uintptr_t)buffer + offset, 5, (uint8_t*)addr + addr_length, 0);
    // add a jmp to the original location to the start of the buffer
    inject_jmp(addr, addr_length, buffer, 1);
    
    va_list va;
    va_start(va, copy_orig);
    for(int patchoffset; (patchoffset = va_arg(va, int)) != -1;){
        if((size_t)patchoffset > bytes_length){
            if(logger){
                fprintf(logger, "move_code_and_add_bytes patch offset %d out of bounds\n", patchoffset);
                fflush(logger);
            }
            break;
        }
        if(copy_orig == HOOK_ADD_ORIGINAL_BEFORE) patchoffset += addr_length;
        void* value = va_arg(va, void*);
        *(void**)((uint8_t*)buffer + patchoffset) = value;
    }
    va_end(va);
    
    if(DEBUGOUTPUT && logger)fprintf(logger, "move_code_and_add_bytes: from %08X[size %zu] to %p[size %zu] copy_orig=%d\n", addr, addr_length, buffer, buffer_size, copy_orig);
    
    return buffer;
}

// tracer function is called every time the given function is called
// tracer args:
//  name: pointer to a const char* given to the trace_function
//  arg[0] contents of ecx (this ptr or garbage)
//  arg[1] return address
//  arg[2...] function parameters
void trace_function(uintptr_t funcaddr, size_t injectbytes, void (__stdcall*tracer)(const char* name, uintptr_t* arg), const char* name){
    move_code_and_add_bytes(funcaddr, injectbytes, (uint8_t[]){
        0x51,                           // push ecx
        0x54,                           // push esp
        0x68, 0xfa, 0xfa, 0xfa, 0xfa,   // push fafafafa <- name ptr goes here
        0xb8, 0xfa, 0xfa, 0xfa, 0xfa,   // mov eax,fafafafa <- trace function ptr goes here
        0xff, 0xd0,                     // call eax  <- call tracer function
        0x59,                           // pop ecx
    }, 15, HOOK_ADD_ORIGINAL_AFTER, 3, name, 8, tracer, -1);
}
// same but fastcall variant, arg[1] contains edx, arg[2] return address, ...
void trace_function_fastcall(uintptr_t funcaddr, size_t injectbytes, void (__stdcall*tracer)(const char* name, uintptr_t* arg), const char* name){
    move_code_and_add_bytes(funcaddr, injectbytes, (uint8_t[]){
        0x52,                           // push edx
        0x51,                           // push ecx
        0x54,                           // push esp
        0x68, 0xfa, 0xfa, 0xfa, 0xfa,   // push fafafafa <- name ptr goes here
        0xb8, 0xfa, 0xfa, 0xfa, 0xfa,   // mov eax,fafafafa <- trace function ptr goes here
        0xff, 0xd0,                     // call eax  <- call tracer function
        0x59,                           // pop ecx
        0x5a,                           // pop edx
    }, 17, HOOK_ADD_ORIGINAL_AFTER, 4, name, 9, tracer, -1);
}

/*
typedef uint32_t(orig_t)();
typedef void (__stdcall detourfn_t)(uintptr_t* args, orig_t* originalfn);
void detour_function(void* funcaddr, size_t injectbytes, detourfn_t* detourfn){
    
}
*/
/*  * args points here
     [retaddr] [ecx] [edx] [arg1] [arg2] ...
pop eax ; = retaddr
push edx ; save reg for arg
push ecx ; save reg for arg
push eax ; 
mov ecx,esp
push 12345678 ; address to call originalfn
push ecx ; args ptr
mov eax,[esp+0x10]
push eax ; return address for detour function
mov eax,12345678 ; address of detour function
jmp eax
;;; code that calls the original function
pop eax ; this is the return address into the detour
pop ecx
pop edx
push eax
jmp where the original code is

*/

