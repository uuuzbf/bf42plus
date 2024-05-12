#include "../pch.h"

__declspec(naked) BFPlayer* __stdcall BFPlayer::getFromID(int id)
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov ecx, [ecx]
        test ecx,ecx
        jnz cont
        xor eax,eax // player manager is null, return 0
        ret 4
    cont:
        mov eax, [ecx]
        jmp dword ptr [eax+0x18] // tailcall to pPlayerManager->getPlayerFromId()
    }
}

__declspec(naked )BFPlayer* BFPlayer::getLocal()
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov ecx, [ecx]
        test ecx, ecx
        jnz cont
        xor eax, eax // player manager is null, return 0
        ret
    cont:
        mov eax, [ecx]
        jmp dword ptr[eax+0x20] // tailcall to pPlayerManager->getLocalHumanPlayer()
    }
}

__declspec(naked) bfs::list<BFPlayer*>* BFPlayer::getPlayers()
{
    __asm {
        mov ecx, 0x0097D76C // pPlayerManager
        mov eax, [ecx]
        test eax, eax
        jz return
        add eax,0xC // PlayerManager->playerList
    return:
        ret
    }
}

__declspec(naked) uint32_t __fastcall calcStringHashValueNoCase(const bfs::string& str)
{
    _asm mov eax, 0x00502E60
    _asm jmp eax
}

__declspec(naked) void __fastcall MD5Digest(const void* data, unsigned int length, char* outputHexString)
{
    _asm mov eax, 0x00473430
    _asm jmp eax
}

static uintptr_t addPlayerInput_addr = 0x0040ECB0;
__declspec(naked) void Game::addPlayerInput_orig(int playerid, PlayerInput* input) noexcept
{
    _asm mov eax, addPlayerInput_addr
    _asm jmp eax
}

static uintptr_t getStringFromRegistry_addr = 0x00443F10;
__declspec(naked) bool __stdcall getStringFromRegistry(const char* key, const char* valueName, char* output, size_t* outlength)
{
    _asm mov eax, getStringFromRegistry_addr
    _asm jmp eax
}

void Game::addPlayerInput_hook(int playerid, PlayerInput* input)
{
    // input can be modified here, the object will not be used after this function returns

    // The following is a fix for the issue that when stainding on-foot, the mouse
    // sensitivity is half compared to when you are moving. This is caused by some bug
    // in BFSoldier::handlePlayerInput, but fixing it there would cause desync when playing
    // on servers (because the server code has the same bug). This workaround halves the
    // LookX sensitivity when the player is moving (forward or strafe inputs arent 0).
    // This is to match the vertical sensitivity, which used to be half of the horizontal
    // when moving.
    if (g_settings.correctedLookSensitivity) {
        auto localPlayer = BFPlayer::getLocal();
        if (localPlayer && localPlayer->getId() == playerid) {
            auto vehicle = localPlayer->getVehicle();
            // is player in BFSoldier?
            if (vehicle && vehicle->getTemplate()->getClassID() == CID_BFSoldierTemplate) {
                // moving? (forward or strafe arent 0)
                if (input->controls[3] != 0.0 || input->controls[0] != 0.0) {
                    // halve look left/right sensitivity
                    input->controls[4] /= 2;
                }
            }
        }
    }

    addPlayerInput_orig(playerid, input);
}

