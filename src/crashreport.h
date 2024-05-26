#pragma once

LONG __stdcall unhandledExceptionFilter(LPEXCEPTION_POINTERS x);

/// <summary>
/// Called when the game's log handler is called with an ERROR message
/// </summary>
void handleFatalError();

/// <summary>
/// Initializes the crash handler and the hang watchdog
/// </summary>
/// <param name="module"></param>
void initCrashReporter(HMODULE module);

void deleteOldCrashDumps(int dumpsToKeep);
