#include "pch.h"

#pragma warning(disable: 4740)

int g_actionsToDrop = 0;

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
        auto ss = std::string("BF1942 v1.61; mod ") + WideStringToISO88591(get_build_version());
        s = ss;
        return &s;
    });
    inject_call(0x0045F0C9, 5, reinterpret_cast<void*>(get_version), 1);
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

void patch_fix_setting_foregroundlocktimeout()
{
    // The game sets ForegroundLockTimeout to 0 on each startup via calling SystemParametersInfo.
    // The user's profile is also updated, causing the modification to be persistent during reboots.
    // This function call is very slow, takes around half a second, modifying it to be only temporary,
    // without affecting the registry makes the call nearly instant. Disabling the call completely may
    // not be ideal, because if u click another window after BF1942.exe starts, after the window is created,
    // it will not have focus, and the game can't bring it to front to fix this.
    BEGIN_ASM_CODE(a)
        push 2 // change fWinIni parameter from SPIF_UPDATEINIFILE|SPIF_SENDCHANGE to only SPIF_SENDCHANGE
    PATCH_CODE(a, 0x00632478, 2);
}

void patch_disable_cpu_clock_measurement()
{
    // System::getMHZ - remove CPU clock speed measurement, it takes 500ms
    // This value is used in two places:
    // - Determining optimal graphical settings, probably, 2200 will force high. (462f9f)
    // - "cpu" rule query response when hosting server. (bfRulesCallback)
    // Set clock speed to a fixed value of 2200MHz ()
    BEGIN_ASM_CODE(a)
        mov eax, 2200
        ret
        nop
        nop
    PATCH_CODE(a, 0x00581d90, 8);
}

void patch_fix_MemoryPool_crash_on_loading()
{
    // This is an old memory allocator fix i was needing on an older install, probably Win7.
    // I did not document at the time exactly how it works, it modifies how MemoryPool::alloc
    // works, it changes the code to check if the pool object has a buffer first.
    patchBytes(0x00601664, {
        0x57,                                   // push    edi
        0x31, 0xDB,                             // xor     ebx, ebx
        0x3B, 0x5E, 0x1C,                       // cmp     ebx, [esi+MemoryPool.pool_data]
        0x74, 0x0E,                             // jz      short 60167A ; allocate_pool_buffer
        0x3B, 0x5E, 0x20,                       // cmp     ebx, [esi+20h]
        0x0F, 0x84, 0x91, 0x00, 0x00, 0x00,     // jz      pool_has_no_free_slot
        0x90, 0x90, 0x90,                       // nop nop nop
        0xEB, 0x7B,                             // jmp     short reserve_space_and_return ; Jump
    });
}

