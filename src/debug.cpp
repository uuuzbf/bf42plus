#include "pch.h"

char* getTimestamp()
{
    static char buffer[128];
    SYSTEMTIME now;
    GetLocalTime(&now);
    snprintf(buffer, 128, "[%02d:%02d:%02d.%03d] (%05d)", now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, GetCurrentThreadId());
    return buffer;
}

char* getFileTimestamp()
{
    static char buffer[128];
    SYSTEMTIME now;
    GetLocalTime(&now);
    snprintf(buffer, 128, "%04d-%02d-%02d_%02d-%02d-%02d-%03d", now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond, now.wMilliseconds);
    return buffer;
}

void __stdcall function_tracer_fastcall(const char* name, uintptr_t* args) {
    static bfs::string* g_console_run_result = (bfs::string*)0x009A9428;
    if (strcmp(name, "?S1S2S3S4:GameConsole__run") == 0 && g_console_run_result->length != 0) {
        debuglog("run result: %s\n", g_console_run_result->c_str());
    }
    char* now = getTimestamp();
    if (*name == '?') {
        // format string: starts with ? ends with :
        // arg format is <type><argid>
        // type can be iuXpcTONS*f see switch below
        // argid is 1..6 or C for ecx(this) D for edx
        // argid==0 is the caller address!
        uintptr_t* p = args + 2;
        const char* nname = strchr(++name, ':');
        debuglog("%s [trace] %p -> %s (", now, (void*)args[2], nname != 0 ? (nname + 1) : name);
        char c;
        char fmt[16] = "%d, ";
        uintptr_t fmtmask = ~1UL;
        int extra = 0;
        void* tmp;
        while ((c = (*(name++))) != ':') {
            int k = 0;
            switch (c) {
            case '&': fmtmask = ~(fmtmask << ((*(name++) - '0') * 8)); continue;
            case '0': k = 0; break; // this is the caller address!
            case '1': k = 1; break;
            case '2': k = 2; break;
            case '3': k = 3; break;
            case '4': k = 4; break;
            case '5': k = 5; break;
            case '6': k = 6; break;
            case 'C': k = -2; break;
            case 'D': k = -1; break;
            case 'i': strcpy(fmt, "%i, "); continue;
            case 'u': strcpy(fmt, "%u, "); continue;
            case 'X': strcpy(fmt, "%08X, "); continue;
            case 'p': strcpy(fmt, "%p, "); continue;
            case 'c': strcpy(fmt, "%c, "); continue;
            case 'T': extra = 1; continue; // template id
            case 'O': extra = 2; continue; // object pointer
            case 'N': extra = 3; continue; // object network id
            case 'S': extra = 4; continue; // std_string*
            case '*': extra = 5; continue; // dereference pointer into p
            case 'f': extra = 6; continue; // float
            case 's': extra = 7; continue; // char* string
            default:
                debuglog("format error '%c'\n", c);
                break;
            }
            switch (extra) {
            case 0: debuglog(fmt, p[k] & fmtmask); break;
            case 1: debuglog("T%d/%s, ", p[k], "_"/*getTemplateNameByID(p[k])*/); break;
                //case 2: debuglog("O-%p/%s, ", (void*)p[k], (tmp = queryInterface((void*)p[k], 0xC378 /* IID_IObject */)) != 0 ? getObjectTemplateName(tmp) : "?"); break;
            case 2: {
                IObject* o = (IObject*)p[k];
                auto networkable = o->getNetworkable();
                if (networkable) {
                    debuglog("O-%p/%sG%dN%u, ", o, o->getTemplate()->getName().c_str(), o->getID(), networkable->getID());
                }
                else {
                    debuglog("O-%p/%sG%d, ", o, o->getTemplate()->getName().c_str(), o->getID());
                }
                break;
            }
            case 3: {
                //void* obj = getObjectByNetworkID((uint16_t)p[k]);
                //if (obj != 0 && queryInterface(obj, 0xC378 /* IID_IObject */) != 0) {
                //    debuglog("N%i/%s, ", p[k], getObjectTemplateName(obj));
                //}
                /*else*/ debuglog("%i/?, ", p[k]);
                break;
            }
            case 4: debuglog("\"%s\", ", ((bfs::string*)p[k])->c_str()); break;
            case 5: // continue parsing a pointer at k
                p = (uintptr_t*)args[k + 2];
                debuglog("args[%d]=%p -> ", k, p);
                break;
            case 6: debuglog("%f, ", ((float*)p)[k]); break; // float
            case 7: debuglog("\"%s\", ", (char*)p[k]); break;
            }
            extra = 0;
            fmtmask = ~1UL;
        }
        debuglog(")\n");
    }
    else if (*name == '%') { // no default logging
        name++;
    }
    else {
        debuglog("%s [trace] %p -> %s (ecx=%08X, edx=%08X, %08X, %08X, %08X, %08X)\n", now, (void*)args[2], name, args[0], args[1], args[3], args[4], args[5], args[6]);
    }
}

//#ifdef _DEBUG
static FILE* debuglogHandle = 0;
static bool debuglogOpenFailed = false;

static bool maybeOpenDebuglog()
{
    if (debuglogOpenFailed) return false;
    CreateDirectory(L"logs", 0);
    DeleteFile(L"logs/bf42plus_debug.3.log");
    MoveFile(L"logs/bf42plus_debug.2.log", L"logs/bf42plus_debug.3.log");
    MoveFile(L"logs/bf42plus_debug.1.log", L"logs/bf42plus_debug.2.log");
    MoveFile(L"logs/bf42plus_debug.log", L"logs/bf42plus_debug.1.log");
    debuglogHandle = fopen("logs/bf42plus_debug.log", "w");
    if (!debuglogHandle) {
        debuglogOpenFailed = true;
        return false;
    }
    return true;
}

void debuglog(const char* fmt, ...)
{
    if (!debuglogHandle && !maybeOpenDebuglog()) {
        return;
    }
    va_list va;
    va_start(va, fmt);
    vfprintf(debuglogHandle, fmt, va);
    va_end(va);
    fflush(debuglogHandle);
}

void debuglogt(const char* fmt, ...)
{
    if (!debuglogHandle && !maybeOpenDebuglog()) {
        return;
    }
    SYSTEMTIME now;
    GetLocalTime(&now);
    fprintf(debuglogHandle, "[%02d:%02d:%02d.%03d] (%05d) ", now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, GetCurrentThreadId());
    va_list va;
    va_start(va, fmt);
    vfprintf(debuglogHandle, fmt, va);
    va_end(va);
    fflush(debuglogHandle);
}

bool isDebuglogOpen()
{
    return debuglogHandle != 0;
}

void closeAndRenameDebuglog(const char* timestamp, const char* suffix)
{
    if (!isDebuglogOpen()) return;

    fclose(debuglogHandle);
    debuglogHandle = 0;

    char newname[MAX_PATH];
    snprintf(newname, MAX_PATH, "logs/crash/%s_%s.log", timestamp, suffix);

    CreateDirectory(L"logs/crash", 0);

    MoveFileA("logs/bf42plus_debug.log", newname);
}

//#endif
