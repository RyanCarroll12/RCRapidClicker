#pragma once

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment (lib, "uxtheme.lib")

#include "resource.h"
#include "rcinputs.h"
#include "rcconfig.h"

#define UNICODE

#include <windows.h>
#include <Uxtheme.h>
#include <stdint.h>
#include <strsafe.h>

#define APP_NAME L"RC's Rapid Clicker"

#ifdef _DEBUG
// Enclosed in a do while loop to ensure line executes as a single statement, otherwise macro if statement could adopt outside else statement
#define Assert(Cond) do { if (!(Cond)) __debugbreak(); } while (0)
#else
#define Assert(Cond) (void)(Cond)
#endif