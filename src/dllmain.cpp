// this file is responsible for the initial hooking
// DllMain is called first, that hooks the game's main() function
// the hooked main function initializes all the other hooks, then calls the original main function

#include "pch.h"

HMODULE g_this_module;
typedef int __stdcall WinMain_t(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
WinMain_t* WinMain_orig = 0;
int __stdcall WinMain_hook(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    bfhook_init();

    updater_client_startup();

    return WinMain_orig(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}

extern "C" __declspec(dllexport)
void* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    char path[MAX_PATH] = ".\\d3d8_next.dll";
    HMODULE d3d8dll = LoadLibraryA(path);
    if (d3d8dll == 0) {
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, sizeof(path), "\\d3d8.dll");
        d3d8dll = LoadLibraryA(path);
    }

    if (d3d8dll != 0) {
    }
    else {
        MessageBoxA(0, path, "failed to load d3d8.dll", 0);
        return 0;
    }

    typedef void* WINAPI Direct3DCreate8_t(UINT SDKVersion);
    Direct3DCreate8_t* Direct3DCreate8_orig = (Direct3DCreate8_t*)(void*)GetProcAddress(d3d8dll, "Direct3DCreate8");

    return Direct3DCreate8_orig(SDKVersion);
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

