#pragma once

#include <windows.h>
#include "inputs.h"

#define CONFIG_INPUT_LEFTCLICK 0
#define CONFIG_INPUT_ENTERKEY 1
#define CONFIG_INPUT_MOVEMOUSE 2

#define CONFIG_TITLE "RC's Rapid Clicker, " __DATE__

// control id's
#define ID_OK                      IDOK     // 1
#define ID_CANCEL                  IDCANCEL // 2
#define ID_DEFAULTS                3

#define ID_OUTPUT_FOLDER           100
#define ID_OPEN_FOLDER             110
#define ID_FRAGMENTED_MP4          120
#define ID_LIMIT_LENGTH            130
#define ID_LIMIT_SIZE              140

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

// group box width/height
#define COL00W 120
#define COL01W 154
#define COL10W 144
#define COL11W 130
#define ROW0H 84
#define ROW1H 124
#define ROW2H 56

#define PADDING 4             // padding for dialog and group boxes
#define BUTTON_WIDTH 50       // normal button width
#define BUTTON_SMALL_WIDTH 14 // small button width
#define ITEM_HEIGHT 14        // control item height

#define DIALOG_X_SIZE PADDING + COL10W + PADDING + COL11W + PADDING
#define DIALOG_Y_SIZE PADDING + ROW0H + PADDING + ROW1H + PADDING + ROW2H + ITEM_HEIGHT
#define COMBOBOX_TEXT_LENGTH 72

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
    DWORD InputEvent;
    DWORD InputEventFrequency;
    // shortcuts
    DWORD ShortcutClicker;
    DWORD ShortcutSavePosition;
}
Config;

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

// currently open dialog window
static HWND gDialogWindow;

// current control to set shortcut
struct
{
    WNDPROC WindowProc;
    Config* Config;
    int Control;
}
static gConfigShortcut;


void Config_Defaults(Config* C)
{
    *C = (Config)
    {
        // output
          .OpenFolder = TRUE
        , .FragmentedOutput = FALSE
        , .EnableLimitLength = FALSE
        , .EnableLimitSize = FALSE
        , .LimitLength = 60
        , .LimitSize = 25
        // input
        , .InputEvent = CONFIG_INPUT_LEFTCLICK
        , .InputEventFrequency = 30
        // shortcuts
        , .ShortcutClicker = HOT_KEY(VK_TAB, MOD_SHIFT)
        , .ShortcutSavePosition = 0//HOT_KEY(VK_DECIMAL, MOD_SHIFT)
    };
}

static void Config__FormatKey(DWORD KeyMod, CHAR* Text, size_t TextBufferSize)
{
    if (KeyMod == 0)
    {
        strcpy_s(Text, TextBufferSize, "[none]");
        return;
    }

    DWORD Mod = HOT_KEY_GET_MOD(KeyMod);

    Text[0] = 0;
    if (Mod & MOD_CONTROL) strcat_s(Text, TextBufferSize, "Ctrl + ");
    if (Mod & MOD_WIN)     strcat_s(Text, TextBufferSize, "Win + ");
    if (Mod & MOD_ALT)     strcat_s(Text, TextBufferSize, "Alt + ");
    if (Mod & MOD_SHIFT)   strcat_s(Text, TextBufferSize, "Shift + ");

    struct
    {
        DWORD Key;
        CHAR* Text;
    }
    static const Overrides[] =
    {
          { VK_PAUSE,    "Pause"    }
        , { VK_SNAPSHOT, "PrtScr"   }
        , { VK_PRIOR,    "PageUp"   }
        , { VK_NEXT,     "PageDown" }
        , { VK_END,      "End"      }
        , { VK_HOME,     "Home"     }
        , { VK_LEFT,     "Left"     }
        , { VK_UP,       "Up"       }
        , { VK_RIGHT,    "Right"    }
        , { VK_DOWN,     "Down"     }
        , { VK_INSERT,   "Insert"   }
        , { VK_DELETE,   "Delete"   }
    };

    for (size_t i = 0; i < ARRAYSIZE(Overrides); i++)
    {
        if (Overrides[i].Key == HOT_KEY_GET_KEY(KeyMod))
        {
            strcat_s(Text, TextBufferSize, Overrides[i].Text);
            return;
        }
    }

    CHAR KeyText[32];
    UINT ScanCode = MapVirtualKey(HOT_KEY_GET_KEY(KeyMod), MAPVK_VK_TO_VSC);
    if (GetKeyNameText(ScanCode << 16, KeyText, _countof(KeyText)) == 0)
    {
        snprintf(KeyText,ARRAYSIZE(KeyText), "[0x%02x]", HOT_KEY_GET_KEY(KeyMod));
    }

    strcat_s(Text, TextBufferSize, KeyText);
}

