// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

#define WINVER 0x0502
#define _WIN32_WINNT 0x0502

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS
// Windows Header Files
#include <windows.h>
#include <CommCtrl.h>
#include <commdlg.h>
#include <float.h>
#include <initializer_list>
#include <string>
#include <vector>
#include <list>
#include <array>
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <optional>
#include <cstdio>
#include <cstdint>
#include <ctime>
#include <cassert>

#include <intrin.h>

#define SODIUM_STATIC
#include <sodium.h>

#include "../res/resource.h"

#include "util.h"
#include "updater.h"
#include "hooks.h"
#include "bfhook.h"
#include "debug.h"
#include "crashreport.h"

#include "bf/stl.h"
#include "bf/generic.h"
#include "bf/object.h"
#include "bf/gameevent.h"
#include "bf/ui.h"
#include "bf/console.h"
#include "bf/renderer.h"

#include "settings.h"

extern HMODULE g_this_module;

#endif //PCH_H
