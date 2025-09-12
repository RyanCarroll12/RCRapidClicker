#pragma once

#include "rcclicker.h"
#include "rcinputs.h"

#include <windows.h>
#include <stdio.h>
#include <strsafe.h>


#define CONFIG_TITLE "RC's Rapid Clicker, " __DATE__

// control id's
#define ID_OK                      IDOK     // 1
#define ID_CANCEL                  IDCANCEL // 2
#define ID_DEFAULTS                3

#define ID_INPUT_EVENT             220
#define ID_INPUT_FREQUENCY         230

#define ID_SHORTCUT_CLICKER        400
#define ID_SHORTCUT_SAVEPOSITION   410

// control types
#define ITEM_CHECKBOX (1<<0)
#define ITEM_NUMBER   (1<<1)
#define ITEM_COMBOBOX (1<<2)
#define ITEM_FOLDER   (1<<3)
#define ITEM_BUTTON   (1<<4)
#define ITEM_HOTKEY   (1<<5)

// win32 control styles
#define CONTROL_BUTTON    0x0080
#define CONTROL_EDIT      0x0081
#define CONTROL_STATIC    0x0082
#define CONTROL_LISTBOX   0x0083
#define CONTROL_SCROLLBAR 0x0084
#define CONTROL_COMBOBOX  0x0085

#define PADDING 4             // padding for dialog and group boxes
#define BUTTON_WIDTH 50       // normal button width
#define BUTTON_SMALL_WIDTH 14 // small button width
#define ITEM_HEIGHT 14        // control item height

#define COMBOBOX_TEXT_LENGTH 72
#define DIALOG_X_SIZE PADDING + ((BUTTON_WIDTH + PADDING) * 3) + PADDING
#define DIALOG_Y_SIZE PADDING + (ITEM_HEIGHT * 7) + PADDING + PADDING

// Hot key getters
#define HOT_KEY(Key, Mod) ((Key) | ((Mod) << 8))
#define HOT_KEY_GET_KEY(KeyMod) ((KeyMod) & 0xff)
#define HOT_KEY_GET_MOD(KeyMod) (((KeyMod) >> 8) & 0xff)


typedef struct
{
    // output
    CHAR OutputFolder[MAX_PATH];
    BOOL OpenFolder;
    BOOL FragmentedOutput;
    BOOL EnableLimitLength;
    BOOL EnableLimitSize;
    DWORD LimitLength;
    DWORD LimitSize;
    // input
    DWORD InputEventType;
    DWORD InputTargetFrequency;
    // shortcuts
    DWORD ShortcutClicker;
    DWORD ShortcutSavePosition;
}
Config;

// current control to set shortcut
struct
{
    WNDPROC WindowProc;
    Config* Config;
    int Control;
}
static gConfigShortcut;

typedef struct {
    int Left;
    int Top;
    int Width;
    int Height;
} Config__DialogRect;

typedef struct {
    const char* Text;
    const WORD Id;
    const WORD Item;
    const DWORD Width; // Width of box
} Config__DialogItem;

typedef struct {
    const char* Caption;
    const Config__DialogRect Rect;
    const Config__DialogItem* Items;
} Config__DialogGroup;

typedef struct {
    const char* Title;
    const char* Font;
    const WORD FontSize;
    const Config__DialogGroup* Groups;
} Config__DialogLayout;


BOOL DisableHotKeys(HWND Window);
BOOL EnableHotKeys(HWND Window);

void Config_Defaults(Config* C);
static void Config__FormatKey(DWORD KeyMod, WCHAR* Text, size_t TextBufferSize);
static void Config__SetDialogValues(HWND Window, Config* C);
static LRESULT CALLBACK Config__DialogProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam);
static void* Config__Align(BYTE* Data, SIZE_T Size);
static BYTE* Config__DoDialogItem(BYTE* Data, LPCSTR Text, WORD Id, WORD Control, DWORD Style, int x, int y, int w, int h);
static void Config__DoDialogLayout(const Config__DialogLayout* Layout, BYTE* Data, SIZE_T DataSize);
BOOL Config_ShowDialog(Config* C);

