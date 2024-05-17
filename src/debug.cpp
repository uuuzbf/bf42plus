#include "pch.h"

static char* get_timestamp()
{
    static char buffer[128];
    SYSTEMTIME now;
    GetLocalTime(&now);
    snprintf(buffer, 128, "[%02d:%02d:%02d.%03d] (%05d)", now.wHour, now.wMinute, now.wSecond, now.wMilliseconds, GetCurrentThreadId());
    return buffer;
}

void __stdcall function_tracer_fastcall(const char* name, uintptr_t* args) {
    static bfs::string* g_console_run_result = (bfs::string*)0x009A9428;
    if (strcmp(name, "?S1S2S3S4:GameConsole__run") == 0 && g_console_run_result->length != 0) {
        debuglog("run result: %s\n", g_console_run_result->c_str());
    }
    char* now = get_timestamp();
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
