// this file is responsible for the initial hooking
// DllMain is called first, that hooks the game's main() function
// the hooked main function initializes all the other hooks, then calls the original main function

#include "pch.h"

HMODULE g_this_module;
typedef int __stdcall WinMain_t(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
WinMain_t* WinMain_orig = 0;
int __stdcall WinMain_hook(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    g_settings.load();
    bfhook_init();

#ifndef _DEBUG
    updater_client_startup();
#endif

    register_custom_console_commands();

    if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
        if (MessageBoxA(0, "Left Shift pressed\nStart in window mode?", "BF42Plus", MB_YESNO) == IDYES) {
            nop_bytes(0x00442686, 2); // force 0 in Setup__setFullScreen
        }
    }

    return WinMain_orig(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

extern "C" __declspec(dllexport)
HRESULT WINAPI DirectSoundCreate8(LPCGUID lpcGuidDevice, void** ppDS8, void* pUnkOuter)
{
    char path[MAX_PATH] = ".\\dsound_next.dll";
    HMODULE dsounddll = LoadLibraryA(path);
    if (dsounddll == 0) {
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, sizeof(path), "\\dsound.dll");
        dsounddll = LoadLibraryA(path);
    }

    if (dsounddll != 0) {
    }
    else {
        MessageBoxA(0, path, "failed to load dsound.dll", 0);
        return 0;
    }

    typedef HRESULT WINAPI Direct3DCreate8_t(LPCGUID lpcGuidDevice, void** ppDS8, void* pUnkOuter);
    Direct3DCreate8_t* DirectSoundCreate8_orig = (Direct3DCreate8_t*)(void*)GetProcAddress(dsounddll, "DirectSoundCreate8");

    return DirectSoundCreate8_orig(lpcGuidDevice, ppDS8, pUnkOuter);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    (void)lpReserved;
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            g_this_module = hModule;
            WinMain_orig = (WinMain_t*)modify_call(0x00804DA6, (void*)WinMain_hook);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