static void Config__SetDialogValues(HWND Window, Config* C)
{
    SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_SETCURSEL, C->InputEvent, 0);
    SetDlgItemInt(Window, ID_INPUT_FREQUENCY, C->InputEventFrequency, FALSE);

    // output
    SetDlgItemText(Window, ID_OUTPUT_FOLDER, C->OutputFolder);
    CheckDlgButton(Window, ID_OPEN_FOLDER, C->OpenFolder);
    CheckDlgButton(Window, ID_FRAGMENTED_MP4, C->FragmentedOutput);
    CheckDlgButton(Window, ID_LIMIT_LENGTH, C->EnableLimitLength);
    CheckDlgButton(Window, ID_LIMIT_SIZE, C->EnableLimitSize);
    SetDlgItemInt(Window, ID_LIMIT_LENGTH + 1, C->LimitLength, FALSE);
    SetDlgItemInt(Window, ID_LIMIT_SIZE + 1, C->LimitSize, FALSE);

    // shortcuts
    CHAR Text[64];
    Config__FormatKey(C->ShortcutClicker, Text, sizeof(Text));
    SetDlgItemText(Window, ID_SHORTCUT_CLICKER, Text);
    SetWindowLongPtr(GetDlgItem(Window, ID_SHORTCUT_CLICKER), GWLP_USERDATA, C->ShortcutClicker);

    Config__FormatKey(C->ShortcutSavePosition, Text, sizeof(Text));
    SetDlgItemText(Window, ID_SHORTCUT_SAVEPOSITION, Text);
    SetWindowLongPtr(GetDlgItem(Window, ID_SHORTCUT_CLICKER), GWLP_USERDATA, C->ShortcutSavePosition);

    EnableWindow(GetDlgItem(Window, ID_LIMIT_LENGTH + 1), C->EnableLimitLength);
    EnableWindow(GetDlgItem(Window, ID_LIMIT_SIZE + 1), C->EnableLimitSize);
}

