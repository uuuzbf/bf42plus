#include "pch.h"

void patch_Particle_handleUpdate_crash()
{
    // fix crash when game is minimized
    // the crash occurs in Particle::handleUpdate
    // it calculates an integer index value 0..99 based on the lifetime of the particle, and the object template
    //  has various values that change over time (gravity, drag, size, ..) in OverTimeDistribution objects.
    //  this object can only handle 100 values, when the game is minimized the particle gets stuck somehow and
    //  this index value can reach over 10k, eventually overflowing heap allocations and crashing.
    //  this bug may be responsible for glitchy particles too, when the game is maxmimized, as random stuff is 
    //  being read from the heap
    // side effect: jet particles in SW xpack are odd
    BEGIN_ASM_CODE(mo)
        test eax, 99
        jl resume           ; to original code after jmp eax
        fstp st(0)          ; pop fpu stack, is this needed ?
        mov eax, 00538BDCh  ; jump to end of function, increments time to live and destroys if needed
        jmp eax
        resume :
    MOVE_CODE_AND_ADD_CODE(mo, 0x005389C9, 5, HOOK_ADD_ORIGINAL_AFTER);
}

void bfhook_init()
{
    init_hooksystem(NULL);

    patch_Particle_handleUpdate_crash();
}
