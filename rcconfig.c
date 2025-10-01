#include "rcconfig.h"


// currently open dialog window
static HWND gDialogWindow;

Config__DialogItem ClickerOptionsItems[] =
{
      { "Input Type",                  ID_INPUT_EVENT,        ITEM_COMBOBOX, COMBOBOX_TEXT_LENGTH }
    , { "Target Frequency",            ID_INPUT_FREQUENCY,    ITEM_NUMBER,   COMBOBOX_TEXT_LENGTH }
    , { NULL }
};

Config__DialogItem ShortcutItems[] =
{
      { "Rapid Clicker", ID_SHORTCUT_CLICKER,      ITEM_HOTKEY, 64 }
    , { "Set Position",  ID_SHORTCUT_SAVEPOSITION, ITEM_HOTKEY, 64 }
    , { NULL }
};


void Config_Defaults(Config* C)
{
    int DefaultEventType = INPUT_LEFTCLICK;
    set_input_event(DefaultEventType);
    *C = (Config)
    {
        // input
          .InputEventType = DefaultEventType
        , .InputTargetFrequency = 250
        // shortcuts
        , .ShortcutClicker = HOT_KEY(VK_TAB, MOD_SHIFT)
        , .ShortcutSavePosition = HOT_KEY(VK_DECIMAL, MOD_CONTROL | MOD_SHIFT)
    };
}

static void Config__FormatKey(DWORD KeyMod, WCHAR* Text, size_t TextBufferSize)
{
    if (KeyMod == 0)
    {
        StringCbCopyW(Text, TextBufferSize, L"[none]");
        return;
    }
    else
    {
        StringCbCopyW(Text, TextBufferSize, L"");
    }

    DWORD Mod = HOT_KEY_GET_MOD(KeyMod);

    Text[0] = 0;
    if (Mod & MOD_CONTROL) StringCbCatW(Text, TextBufferSize, L"Ctrl + ");
    if (Mod & MOD_WIN)     StringCbCatW(Text, TextBufferSize, L"Win + ");
    if (Mod & MOD_SHIFT)   StringCbCatW(Text, TextBufferSize, L"Shift + ");
    if (Mod & MOD_ALT)     StringCbCatW(Text, TextBufferSize, L"Alt + ");

    struct
    {
        DWORD Key;
        WCHAR* Text;
    }
    static const Overrides[] =
    {
          { VK_PAUSE,    L"Pause"    }
        , { VK_SNAPSHOT, L"PrtScr"   }
        , { VK_PRIOR,    L"PageUp"   }
        , { VK_NEXT,     L"PageDown" }
        , { VK_END,      L"End"      }
        , { VK_HOME,     L"Home"     }
        , { VK_LEFT,     L"Left"     }
        , { VK_UP,       L"Up"       }
        , { VK_RIGHT,    L"Right"    }
        , { VK_DOWN,     L"Down"     }
        , { VK_INSERT,   L"Insert"   }
        , { VK_DELETE,   L"Delete"   }
    };

    for (size_t i = 0; i < ARRAYSIZE(Overrides); i++)
    {
        if (Overrides[i].Key == HOT_KEY_GET_KEY(KeyMod))
        {
            StringCbCatW(Text, TextBufferSize, Overrides[i].Text);
            return;
        }
    }

    WCHAR KeyText[32];
    UINT ScanCode = MapVirtualKeyW(HOT_KEY_GET_KEY(KeyMod), MAPVK_VK_TO_VSC);
    if (GetKeyNameTextW(ScanCode << 16, KeyText, _countof(KeyText)) == 0)
    {
        _snwprintf_s(KeyText, ARRAYSIZE(KeyText), ARRAYSIZE(KeyText), L"[0x%02x]", HOT_KEY_GET_KEY(KeyMod));
    }

    StringCbCatW(Text, TextBufferSize, KeyText);
}

static void Config__SetDialogValues(HWND Window, Config* C)
{
    // input event
    SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_SETCURSEL, C->InputEventType, 0);
    SetDlgItemInt(Window, ID_INPUT_FREQUENCY, C->InputTargetFrequency, FALSE);

    // shortcuts
    WCHAR Text[64];
    Config__FormatKey(C->ShortcutClicker, Text, sizeof(Text));
    SetDlgItemTextW(Window, ID_SHORTCUT_CLICKER, Text);
    SetWindowLongPtrW(GetDlgItem(Window, ID_SHORTCUT_CLICKER), GWLP_USERDATA, C->ShortcutClicker);

    Config__FormatKey(C->ShortcutSavePosition, Text, sizeof(Text));
    SetDlgItemTextW(Window, ID_SHORTCUT_SAVEPOSITION, Text);
    SetWindowLongPtrW(GetDlgItem(Window, ID_SHORTCUT_SAVEPOSITION), GWLP_USERDATA, C->ShortcutSavePosition);
}