void patch_lower_nametags_when_close()
{
    // Patch to make nametags lower when u get close to players, left is original, right is new
    // At 20 units distance the nametag starts to gradually get lower, at 0 distance offset is y-1
    // 00440F90 - D9 44 24 24        fld dword ptr [esp+24]    00440F90 - 8B 4C 24 50        mov ecx,[esp+50] // move pos.x
    // 00440F94 - D8 44 24 44        fadd dword ptr [esp+44]   00440F94 - 89 4C 24 38        mov [esp+38],ecx
    // 00440F98 - 99                 cdq                       00440F98 - 99                 cdq
    // 00440F99 - B9 03000000        mov ecx,00000003          00440F99 - B9 03000000        mov ecx,3
    // 00440F9E - F7 F9              idiv ecx                  00440F9E - F7 F9              idiv ecx
    // 00440FA0 - D9 5C 24 7C        fstp dword ptr [esp+7C]   00440FA0 - 8B 44 24 54        mov eax,[esp+54] // move pos.y
    // 00440FA4 - D9 44 24 28        fld dword ptr [esp+28]    00440FA4 - 89 44 24 3C        mov [esp+3C],eax
    // 00440FA8 - D8 44 24 48        fadd dword ptr [esp+48]   00440FA8 - 8B 44 24 58        mov eax,[esp+58] // move pos.z
    // 00440FAC - 8B 44 24 7C        mov eax,[esp+7C]          00440FAC - 89 44 24 40        mov [esp+40],eax
    // 00440FB0 - 89 44 24 38        mov [esp+38],eax          00440FB0 - D9 44 24 14        fld dword ptr [esp+14] // load distance
    // 00440FB4 - D9 9C 24 80000000  fstp dword ptr [esp+80]   00440FB4 - D8 1D 345A8D00     fcomp dword ptr [008D5A34] { (20,00) }
    // 00440FBB - 8B 8C 24 80000000  mov ecx,[esp+00000080]    00440FBA - DFE0               fnstsw ax
    // 00440FC2 - D9 44 24 2C        fld dword ptr [esp+2C]    00440FBC - F6 C4 41           test ah,41 // distance <= 20
    // 00440FC6 - 89 4C 24 3C        mov [esp+3C],ecx          00440FBF - 74 1A              je 00440FDB // jump if false
    // 00440FCA - D8 44 24 4C        fadd dword ptr [esp+4C]   00440FC1 - D9 44 24 3C        fld dword ptr [esp+3C] // load pos.y
    // 00440FCE - 8B CB              mov ecx,ebx               00440FC5 - D9 05 345A8D00     fld dword ptr [008D5A34] { (20,00) }
    // 00440FD0 - D9 9C 24 84000000  fstp dword ptr [esp+84]   00440FCB - D9C0               fld st(0) // 20
    // 00440FD7 - 8B 14 95 14769500  mov edx,[edx*4+00957614]  00440FCD - D8 64 24 14        fsub dword ptr [esp+14] // inv_dist = 20 - distance
    // 00440FDE - 89 54 24 14        mov [esp+14],edx          00440FD1 - D9C9               fxch st(1) // swap fpu regs 0 and 1
    // 00440FE2 - 8B 94 24 84000000  mov edx,[esp+84]          00440FD3 - DEF9               fdivp st(1),st(0) // offs = 20 / inv_dist
    // 00440FE9 - 89 54 24 40        mov [esp+40],edx          00440FD5 - DEE9               fsubp st(1),st(0) // pos.y -= offs
    //                                                         00440FD7 - D9 5C 24 3C        fstp dword ptr [esp+3C] // update pos.y
    //                                                         00440FDB - B9 FAFAFAFA        mov ecx,FAFAFAFA // nop
    //                                                         00440FE0 - 8B 14 95 14769500  mov edx,[edx*4+00957614]
    //                                                         00440FE7 - 89 54 24 14        mov [esp+14],edx
    //                                                         00440FEB - 8B CB              mov ecx,ebx
    patchBytes(0x00440F90, {
        0x8B, 0x4C, 0x24, 0x50, 0x89, 0x4C, 0x24, 0x38, 0x99, 0xB9, 0x03, 0x00, 0x00, 0x00,
        0xF7, 0xF9, 0x8B, 0x44, 0x24, 0x54, 0x89, 0x44, 0x24, 0x3C, 0x8B, 0x44, 0x24, 0x58,
        0x89, 0x44, 0x24, 0x40, 0xD9, 0x44, 0x24, 0x14, 0xD8, 0x1D, 0x34, 0x5A, 0x8D, 0x00,
        0xDF, 0xE0, 0xF6, 0xC4, 0x41, 0x74, 0x1A, 0xD9, 0x44, 0x24, 0x3C, 0xD9, 0x05, 0x34,
        0x5A, 0x8D, 0x00, 0xD9, 0xC0, 0xD8, 0x64, 0x24, 0x14, 0xD9, 0xC9, 0xDE, 0xF9, 0xDE,
        0xE9, 0xD9, 0x5C, 0x24, 0x3C, 0xB9, 0xFA, 0xFA, 0xFA, 0xFA, 0x8B, 0x14, 0x95, 0x14,
        0x76, 0x95, 0x00, 0x89, 0x54, 0x24, 0x14, 0x8B, 0xCB
        });
}

