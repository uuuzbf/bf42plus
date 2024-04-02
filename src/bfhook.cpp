#include "pch.h"

#pragma warning(disable: 4740)

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
    BEGIN_ASM_CODE(mo)
        cmp eax, 99
        jl resume           ; to original code after jmp eax
        fstp st(0)          ; pop fpu stack, is this needed ?
        mov eax, 00538BDCh  ; jump to end of function, increments time to live and destroys if needed
        jmp eax
        resume :
    MOVE_CODE_AND_ADD_CODE(mo, 0x005389C9u, 5, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_scoreboard_column_widths()
{
    // fix ping display on scoreboard
    // the issue is that the ping column width is only 30 (pixel?), whereas for a proper 3 digit column, atleast 35 is needed
    // this code widens the ping column (id 6), and moves the id column (id 7) by 5 pixels, there are 3 always empty columns (8, 9, 10; all width 15)
    // before the 3 award icon columns on the far right side, column id 8 will be squished to width 10 but its unused anyway
    // there are two lists for the two teams, the patching needs to be done twice
    patchBytes(0x006DFB7E, { 0x68, 0x3b, 0x01, 0, 0 }); // push 315  axis - id column (7) - pos x
    patchBytes(0x006DFB8B, { 0x68, 0x68, 0x01, 0, 0 }); // push 360  axis - unused column (8) - pos x
    patchBytes(0x006DFAC0, { 0x68, 0x3b, 0x01, 0, 0 }); // push 315  allied - id column (7) - pos x
    patchBytes(0x006DFACD, { 0x68, 0x68, 0x01, 0, 0 }); // push 360  allied - unused column (8) - pos x
    // also widen name column by squishing the unused column between name(1) and score(3)
    patchBytes(0x006DFA7F, { 0x68, 0xAE, 0x00, 0, 0 }); // push 174  allied - unknown column (2) - pos x
    patchBytes(0x006DFB3D, { 0x68, 0xAE, 0x00, 0, 0 }); // push 174  axis - unknown column (2) - pos x
}

void patch_screen_resolution_fixes()
{
    // A different way to fix this: UserInterface__parseDisplaySettings(45f8f0) has the wrong sscanf parameter
    // It searches for game.setDisplayMode command which doesn't actually exist, it is called game.setGameDisplayMode
    // Change the referenced string to use the correct command
    // Found by tuia
    unprotect((void*)0x0045FBD0, 4);
    *(const char**)0x0045FBD0 = "game.setGameDisplayMode %d %d %d";

    // Additional patch to avoid changing resolution back to 800x600 when the game first loads the config.
    // The loaded resolution info needs to be copied to the active resolution before calling changeResolution()
    BEGIN_ASM_CODE(mo)
        mov eax,[ecx+0x40]
        mov [ecx+0x30],eax
        mov eax, [ecx + 0x44]
        mov[ecx + 0x34], eax
        mov eax, [ecx + 0x48]
        mov[ecx + 0x38], eax
        mov eax, [ecx + 0x4C]
        mov[ecx + 0x3C], eax
        mov eax,0x006B0DB0 // original call, __changeResolution
        call eax
    MOVE_CODE_AND_ADD_CODE(mo, 0x006B1083u, 5, HOOK_DISCARD_ORIGINAL);

    // menu resolution patch from https://team-simple.org/forum/viewtopic.php?id=7928 / https://bfmods.com/viewtopic.php?f=9&t=47957
    // this is already present in henk's latest patched exe but not everybody uses that
    patchBytes(0x0045DD69, { 0xE8, 0x02, 0x34, 0x25, 0x00 });
}

void patch_quicker_server_pinging_on_restart()
{
    // When a map ends, the client displays the "LOADING, PLEASE WAIT" message. During this,
    // the client waits for the server to come online before restarting itself. There is a
    // system built into the client that pings the server to accomplish this. By default
    // there is a delay of 16 seconds before this pinging starts, however servers can
    // restart much faster than that.

    // make server pinging quicker when restarting on map change
    static float delayBeforePingingStarts = 1.0f; // default 16 sec
    static float pingInterval = 2.0f; // default 3 sec
    patchBytes(0x004B7000, delayBeforePingingStarts);
    patchBytes(0x004B6F6C, pingInterval);
}

void patch_master_address()
{
    // make sure the client uses the latest master address
    const char* masteraddr = "master.bf1942.org";
    strcpy((char*)0x00957C30, masteraddr); // char[64]
    strcpy((char*)0x00957DF8, masteraddr); // char[64]
}

void patch_show_version_in_menu()
{
    auto get_version = LAMBDA_FASTCALL(bfs::string*, (bfs::string & s), {
        auto ss = std::string("BF1942 v1.61; mod ") + WideStringToASCII(get_build_version());
        s = ss;
        return &s;
    });
    inject_jmp(0x0045F0C9, 5, reinterpret_cast<void*>(get_version), 1);
    *(uint8_t*)0x0045F0C9 = 0xE8; // this changes the jmp above to a call (TODO: inject_call)
}

void patch_use_mod_in_serverlist_on_connect()
{
    // faster restart when switching mod on connecting
    // use the gameId from the server list/server query to determine if the client needs a restart
    // this way the client doesn't have to connect to the server, just to get the server's mod name and restart
    // this was implemented in vanilla bf1942, but the code is completely broken
    // InternetServerList only has 14 columns (0-13), the server list code tries to put the mod name into column 18
    // and on connecting it is getting pulled from column 15, both always fails because there is no column 18 or 15
    // to fix it: add two more zero width (data) columns: 14,15
    // and modify the code that gets the mod from the listbox to get it from column 15
    // ecx contains meme_ListBoxData*
    BEGIN_ASM_CODE(a)
        push ebx            // save ebx
        mov ebx, 0x007D42A0 // meme_ListBoxData__addColumnNoWidth
        call ebx            // meme_ListBoxData__addColumnNoWidth()
        mov ecx,[esi+0Ch]
        call ebx            // meme_ListBoxData__addColumnNoWidth()
        mov ecx,[esi+0Ch]   // meme_ListBoxData__addColumnNoWidth()
        call ebx
        pop ebx
    MOVE_CODE_AND_ADD_CODE(a, 0x0069F4AEu, 5, HOOK_DISCARD_ORIGINAL);
    // fix code that gets the gameId from the listbox to get it from column 15 instead of 18
    patchBytes(0x0069F971, { 0x6a, 0x0f }); // push 18 -> push 15
}

void patch_empty_maplist()
{
    // fix maplist sometimes empty, the MapEvent has a bool that gets stack garbage, and the client only accepts the packet if its true
    nop_bytes(0x00494DFA, 6);
}

void patch_serverlist_wrong_version_grey_servers()
{
    // patch BfMultiplayerLobby::getServerColor to ignore the server version
    // this fixes servers with wrong version showing up as grey in the serverlist
    // this greying is not needed because there are no multiple versions anymore
    // skip the compare to the version string and the check after it
    inject_jmp(0x00401784, 4, (void*)0x004017CE, 1);
}

void patch_WindowWin32__init_hook_for_updating()
{
    BEGIN_ASM_CODE(a)
        mov ecx, updater_wait_for_updating
        call ecx
    MOVE_CODE_AND_ADD_CODE(a, 0x00632478, 9, HOOK_ADD_ORIGINAL_AFTER);
}

void bfhook_init()
{
    init_hooksystem(NULL);

    patch_Particle_handleUpdate_crash();
    patch_scoreboard_column_widths();
    patch_screen_resolution_fixes();
    patch_quicker_server_pinging_on_restart();
    patch_master_address();
    patch_show_version_in_menu();
    patch_empty_maplist();
    patch_use_mod_in_serverlist_on_connect();
    patch_serverlist_wrong_version_grey_servers();
    patch_WindowWin32__init_hook_for_updating();

    gameevent_hook_init();

    dynbuffer_make_nonwritable();
}