static LRESULT CALLBACK Config__DialogProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (Message == WM_INITDIALOG)
    {
        Config* C = (Config*)LParam;
        SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR)C);

        SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_ADDSTRING, 0, (LPARAM)"Left Click");
        SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_ADDSTRING, 0, (LPARAM)"Enter Key");
        SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_ADDSTRING, 0, (LPARAM)"Move Mouse");

        Config__SetDialogValues(Window, C);

        SetForegroundWindow(Window);
        gDialogWindow = Window;
        gConfigShortcut.Control = 0;
        return TRUE;
    }
    else if (Message == WM_DESTROY)
    {
        gDialogWindow = NULL;
    }
    else if (Message == WM_COMMAND)
    {
        Config* C = (Config*)GetWindowLongPtr(Window, GWLP_USERDATA);
        int Control = LOWORD(WParam);
        if (Control == ID_OK)
        {
            // Input Event
            C->InputEvent = (DWORD)SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_GETCURSEL, 0, 0);
            C->InputEventFrequency = GetDlgItemInt(Window, ID_INPUT_FREQUENCY, NULL, FALSE);
            // output
            GetDlgItemText(Window, ID_OUTPUT_FOLDER, C->OutputFolder, _countof(C->OutputFolder));
            C->OpenFolder = IsDlgButtonChecked(Window, ID_OPEN_FOLDER);
            C->FragmentedOutput = IsDlgButtonChecked(Window, ID_FRAGMENTED_MP4);
            C->EnableLimitLength = IsDlgButtonChecked(Window, ID_LIMIT_LENGTH);
            C->EnableLimitSize = IsDlgButtonChecked(Window, ID_LIMIT_SIZE);
            C->LimitLength = GetDlgItemInt(Window, ID_LIMIT_LENGTH + 1, NULL, FALSE);
            C->LimitSize = GetDlgItemInt(Window, ID_LIMIT_SIZE + 1, NULL, FALSE);
            // shortcuts
            C->ShortcutClicker = GetWindowLong(GetDlgItem(Window, ID_SHORTCUT_CLICKER), GWLP_USERDATA);
            C->ShortcutSavePosition = GetWindowLong(GetDlgItem(Window, ID_SHORTCUT_SAVEPOSITION), GWLP_USERDATA);

            EndDialog(Window, TRUE);
            return TRUE;
        }
        else if (Control == ID_CANCEL)
        {
            EndDialog(Window, FALSE);
            return TRUE;
        }
        else if (Control == ID_DEFAULTS)
        {
            Config DefaultConfig;
            Config_Defaults(&DefaultConfig);
            Config__SetDialogValues(Window, &DefaultConfig);
            if (gConfigShortcut.Control)
            {
                SetWindowLongPtrW(GetDlgItem(Window, gConfigShortcut.Control), GWLP_WNDPROC, (LONG_PTR)gConfigShortcut.WindowProc);
                gConfigShortcut.Control = 0;
                EnableHotKeys(Window);
            }
            return TRUE;
        }
        else if (Control == ID_LIMIT_LENGTH && HIWORD(WParam) == BN_CLICKED)
        {
            EnableWindow(GetDlgItem(Window, ID_LIMIT_LENGTH + 1), (BOOL)SendDlgItemMessage(Window, ID_LIMIT_LENGTH, BM_GETCHECK, 0, 0));
            return TRUE;
        }
        else if (Control == ID_LIMIT_SIZE && HIWORD(WParam) == BN_CLICKED)
        {
            EnableWindow(GetDlgItem(Window, ID_LIMIT_SIZE + 1), (BOOL)SendDlgItemMessage(Window, ID_LIMIT_SIZE, BM_GETCHECK, 0, 0));
            return TRUE;
        }
        else if (Control == ID_OUTPUT_FOLDER + 1)
        {
            // this expects caller has called CoInitializeEx with single or apartment-threaded model
            /*IFileDialog* Dialog;
            HR(CoCreateInstance(&CLSID_FileOpenDialog, NULL, CLSCTX_INPROC, &IID_IFileDialog, &Dialog));

            WCHAR Text[MAX_PATH];
            GetDlgItemTextW(Window, ID_OUTPUT_FOLDER, Text, _countof(Text));

            IShellItem* Folder;
            if (SUCCEEDED(SHCreateItemFromParsingName(Text, NULL, &IID_IShellItem, &Folder)))
            {
                HR(IFileDialog_SetFolder(Dialog, Folder));
                IShellItem_Release(Folder);
            }

            HR(IFileDialog_SetOptions(Dialog, FOS_NOCHANGEDIR | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST));
            if (SUCCEEDED(IFileDialog_Show(Dialog, Window)) && SUCCEEDED(IFileDialog_GetResult(Dialog, &Folder)))
            {
                LPWSTR Path;
                if (SUCCEEDED(IShellItem_GetDisplayName(Folder, SIGDN_FILESYSPATH, &Path)))
                {
                    SetDlgItemTextW(Window, ID_OUTPUT_FOLDER, Path);
                    CoTaskMemFree(Path);
                }
                IShellItem_Release(Folder);
            }
            IFileDialog_Release(Dialog);*/

            return TRUE;
        }
        else if ((Control == ID_SHORTCUT_CLICKER || Control == ID_SHORTCUT_SAVEPOSITION) && HIWORD(WParam) == BN_CLICKED)
        {
            if (gConfigShortcut.Control == 0)
            {
                SetDlgItemText(Window, Control, "Press new shortcut, [ESC] to cancel, [BACKSPACE] to disable");

                gConfigShortcut.Control = Control;
                gConfigShortcut.Config = C;

                HWND ControlWindow = GetDlgItem(Window, Control);
                gConfigShortcut.WindowProc = (WNDPROC)GetWindowLongPtr(ControlWindow, GWLP_WNDPROC);
                /*SetWindowLongPtrW(ControlWindow, GWLP_WNDPROC, (LONG_PTR)&Config__ShortcutProc);
                DisableHotKeys();*/
            }
        }
    }
    return FALSE;
}

