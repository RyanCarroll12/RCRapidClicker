#pragma once

#include "rcclicker.h"
#include "rcconfig.h"

#include <windows.h>


enum InputType {
    INPUT_LEFTCLICK
    , INPUT_INVERTED_LEFTCLICK
    , INPUT_ENTERKEY
    , INPUT_MOVEMOUSE
    , MAX_INPUT
};

struct InputEventInfo
{
    enum InputType type;
    const WCHAR* name;
    const INPUT* inputs;
    int size;
};

extern const struct InputEventInfo InputEvents[];


void set_input_event(DWORD InputEventType);
UINT execute_input_event();