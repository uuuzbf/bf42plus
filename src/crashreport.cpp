#include "pch.h"

#include <DbgHelp.h>



static HMODULE thisModule = 0;
static DWORD mainThreadID = 0;

typedef BOOL __stdcall MiniDumpWriteDump_t(HANDLE, DWORD, HANDLE, DWORD, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

static MiniDumpWriteDump_t* pMiniDumpWriteDump = 0;

static void writeCoredump(const char* timestamp, EXCEPTION_POINTERS* x, DWORD threadid) {
    if (pMiniDumpWriteDump) {
        char dumpfile[MAX_PATH];
        CreateDirectory(L"logs", 0);
        CreateDirectory(L"logs/crash", 0);
        snprintf(dumpfile, MAX_PATH, "logs/crash/%s.dmp", timestamp);

        debuglog("writing %s... ", dumpfile);
        HANDLE file = CreateFileA(dumpfile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
        if (file != 0) {
            MINIDUMP_EXCEPTION_INFORMATION excinfo_s;
            MINIDUMP_EXCEPTION_INFORMATION* excinfo = 0;
            if (x) {
                excinfo_s.ThreadId = threadid;
                excinfo_s.ExceptionPointers = x;
                excinfo_s.ClientPointers = FALSE;
                excinfo = &excinfo_s;
            }
            DWORD type;
            if (g_settings.crashCreateFullDump) { // create full dump
                type = MiniDumpWithFullMemoryInfo | MiniDumpWithFullMemory;
            }
            else { // minidump
                type = MiniDumpNormal | MiniDumpFilterMemory | MiniDumpWithDataSegs | MiniDumpFilterModulePaths | MiniDumpWithFullMemoryInfo;
            }

            pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, type, excinfo, 0, 0);
            CloseHandle(file);
            debuglog("done\n");
        }
        else debuglog("failed to open - %x\n", GetLastError());
    }
}

static void logStack(uintptr_t start)
{
    // print stack contents
    uint32_t* stack = (uint32_t*)(start & ~3); // make sure pointer to stack is 4-byte aligned
    for (int i = 0; i < 1024; i += 16) {
        debuglog("%p:  %08X %08X %08X %08X   ", stack, stack[0], stack[1], stack[2], stack[3]);
        char* cptr = (char*)stack;
        for (int j = 0; j < 16; j++) {
            char c = cptr[j];
            debuglog("%c", c < 32 || c > 126 ? '.' : c);
        }
        debuglog("\n");
        stack += 4;
    }

    // print numbers on stack that look like pointers to executable memory
    void** stackp = (void**)(start & ~3);
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((void*)start, &mbi, sizeof(mbi)) != 0) {
        int n = ((uintptr_t)mbi.BaseAddress + mbi.RegionSize - start) / 4;
        int cnt = 0;
        for (int i = 0; i < n; i++) {
            //if (VirtualQuery(*stackp, &mbi, sizeof(mbi)) != 0 && mbi.AllocationProtect & 0xF0) { // 0x10 0x20 0x40 0x80 are constants with execute
            // use a hardcoded range for BF1942 executable memory
            if ((uintptr_t)*stackp > 0x00401000 && (uintptr_t)*stackp < 0x008C3000) {
                debuglog("%p: %p\n", stackp, *stackp);
                if (++cnt > 30) break;
            }
            stackp++;
        }
    }
}

static void logContextRecord(CONTEXT* ctx)
{
    // print address of our module
    debuglog("module address = %p version = %ls\n", thisModule, get_build_version());
    // print register contents
    debuglog("eax:%08X ebx:%08X ecx:%08X edx:%08X edi:%08X esi:%08X ebp:%08X eip:%08X esp:%08X\n",
        ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx, ctx->Edi, ctx->Esi, ctx->Ebp, ctx->Eip, ctx->Esp);

    logStack(ctx->Esp);

    // print readable memory regions pointed to by registers
    uint32_t** ptr = (uint32_t**)&ctx->Edi;
    static const char* regNames[] = { "edi", "esi", "ebx", "edx", "ecx", "eax" };
    for (int i = 0; i <= 6; i++) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((void*)ptr[i], &mbi, sizeof(mbi)) != 0 && mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS) {
            debuglog("%s: %p -> type %X protect %X - %08X %08X %08X %08X\n", regNames[i], ptr[i], mbi.Type, mbi.Protect, ptr[i][0], ptr[i][1], ptr[i][2], ptr[i][3]);
        }
    }
}

static void logThreadContext(DWORD threadID)
{
    // THREAD_QUERY_INFORMATION  is only needed on XP
    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, false, threadID);
    if (thread)
    {
        if (SuspendThread(thread) != (DWORD)-1) {
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL; // get register values for eax, ecx, esp, eip etc..
            if (GetThreadContext(thread, &ctx)) {
                logContextRecord(&ctx);
            }
            else debuglog("logThreadContext: %s failed %X", "context", GetLastError());

            ResumeThread(thread);
        }
        else debuglog("logThreadContext: %s failed %X", "suspend", GetLastError());
        CloseHandle(thread);
    }
    else debuglog("logThreadContext: %s %X", "open", GetLastError());
}

