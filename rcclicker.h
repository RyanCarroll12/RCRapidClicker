#pragma once

#include "resource.h"
#include "rcinputs.h"
#include "rcconfig.h"

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