static LRESULT CALLBACK Config__DialogProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    if (Message == WM_INITDIALOG)
    {
        Config* C = (Config*)LParam;
        SetWindowLongPtrW(Window, GWLP_USERDATA, (LONG_PTR)C);
        for (int i = 0; i < MAX_INPUT; i++)
        {
            SendDlgItemMessageW(Window, ID_INPUT_EVENT, CB_INSERTSTRING, InputEvents[i].type, (LPARAM)InputEvents[i].name);
        }

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
            C->InputEventType = (DWORD)SendDlgItemMessage(Window, ID_INPUT_EVENT, CB_GETCURSEL, 0, 0);
            set_input_event(C->InputEventType);
            C->InputTargetFrequency = GetDlgItemInt(Window, ID_INPUT_FREQUENCY, NULL, FALSE);
            
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
        else if ((Control == ID_SHORTCUT_CLICKER || Control == ID_SHORTCUT_SAVEPOSITION) && HIWORD(WParam) == BN_CLICKED)
        {
            if (gConfigShortcut.Control == 0)
            {
                SetDlgItemTextW(Window, Control, L"Press new shortcut, [ESC] to cancel, [BACKSPACE] to disable");

                gConfigShortcut.Control = Control;
                gConfigShortcut.Config = C;

                HWND ControlWindow = GetDlgItem(Window, Control);
                gConfigShortcut.WindowProc = (WNDPROC)GetWindowLongPtr(ControlWindow, GWLP_WNDPROC);
                /*SetWindowLongPtr(ControlWindow, GWLP_WNDPROC, (LONG_PTR)&Config__ShortcutProc);
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

    int ButtonX = DIALOG_X_SIZE;
    int ButtonY = DIALOG_Y_SIZE - (PADDING + ITEM_HEIGHT);

    DLGITEMTEMPLATE* DefaultsData = Config__Align(Data, sizeof(DWORD));
    Data = Config__DoDialogItem(Data, "Defaults", ID_DEFAULTS, CONTROL_BUTTON, WS_TABSTOP | BS_PUSHBUTTON, ButtonX -= BUTTON_WIDTH + PADDING, ButtonY, BUTTON_WIDTH, ITEM_HEIGHT);

    DLGITEMTEMPLATE* CancelData = Config__Align(Data, sizeof(DWORD));
    Data = Config__DoDialogItem(Data, "Cancel", ID_CANCEL, CONTROL_BUTTON, WS_TABSTOP | BS_PUSHBUTTON, ButtonX -= BUTTON_WIDTH + PADDING, ButtonY, BUTTON_WIDTH, ITEM_HEIGHT);

    DLGITEMTEMPLATE* OkData = Config__Align(Data, sizeof(DWORD));
    Data = Config__DoDialogItem(Data, "OK", ID_OK, CONTROL_BUTTON, WS_TABSTOP | BS_DEFPUSHBUTTON, ButtonX -= BUTTON_WIDTH + PADDING, ButtonY, BUTTON_WIDTH, ITEM_HEIGHT);

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

    Assert(Data <= End);
}

BOOL Config_ShowDialog(Config* C)
{
    if (gDialogWindow)
    {
        SetForegroundWindow(gDialogWindow);
        return FALSE;
    }

    int DialogItemHeight = 0;
    int DialogItemWidth = DIALOG_X_SIZE - (PADDING * 2);

    Config__DialogRect ClickerOptionsRect[] =
    { 0, DialogItemHeight, DialogItemWidth, ITEM_HEIGHT * ARRAYSIZE(ClickerOptionsItems) };
    DialogItemHeight += ITEM_HEIGHT * ARRAYSIZE(ClickerOptionsItems);

    Config__DialogRect ShortcutRect[] =
    { 0, DialogItemHeight, DialogItemWidth, ITEM_HEIGHT * ARRAYSIZE(ShortcutItems) };
    DialogItemHeight += ITEM_HEIGHT * ARRAYSIZE(ShortcutItems);

    Config__DialogLayout Dialog = (Config__DialogLayout)
    {
        .Title = CONFIG_TITLE,
        .Font = "Segoe UI",
        .FontSize = 9,
        .Groups = (Config__DialogGroup[])
        {
            {
                .Caption = "&Clicker Options",
                .Rect = *ClickerOptionsRect,
                .Items = ClickerOptionsItems
            },
            {
                .Caption = "Shor&tcuts",
                .Rect = *ShortcutRect,
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