LONG __stdcall unhandledExceptionFilter(LPEXCEPTION_POINTERS x) {
    DWORD threadID = GetCurrentThreadId();
    EXCEPTION_RECORD* xr = x->ExceptionRecord;

    if (isDebuglogOpen()) {
        debuglog("\n");
        debuglogt("exception %08X at %p", xr->ExceptionCode, xr->ExceptionAddress);
        if (xr->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
            debuglog(xr->ExceptionInformation[0] == 0 ? " reading %p\n" : " writing %p\n", (void*)xr->ExceptionInformation[1]);
        }
        else
            debuglog("\n");

        logContextRecord(x->ContextRecord);
    }


    const char* now = getFileTimestamp();

    writeCoredump(now, x, threadID);
    closeAndRenameDebuglog(now, "crash");

    return EXCEPTION_EXECUTE_HANDLER;
}

static DWORD __stdcall watchdogThread(void*)
{


    // wait until global Setup* is initialized
    while (!*(void**)0x00971EAC) Sleep(1000);

    int ticksUntilKill = 0;
    do {
        // TODO: detect hangs automatically, this is a bit more complicated
        // - detect hangs in main menu before loading a game
        // - detect hangs when GameServer is used (host or singleplayer)
        // - detect hangs when GameClient is used (multiplayer client)
        // -  GameClient hangs the main loop while on loading screen!

        Sleep(1000);
        if ((GetAsyncKeyState(VK_ESCAPE) >> 15)) ticksUntilKill++;
        else ticksUntilKill = 0;

        // is player holding Escape key to force an exit?
        if (ticksUntilKill == 10) {
            debuglog("\n\n%s force kill key triggered, forcing exit..\n\n", getTimestamp());

            const char* now = getFileTimestamp();

            if (isDebuglogOpen()) {
                // Save main thread registers and stack into log
                logThreadContext(mainThreadID);

                closeAndRenameDebuglog(now, "forceexit");
            }

            writeCoredump(now, 0, mainThreadID);

            //ExitProcess(48); // ExitProcess calls global destructors and may cause crash
            TerminateProcess(GetCurrentProcess(), 48);
        }

    } while (!(*(bool**)0x00971EAC)[0x14d]); // while (!setup->got_quit_message)
    return 0;
}

void handleFatalError()
{
    const char* now = getFileTimestamp();
    writeCoredump(now, 0, GetCurrentThreadId());

    if (isDebuglogOpen()) {

        closeAndRenameDebuglog(now, "error");
    }

    //ExitProcess(48); // ExitProcess calls global destructors and may cause crash
    TerminateProcess(GetCurrentProcess(), 49);
}

void initCrashReporter(HMODULE module)
{
    thisModule = module;
    mainThreadID = GetCurrentThreadId();
    SetUnhandledExceptionFilter(unhandledExceptionFilter);

    CreateThread(0, 0, watchdogThread, 0, 0, 0);

    HMODULE dbghelp = LoadLibraryA("dbghelp.dll");
    if (dbghelp == 0) {
        debuglog("failed to load dbghelp.dll\n");
        return;
    }
    pMiniDumpWriteDump = (MiniDumpWriteDump_t*)GetProcAddress(dbghelp, "MiniDumpWriteDump");
}

void deleteOldCrashDumps(int dumpsToKeep)
{
    // Create a struct and a list to store dump file paths in, sorted by modification time, ascending.
    struct DumpFile {
        wchar_t filename[MAX_PATH];
        FILETIME mtime;
        DumpFile(WIN32_FIND_DATA& f) : mtime(f.ftLastWriteTime) { wcscpy(filename, f.cFileName); };
    };
    std::list<DumpFile> dumpFiles;

    // Find all *.dmp files and add them to the list so that it will be sorted after insertion.
    WIN32_FIND_DATA find;
    HANDLE search;
    if ((search = FindFirstFile(L"logs/crash/*.dmp", &find)) != INVALID_HANDLE_VALUE) {
        do {
            auto it = dumpFiles.begin();
            for(; it != dumpFiles.end(); ++it){
                if (CompareFileTime(&it->mtime, &find.ftLastWriteTime) == 1) break; // item > new
            }
            dumpFiles.insert(it, DumpFile(find));
        } while (FindNextFile(search, &find));
        FindClose(search);
    }
    else return; // no files found

    // Delete old dump files until there aren't too many
    while (dumpFiles.size() > dumpsToKeep) {
        wchar_t path[MAX_PATH] = L"logs/crash/";
        wcsncat(path, dumpFiles.front().filename, MAX_PATH - wcslen(path));
        debuglog("deleting %ls\n", path);
        DeleteFile(path);
        dumpFiles.pop_front();
    }
}