static void* Config__Align(BYTE* Data, SIZE_T Size)
{
    SIZE_T Pointer = (SIZE_T)Data;
    // Moves the data pointer (to a lower bit value) to align with Size provided that Size is a power of 2
    return Data + ((Pointer + Size - 1) & ~(Size - 1)) - Pointer;
}

static BYTE* Config__DoDialogItem(BYTE* Data, LPCSTR Text, WORD Id, WORD Control, DWORD Style, int x, int y, int w, int h)
{
    Data = Config__Align(Data, sizeof(DWORD));

    *(DLGITEMTEMPLATE*)Data = (DLGITEMTEMPLATE)
    {
        .style = Style | WS_CHILD | WS_VISIBLE,
        .x = x,
        .y = y + (Control == CONTROL_STATIC ? 2 : 0),
        .cx = w,
        .cy = h - (Control == CONTROL_EDIT ? 2 : 0) - (Control == CONTROL_STATIC ? 2 : 0),
        .id = Id,
    };
    Data += sizeof(DLGITEMTEMPLATE);

    // window class
    Data = Config__Align(Data, sizeof(WORD));
    *(WORD*)Data = 0xffff;
    Data += sizeof(WORD);
    *(WORD*)Data = Control;
    Data += sizeof(WORD);

    // item text
    Data = Config__Align(Data, sizeof(WCHAR));
    DWORD ItemChars = MultiByteToWideChar(CP_UTF8, 0, Text, -1, (WCHAR*)Data, 128);
    Data += ItemChars * sizeof(WCHAR);

    // create extras
    Data = Config__Align(Data, sizeof(WORD));
    *(WORD*)Data = 0;
    Data += sizeof(WORD);

    return Data;
}

