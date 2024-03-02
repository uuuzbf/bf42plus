#include "pch.h"


void bfhook_init()
{
    init_hooksystem(NULL);

    // fix crash when game is minimized
    // the crash occurs in Particle::handleUpdate
    // it calculates an integer index value 0..99 based on the lifetime of the particle, and the object template
    //  has various values that change over time (gravity, drag, size, ..) in OverTimeDistribution objects.
    //  this object can only handle 100 values, when the game is minimized the particle gets stuck somehow and
    //  this index value can reach over 10k, eventually overflowing heap allocations and crashing.
    //  this bug may be responsible for glitchy particles too, when the game is maxmimized, as random stuff is 
    //  being read from the heap
    // side effect: jet particles in SW xpack are odd
    moveCodeAndAddBytes(0x005389C9, 5, {
        0xa9, 0x63, 0x00, 0x00, 0x00,   // test eax,00000063 { 99 }
        0x7c, 0x09,                     // jl +09               ; to original code after jmp eax
        0xdd, 0xd8,                     // fstp st(0)           ; pop fpu stack, is this needed?
        0xb8, 0xdc, 0x8b, 0x53, 0x00,   // mov eax,00538BDC     ; jump to end of function, increments time to live and destroys if needed
        0xff, 0xe0,                     // jmp eax
    }, HOOK_ADD_ORIGINAL_AFTER, -1);
}
