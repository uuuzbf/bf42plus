#pragma once
#include <cstdint>

char* getTimestamp();
char* getFileTimestamp();

void __stdcall function_tracer_fastcall(const char* name, uintptr_t* args);

//#ifdef _DEBUG
void debuglog(const char* fmt, ...);
void debuglogt(const char* fmt, ...);
bool isDebuglogOpen();
void closeAndRenameDebuglog(const char* timestamp, const char* suffix);
//#else
//#define debuglog	(void)
//#endif