static void Config__DoDialogLayout(const Config__DialogLayout* Layout, BYTE* Data, SIZE_T DataSize)
{
    BYTE* End = Data + DataSize;

    // header
    DLGTEMPLATE* Dialog = (void*)Data;
    Data += sizeof(DLGTEMPLATE);

    // menu
    Data = Config__Align(Data, sizeof(WCHAR));
    *(WCHAR*)Data = 0;
    Data += sizeof(WCHAR);

    // window class
    Data = Config__Align(Data, sizeof(WCHAR));
    *(WCHAR*)Data = 0;
    Data += sizeof(WCHAR);

    // title
    Data = Config__Align(Data, sizeof(WCHAR));
    DWORD TitleChars = MultiByteToWideChar(CP_UTF8, 0, Layout->Title, -1, (WCHAR*)Data, 128);
    Data += TitleChars * sizeof(WCHAR);

    // font size
    Data = Config__Align(Data, sizeof(WORD));
    *(WORD*)Data = Layout->FontSize;
    Data += sizeof(WORD);

    // font name
    Data = Config__Align(Data, sizeof(WCHAR));
    DWORD FontChars = MultiByteToWideChar(CP_UTF8, 0, Layout->Font, -1, (WCHAR*)Data, 128);
    Data += FontChars * sizeof(WCHAR);

    int ItemCount = 3;

    int ButtonX = PADDING + COL10W + COL11W + PADDING - 3 * (PADDING + BUTTON_WIDTH);
    int ButtonY = PADDING + ROW0H + ROW1H + ROW2H + PADDING;

    DLGITEMTEMPLATE* OkData = Config__Align(Data, sizeof(DWORD));
    Data = Config__DoDialogItem(Data, "OK", ID_OK, CONTROL_BUTTON, WS_TABSTOP | BS_DEFPUSHBUTTON, ButtonX, ButtonY, BUTTON_WIDTH, ITEM_HEIGHT);
    ButtonX += BUTTON_WIDTH + PADDING;

    DLGITEMTEMPLATE* CancelData = Config__Align(Data, sizeof(DWORD));
    Data = Config__DoDialogItem(Data, "Cancel", ID_CANCEL, CONTROL_BUTTON, WS_TABSTOP | BS_PUSHBUTTON, ButtonX, ButtonY, BUTTON_WIDTH, ITEM_HEIGHT);
    ButtonX += BUTTON_WIDTH + PADDING;

    DLGITEMTEMPLATE* DefaultsData = Config__Align(Data, sizeof(DWORD));
    Data = Config__DoDialogItem(Data, "Defaults", ID_DEFAULTS, CONTROL_BUTTON, WS_TABSTOP | BS_PUSHBUTTON, ButtonX, ButtonY, BUTTON_WIDTH, ITEM_HEIGHT);
    ButtonX += BUTTON_WIDTH + PADDING;

    for (const Config__DialogGroup* Group = Layout->Groups; Group->Caption; Group++)
    {
        int X = Group->Rect.Left + PADDING;
        int Y = Group->Rect.Top + PADDING;
        int W = Group->Rect.Width;
        int H = Group->Rect.Height;

        Data = Config__DoDialogItem(Data, Group->Caption, -1, CONTROL_BUTTON, BS_GROUPBOX, X, Y, W, H);
        ItemCount++;

        X += PADDING;
        Y += ITEM_HEIGHT - PADDING;
        W -= 2 * PADDING;

        for (const Config__DialogItem* Item = Group->Items; Item->Text; Item++)
        {
            int HasCheckbox = !!(Item->Item & ITEM_CHECKBOX);
            int HasNumber = !!(Item->Item & ITEM_NUMBER);
            int HasCombobox = !!(Item->Item & ITEM_COMBOBOX);
            int OnlyCheckbox = !(Item->Item & ~ITEM_CHECKBOX);
            int HasHotKey = !!(Item->Item & ITEM_HOTKEY);

            int ItemX = X;
            int ItemW = W;
            int ItemId = Item->Id;

            if (HasCheckbox)
            {
                if (!OnlyCheckbox)
                {
                    // reduce width so checbox can fit other control on the right
                    ItemW = Item->Width;
                }
                Data = Config__DoDialogItem(Data, Item->Text, ItemId, CONTROL_BUTTON, WS_TABSTOP | BS_AUTOCHECKBOX, ItemX, Y, ItemW, ITEM_HEIGHT);
                ItemCount++;
                ItemId++;
                if (!OnlyCheckbox)
                {
                    ItemX += Item->Width + PADDING;
                    ItemW = W - (Item->Width + PADDING);
                }
            }

            if ((HasCombobox && !HasCheckbox) || (HasNumber || HasHotKey) && !HasCheckbox)
            {
                // label, only for controls without checkbox, or combobox
                Data = Config__DoDialogItem(Data, Item->Text, -1, CONTROL_STATIC, 0, ItemX, Y, Item->Width, ITEM_HEIGHT);
                ItemCount++;
                ItemX += Item->Width + PADDING;
                ItemW -= Item->Width + PADDING;
            }

            if (HasNumber)
            {
                Data = Config__DoDialogItem(Data, "", ItemId, CONTROL_EDIT, WS_TABSTOP | WS_BORDER | ES_RIGHT | ES_NUMBER, ItemX, Y, ItemW, ITEM_HEIGHT);
                ItemCount++;
            }

            if (HasHotKey)
            {
                Data = Config__DoDialogItem(Data, "", ItemId, CONTROL_BUTTON, WS_TABSTOP, ItemX, Y, ItemW, ITEM_HEIGHT);
                ItemCount++;
            }

            if (HasCombobox)
            {
                Data = Config__DoDialogItem(Data, "", ItemId, CONTROL_COMBOBOX, WS_TABSTOP | CBS_DROPDOWNLIST | CBS_HASSTRINGS, ItemX, Y, ItemW, ITEM_HEIGHT * PADDING);
                ItemCount++;
            }

            if (Item->Item & ITEM_FOLDER)
            {
                Data = Config__DoDialogItem(Data, "", ItemId, CONTROL_EDIT, WS_TABSTOP | WS_BORDER, X, Y, W - BUTTON_SMALL_WIDTH - PADDING + 2, ITEM_HEIGHT);
                ItemCount++;
                ItemId++;

                Data = Config__DoDialogItem(Data, "...", ItemId, CONTROL_BUTTON, WS_TABSTOP | BS_PUSHBUTTON, X + W - BUTTON_SMALL_WIDTH + 2, Y + 1, BUTTON_SMALL_WIDTH - 4, ITEM_HEIGHT - 3);
                ItemCount++;
            }

            Y += ITEM_HEIGHT;
        }
    }

    // Window sizing
    *Dialog = (DLGTEMPLATE)
    {
        .style = DS_SETFONT | DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU,
        .cdit = ItemCount,
        .cx = DIALOG_X_SIZE,
        .cy = DIALOG_Y_SIZE,
    };

    //Assert(Data <= End);
}

