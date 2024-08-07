// this file is responsible for the initial hooking
// DllMain is called first, that hooks the game's main() function
// the hooked main function initializes all the other hooks, then calls the original main function

#include "pch.h"
#include <tlhelp32.h>

HMODULE g_this_module;
typedef int __stdcall WinMain_t(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
WinMain_t* WinMain_orig = 0;
int __stdcall WinMain_hook(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{

    debuglogt("BF1942 started, plus version: %ls parameters: %s\n", get_build_version(), lpCmdLine);

    initCrashReporter(g_this_module);

    g_settings.load();

    bfhook_init();

    if (g_settings.crashDumpsToKeep > 0) deleteOldCrashDumps(g_settings.crashDumpsToKeep);

    bool forceWindowMode = false;

    if (strstr(lpCmdLine, " +forceWindow") != 0) {
        forceWindowMode = true;
    }

#ifndef _DEBUG
    // Do not check for updates on map restarts
    if(strstr(lpCmdLine, " +reconnectPassword ") == 0) {
        bool blackscreenFound = false;
#if 0
        HWND blackscreenWnd = FindWindow(L"black", L"");
        if (blackscreenWnd) {
            DWORD pid;
            if (GetWindowThreadProcessId(blackscreenWnd, &pid)) {
                blackscreenFound = true;
            }
        }
#endif
        // Before running the updater, check if BlackScreen.exe is running (BF1942 is being restarted)
        // Blocking the startup process while blackscreen is running can cause problems, because it hides
        // windows and it 
        HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (h != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 pe = {};
            pe.dwSize = sizeof(PROCESSENTRY32);

            // Find PID of parent process
            DWORD myPID = GetCurrentProcessId();
            DWORD parentPID = 0;
            if (Process32First(h, &pe)) {
                do {
                    if (pe.th32ProcessID == myPID) {
                        parentPID = pe.th32ParentProcessID;
                        break;
                    }
                } while (Process32Next(h, &pe));
            }
            // Find name of parent process ID
            if (parentPID && Process32First(h, &pe)) {
                do {
                    if (pe.th32ProcessID == parentPID) {
                        // Parent is blackscreen.exe?
                        if (_wcsicmp(pe.szExeFile, L"BlackScreen.exe") == 0) {
                            blackscreenFound = true;
                        }
                        break;
                    }
                } while (Process32Next(h, &pe));
            }

            CloseHandle(h);
        }

        if (!blackscreenFound) {
            updater_client_startup();
        }
    }
#endif
    if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
        if (MessageBoxA(0, "Left Shift pressed\nStart in window mode?", "BF42Plus", MB_YESNO) == IDYES) {
            forceWindowMode = true;
        }
    }

    if (forceWindowMode) {
        nop_bytes(0x00442686, 2); // force 0 in Setup__setFullScreen
    }

    register_custom_console_commands();

#if 0
    // For debugging the unhandled exception filter
    __try {
        return WinMain_orig(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
    }
    __except (unhandledExceptionFilter(GetExceptionInformation())) {
        OutputDebugStringA("exception filter executed");
    }
#else
    return WinMain_orig(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
#endif
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
            // Check if we are running in the correct executable. This address has the call to main()
            if (memcmp((void*)0x00804DA6, "\xE8\x95\x19\xC0\xFF", 5) != 0) break;

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