// List of keyhashes belonging to CD-keys available online.
// Playing with these CD keys may cause issues because these
// may be banned on some servers. The keys these keyhashes are
// for were collected from various websites over the internet.
// This list is only checked if the key is 22 characters and starts
// with a number. The list must be ordered because it is binary-searched.
static constexpr auto publicKeyhashes = std::to_array<const char*>({
    "03cc24068f2d6ace6df5dfbe33192f79", "053a82bf31d0f63e669e9035bea4f311", "07c55b2e546cffd5bb4f651011278a22", "0c531ca3a131c03bbd46c51080f1766a", "0cdc91fca51974eec0d543adfc599184",
    "1771923a84b5e49cf5d9bd9604ca9270", "1aa1d4e206ca2fc392ff0036b95f763f", "1f05a510cf7702a1b9f55bc019ae0508", "21f2ccf610e9501bc97e8ac1229ab6ac", "22ec0d18115c7a18ca7a16be51ebbfdb",
    "255261c6d23ceb079f12a09308a64405", "2c4eeb9cfdea0967453aa5f581762275", "358f5c7ebd91b056945870e42afb2931", "3605e2971404916661b2827645b53f29", "362c3da00a64548006e57b52f9622643",
    "36c48983173ceb22aabbee02c6f6a257", "3bd7672544d559ff2a35ab514e71e181", "3bec87f93d82b64673c45525e810cc05", "4104105c7364bcc93dff356b2551b940", "41c0331a6016e1fc79fb67124596ee00",
    "455fd2eee9e12c8b60d66dd2a744effd", "46694c558edbdc4ae977fefd187da121", "478426276878db12b2efb0e860a5198f", "489dd73074a2e01f2987d54831e202f1", "4f944cad3f8dfa5659d1839fc581eb25",
    "53c0468bae3da7372eea1635fdd715a3", "586083292728d3862ddfca42f3e7eca8", "599694ad90632be71afdef6fafbae4d9", "5e635b66a96f675dea234c08e3eacb9e", "5e95075828fd6bc0d7e40741b4858f1c",
    "61aaca8b58632eacb188fddfb31a5720", "630baa5117672fafa76f29f660ed501b", "659135be470b0dafa60c33606ae35af9", "6af8e245e6314e304576c6f5b09f0c9a", "70d783189d8ad1b55996795946a6eae3",
    "74e6f15c7754719e8024e857826196ea", "75e93c39cd97b00697e55b987bf474ae", "76b2d8fbdf7436db915a9ffb2d509e28", "794963c6cad8f381c9ca1d108040d735", "808e63e60bdd5772f29423c47b8e86aa",
    "8384f4dd8573ef62b5df82aa89ef7adb", "880053b83daaf58661e9c0c89c04de2f", "89b9665968aeac6a4e7025eac08c3d46", "8a0778fc9f844a9a52433c1a7afaaf3a", "923ba7463fc02e59ba16afd2220c643f",
    "9378afca20de945b01558e95c0404a7d", "94f8f7abda5fef546a48b3c88a22f53f", "9526e11d8e7a5eeb474641aac778b61f", "954a111fa6633df3b95418c19290a545", "9865dfb34f393cfddf2209ed3140c859",
    "9bc97e57ba68034e898fa3b3c93bd054", "9e5d15c8c748741e22ee93e1671bd1b9", "aa73b06e45f8e0df52e9aa0b44f41869", "b4dc974dde83dc764dc5e2979a207bfb", "b62539b6578e540b4c649e7d2ca24d75",
    "b83d0fc147343db4da01fac3ba250358", "ba3428336c05da461add6867cddd90cc", "c453a3069dec719447a9af5c1ffd4076", "c5bb5befd67806eb46247b83d7695eb1", "c98aca307b0ed492373a60333ece3ea0",
    "cfb7aaad39880ad4a5d6734935f80e13", "d5ced1c6328765407786032ca0f700d6", "d64ab2b4383d6e1193051a2cb6965577", "d9feb98e55f2b76f5add2561a8994e4b", "e14156f380f9cc5c41bf85dfaf5c936b",
    "ea185cc2d00e31972111ebd125ebaddd", "f2786e4c32b8c15feff542b65d88b7cd", "f412d71ec3831bfb45e5be1cd0f6f0ac", "fb5fbc503ea25b044106f93e14c34f05", "fddbcf8f859d572031760fdee346de0d",
});

// This function reads a value from HKLM, it is used by the game to read the CD key out of registry.
// This is a Setup method but the "this" object pointer is not used so it can be called as __stdcall
// outlength includes the string zero terminator
bool __stdcall getStringFromRegistry_hook(const char* key, const char* valueName, char* output, size_t* outlength)
{
    bool ok = getStringFromRegistry(key, valueName, output, outlength);

    // Make sure a CD key is being retrieved
    size_t keylength = strlen(key);
    if (keylength < 6 || strcmp(key + (keylength - 5), "\\ergc") != 0 || *valueName != 0) {
        // Not a CD key is being retrieved, this never actually happens
        return ok;
    }

    bool useMachineGuid = false;
    if (!ok) {
        // Reading CD key from registry failed, use alternate methods for obtaining an unique ID
        useMachineGuid = true;
        ok = true;
    }
    else {
        // Check if the key is a public key.
        // Length is always 22, all characters are numbers, checking the first character for
        // a number should be enough.
        // outlength always includes the zero terminator
        if (*outlength == 23 && output[22] == 0 && isdigit(output[0])) {
            char keyhash[36];
            MD5Digest(output, 22, keyhash);

            auto it = std::lower_bound(publicKeyhashes.begin(), publicKeyhashes.end(), keyhash, [](const char* a, const char* b) {
                return strcmp(a, b) < 0;
            });
            // If the keyhash was found in the list, do not use it
            // Extra strcmp is needed because lower_bound is used instead of binary_search
            if (it != publicKeyhashes.end() && strcmp(*it, keyhash) == 0) {
                useMachineGuid = true;
            }
        }
    }

    if (useMachineGuid) {
        // Use windows installation ID as a base for key generation
        char guid[40];
        DWORD guid_size = sizeof(guid);
        if (GetMachineGUID((unsigned char*)guid, &guid_size)) {
            // The hash of the installation id is converted into a string containing a 22 digit decimal number
            
            // Hash the installation ID, use the 96 bits of the result as 3 32 bit integers
            sodium_init();
            uint32_t temp[3];
            crypto_generichash((unsigned char*)temp, sizeof(temp), (unsigned char*)guid, guid_size, 0, 0);

            // convert the three integers into three 10 character decimal numbers
            char tempstr[3][12];
            for (int i = 0; i < 3; i++) sprintf(tempstr[i], "%010u", temp[i]);

            // Concatenate the three numbers, drop the first digits because its less random.
            // The output buffer in the caller is always 40 bytes.
            snprintf(output, 23, "%s%s%s\n", tempstr[0] + 3, tempstr[1] + 2, tempstr[2] + 3);
            *outlength = strlen(output) + 1; // outlength always includes the zero terminator
        }
        debuglog("new key: %s\n", output);
    }
    return ok;
}

void generic_hook_init()
{
    addPlayerInput_addr = (uintptr_t)hook_function(addPlayerInput_addr, 8, method_to_voidptr(&Game::addPlayerInput_hook));
    getStringFromRegistry_addr = (uintptr_t)hook_function(getStringFromRegistry_addr, 5, getStringFromRegistry_hook);
}
