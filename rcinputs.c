#include "rcinputs.h"


INPUT LeftClick[] =
{
    {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_LEFTDOWN}
    }
    , {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_LEFTUP}
    }
};

INPUT InvLeftClick[] =
{
    {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_LEFTUP}
    }
    , {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_LEFTDOWN}
    }
};

INPUT RightClick[] =
{
    {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_RIGHTDOWN}
    }
    , {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_RIGHTUP}
    }
};

INPUT PointLeftClick[] =
{
    {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE}
    }
    , {
        .type = INPUT_MOUSE
        , .mi = {.dwFlags = MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE}
    }
};

INPUT EnterKey[] =
{
    {
        .type = INPUT_KEYBOARD
        , .ki = {.wVk = VK_RETURN}
    }
    , {
        .type = INPUT_KEYBOARD
        , .ki = {.wVk = VK_RETURN, .dwFlags = KEYEVENTF_KEYUP}
    }
};

INPUT MoveMouse[] =
{
    {
        .type = INPUT_MOUSE
        , .mi = {.dx = 1, .dy = 1, .dwFlags = MOUSEEVENTF_MOVE}
    }
};


const struct InputEventInfo InputEvents[] =
{
    {
          .type = INPUT_LEFTCLICK
        , .name = L"Left Click"
        , .inputs = LeftClick
        , .size = ARRAYSIZE(LeftClick)
    }
    , {
          .type = INPUT_INVERTED_LEFTCLICK
        , .name = L"Inverted Left Click"
        , .inputs = InvLeftClick
        , .size = ARRAYSIZE(InvLeftClick)
    }
    , {
          .type = INPUT_RIGHTCLICK
        , .name = L"Right Click"
        , .inputs = RightClick
        , .size = ARRAYSIZE(RightClick)
    }
    , {
          .type = INPUT_ENTERKEY
        , .name = L"Enter Key"
        , .inputs = EnterKey
        , .size = ARRAYSIZE(EnterKey)
    }
    , {
          .type = INPUT_MOVEMOUSE
        , .name = L"Move Mouse"
        , .inputs = MoveMouse
        , .size = ARRAYSIZE(MoveMouse)
    }
};

struct InputEventInfo InputEvent;

void set_input_event(DWORD InputEventType)
{
    for (int i = 0; i < ARRAYSIZE(InputEvents); i++)
    {
        if (InputEvents[i].type == InputEventType)
        {
            InputEvent = InputEvents[i];
            return;
        }
    }

    //throw error
    MessageBoxW(NULL,
        L"Invalid Input Event - RC",
        APP_NAME,
        0);
}

UINT execute_input_event()
{
    // Casting here just to remove const warning
    return SendInput(InputEvent.size, (LPINPUT) InputEvent.inputs, sizeof(INPUT));
}