void patch_WindowWin32__init_hook_for_updating()
{
    BEGIN_ASM_CODE(a)
        mov ecx, updater_wait_for_updating
        call ecx
    MOVE_CODE_AND_ADD_CODE(a, 0x00632478, 9, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_add_plus_version_to_accept_ack()
{
    // This patch adds a marker and the current mod version to the
    // CONNECT_ACCEPT_ACK packet, so a server can identify Plus clients
    // This could allow for using extra features only present in these clients.
    static auto insert_version = LAMBDA_STDCALL(void, (uint8_t * data), {
        // data[0] will be overwritten with the player id
        // high 4 bits of data[2] will be overwritten by the packet id (4)
        // ID XX PX XX XX XX XX XX
        uint8_t major, minor, patch, build;
        get_build_version_components(major, minor, patch, build);
        data[1] = '+';
        data[2] = 0x40 | ((build > 15 ? 15 : build) & 0xF); // set high 4 bits to 4 so the crc will be correct
        data[3] = major;
        data[4] = minor;
        data[5] = patch;
        data[6] = PLUS_PROTOCOL_VERSION;
        // last byte is a crc of the previous 6 bytes (1..6)
        data[7] = crc8(data + 1, 6);
    });
    BEGIN_ASM_CODE(a)
        mov eax, esp // esp points to the 8 byte buffer used for packet data in the function
        push ecx // save *this
        push eax
        mov eax, insert_version
        call eax
        pop ecx // restore *this
    MOVE_CODE_AND_ADD_CODE(a, 0x0060F393, 10, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_higher_precision_fpu()
{
    // This patch makes sure the FPU runs at double precision instead of single precision, which is
    // enforced by DirectX in the game. This may improve various calculations in the game, including
    // frame timing, and simulation. Frame timing will get more accurate. With lower precision, the game
    // usually runs a bit too fast, or too slow, which causes synchronization issues when playing on
    // servers. Simulation accuracy may improve too, because servers run with double precision too,
    // so the values calculated by the client and the server will have less difference.
    // Pass D3DCREATE_FPU_PRESERVE (bitmask 0x02) to IDirect3D8::CreateDevice.
    // See also https://stackoverflow.com/questions/12707961/switching-fpu-to-single-precision
    // 0063F217 83 C8 40                        or eax, 40h      
    patchBytes(0x0063F217, { 0x83, 0xc8, 0x42 });
    // 0063F0C1 C7 86 B8 00 00 00 20 00 00 00   mov dword ptr [esi+0B8h], 20h ; ' '
    patchBytes(0x0063F0C1, { 0xc7, 0x86, 0xb8, 0, 0, 0, 0x22, 0, 0, 0 });

    // DirectX shouldn't touch the fpu masks now, but it expects all exceptions to be masked
    // Set some values taken from a working game
    _control87(
        _EM_DENORMAL | _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT | // set all exception masks
        _RC_NEAR | // round near 
        _PC_53 | // 53 bit precision (MSVCRT default)
        _IC_PROJECTIVE, // projective infinity control
        _MCW_EM | _MCW_RC | _MCW_PC | _MCW_IC // change everything
    );
}

void patch_drop_actions()
{
    // Hook GameClient::registerPlayerAction to drop a number of player actions when requested.
    // When the global g_actionsToDrop is nonzero, a PlayerAction is dropped instead of it being
    // queued for sending to the server. The variable is also decremented.
    // Regularly dropping these packets helps in minimizing input delay between the client and
    // the server simulation. When the player dies its safe to drop some of these actions because
    // the resulting stuttering won't be noticeable, and there is no real effect to the gameplay
    // because the player just died anyway.
    // 
    // When the client sends more actions than the server can process (can easily happen if
    // the server runs slower than the client, see also FPU precision patch above),
    // the server may buffer up to 4 player actions before processing it. This can result in
    // a total input delay of about 133ms (an action is sent every 33ms). Keeping this buffer short
    // may improve gameplay.
    
    BEGIN_ASM_CODE(a)
        cmp g_actionsToDrop, 0
        jz cont
        dec g_actionsToDrop
        ret 8
    cont:
    MOVE_CODE_AND_ADD_CODE(a, 0x004904F0, 8, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_showFPS_more_precision_on_averages()
{
    // On the console.showFPS debug display, on the average fps/frame values (second column),
    // increase the number of digits after the decimal point to 3
    const char* fmt = "%.3f";
    patchBytes(0x00462826 + 1, fmt);
    patchBytes(0x004628E6 + 1, fmt);
}

void patch_key_reading_to_silently_fail()
{
    // When these patches are applied the game won't care if the registry keys containing the
    // CD key are missing, random garbage present in the target buffers will be used.
    // These patches are from the wild, collected from some exes.

    patchBytes(0x0040CD5E, { 0xeb, 0x66 }); // jnz to jmp
    patchBytes(0x00459EED, { 0xeb, 0x07 }); // jnz to jmp
    patchBytes(0x0049510A, { 0xeb, 0x20 }); // jnz to jmp

    // Disable a call to a function that tries to blacklist all keys starting with "0901"
    // The function is broken anyway because its querying the (Default) value the wrong way
    nop_bytes(0x0045831D, 5);
}

void patch_fix_mine_warning_not_going_away()
{
    // Sometimes the mine warning icon remains where previously was a mine. This happens
    // because the mine isn't reset properly when it is disabled by the server. Mines are
    // only destroyed with the kit they belong to, they are just disabled when exploding/removing.
    // ObjectManager has a projectile list that is only used (?) for displaying the mine warning,
    // if Projectile::resetProjectile is not called when the projectile is disabled, it is not removed
    // from the projectile list. Normally the server doesn't send ghost updates for disabled projectiles
    // so it never gets disabled by the network code, only by the BFSoldier::useRepairPack method.
    // If another network update is received after the wrench code disables the mine, it gets reenabled
    // and it stays in the projectile list.

    // Hook global disableObject function, which is called when an object is disabled by the network code.
    // Call Projectile::resetProjectile if its a projectile
    BEGIN_ASM_CODE(a)
        // esi contains object ptr
        mov ecx, [esi + 0x4C] // template
        mov eax, [ecx]
        call[eax + 0x0C] // template->getClassId()
        cmp eax, 0x9495 // CID_ProjectileTemplate
        jnz not_projectile
        mov ecx, esi
        mov eax, 0x00541F50 // Projectile::resetProjectile
        call eax
        not_projectile :
    MOVE_CODE_AND_ADD_CODE(a, 0x004B77C7, 5, HOOK_ADD_ORIGINAL_AFTER);
}

void patch_fix_glitchy_projectile_pickup()
{
    // In BFSoldier::useRepairPack, do not call resetProjectile when picking up projectiles if
    // we are playing on a server. The network code will remove the mine when it is picked up
    // serverside. This fixes the flashing projectile bug when picking up stuff.
    BEGIN_ASM_CODE(b)
        push ecx
        mov eax, 0x0095F8D4
        mov ecx, [eax] // g_pIGame
        mov eax, [ecx]
        push 120002 // IGameClient
        call [eax+8]
        pop ecx
        test eax,eax
        jnz skip_reset_projectile // if IGameClient != 0 skip resetProjectile()
        mov eax, 0x00541F50 // Projectile::resetProjectile
        call eax
    skip_reset_projectile:
    MOVE_CODE_AND_ADD_CODE(b, 0x004F85C7, 5, HOOK_DISCARD_ORIGINAL);
}

static void __fastcall debugcallback(unsigned int level, bfs::string* modulename, bfs::string* filename, int line, bfs::string* expression, bfs::string* message, bfs::string* p5)
{
    static auto debugLevels = std::to_array<const char*>({"DEBUG", "INFO", "WARNING", "ASSERT", "ERROR", "LOG"});
    auto levelName = level < debugLevels.size() ? debugLevels[level] : "?";

    debuglogt("[game] %s %s:%s:%d p3:%s message:'%s' expr:%s\n",
        levelName, modulename->c_str(), filename->c_str(),
        line, expression->c_str(), message->c_str(), p5->c_str());

    if (level == 4) { // error
        debuglogt("error message received, aborting! (%s)\n", message->c_str());
        handleFatalError();
    }
}

void patch_install_bf_debug_callback_handler()
{
    // Override handlers used by Debug() in the game
    // First disable functions that change or disable these
    // handlers, then install our own for each message level.
    nop_bytes(0x5821E0, 7); // disable Debug::setDebugCallback
    nop_bytes(0x582470, 7); // disable turnOffAllDebug
    // set debug callback for each message level (0..5)
    void** g_debug_callback = (void**)0x009A2320;
    for (int i = 0; i < 6; i++) g_debug_callback[i] = (void*)debugcallback;
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
    patch_fix_setting_foregroundlocktimeout();
    patch_disable_cpu_clock_measurement();
    patch_fix_MemoryPool_crash_on_loading();
    if (g_settings.lowerNametags) patch_lower_nametags_when_close();
    if (g_settings.smootherGameplay) {
        patch_higher_precision_fpu();
        patch_drop_actions();
    }

    patch_WindowWin32__init_hook_for_updating();
    patch_add_plus_version_to_accept_ack();
    patch_showFPS_more_precision_on_averages();
    patch_key_reading_to_silently_fail();
    patch_fix_mine_warning_not_going_away();
    patch_fix_glitchy_projectile_pickup();

    generic_hook_init();
    gameevent_hook_init();
    ui_hook_init();
    renderer_hook_init();

    patch_install_bf_debug_callback_handler();

    dynbuffer_make_nonwritable();
}