BOOL Config_ShowDialog(Config* C)
{
    if (gDialogWindow)
    {
        SetForegroundWindow(gDialogWindow);
        return FALSE;
    }

    Config__DialogItem ClickerOptionsItems[] =
    {
          { "Input Event",                ID_INPUT_EVENT,            ITEM_COMBOBOX, COMBOBOX_TEXT_LENGTH }
        , { "Input Event Frequency",      ID_INPUT_FREQUENCY,    ITEM_NUMBER,   COMBOBOX_TEXT_LENGTH }
        , { NULL }
    };

    Config__DialogRect ClickerOptionsRect[] =
    { 0, 0, COMBOBOX_TEXT_LENGTH * 2, ITEM_HEIGHT * ARRAYSIZE(ClickerOptionsItems) };

    Config__DialogItem OutputItems[] =
    {
          { "",                            ID_OUTPUT_FOLDER,  ITEM_FOLDER                     }
        , { "O&pen When Finished",         ID_OPEN_FOLDER,    ITEM_CHECKBOX                   }
        , { "Fragmented MP&4 (H264 only)", ID_FRAGMENTED_MP4, ITEM_CHECKBOX                   }
        , { "Limit &Length (seconds)",     ID_LIMIT_LENGTH,   ITEM_CHECKBOX | ITEM_NUMBER, 80 }
        , { "Limit &Size (MB)",            ID_LIMIT_SIZE,     ITEM_CHECKBOX | ITEM_NUMBER, 80 }
        , { NULL }
    };

    Config__DialogItem ShortcutItems[] =
    {
          { "Rapid Clicker (INOP)", ID_SHORTCUT_CLICKER,      ITEM_HOTKEY, 64 }
        , { "Set Position",         ID_SHORTCUT_SAVEPOSITION, ITEM_HOTKEY, 64 }
        , { NULL }
    };

    int DialogHeight = 0;

    Config__DialogLayout Dialog = (Config__DialogLayout)
    {
        .Title = CONFIG_TITLE,
        .Font = "Segoe UI",
        .FontSize = 9,
        .Groups = (Config__DialogGroup[])
        {
            {
                .Caption = "&Clicker Options",
                .Rect = { 0, DialogHeight, COMBOBOX_TEXT_LENGTH * 2, ITEM_HEIGHT * ARRAYSIZE(ClickerOptionsItems) },
                .Items = ClickerOptionsItems
            },
            {
                .Caption = "&Output",
                .Rect = { 0, DialogHeight += (ITEM_HEIGHT * ARRAYSIZE(ClickerOptionsItems)), COL01W, ITEM_HEIGHT * ARRAYSIZE(OutputItems) },
                .Items = OutputItems
            },
            {
                .Caption = "Shor&tcuts",
                .Rect = { 0, DialogHeight += (ITEM_HEIGHT * ARRAYSIZE(OutputItems)), COL00W + PADDING + COL01W, ITEM_HEIGHT * ARRAYSIZE(ShortcutItems) },
                .Items = ShortcutItems
            },
            { NULL },
        },
    };

    // Data will be aligned on an 8 byte boundary
    BYTE __declspec(align(8)) Data[4096];
    Config__DoDialogLayout(&Dialog, Data, sizeof(Data));

    return (BOOL)DialogBoxIndirectParam(GetModuleHandle(NULL), (LPCDLGTEMPLATE)Data, NULL, Config__DialogProc, (LPARAM)C);
}
