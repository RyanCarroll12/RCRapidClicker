#include <windows.h>
#include <stdint.h>
#include <uxtheme.h>
#include <strsafe.h>

#include "resource.h"
#include "config.h"
#include "inputs.h"

#pragma comment (lib, "uxtheme.lib")

#define WM_RC_ALREADY_RUNNING	(WM_USER+1)
#define WM_RC_COMMAND			(WM_USER+4)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

const char APP_NAME[] = "RC's Rapid Clicker";

const enum HotKeyID
{
	  TOGGLE_INPUT_EVENT = 1
	, SAVE_MOUSE_POSITION = 2
};

const enum MenuCMD
{
	  SETTINGS
	, QUIT
};

HICON rcIcon;
UINT WM_TASKBARCREATED;

BOOL clicking = FALSE;

POINT SavedCursorPosition = { 0 };
BOOL PointClicking = FALSE;

//globals
static Config gConfig;


void ShowNotification(HWND Window, LPCSTR Message, LPCSTR Title, DWORD Flags)
{
	NOTIFYICONDATA Data = { 0 };
	Data.cbSize = sizeof(Data);
	Data.hWnd = Window;
	Data.uFlags = NIF_INFO | NIF_TIP;
	Data.dwInfoFlags = Flags;
	StringCbCopy(Data.szTip, sizeof(Data.szTip), APP_NAME);
	StringCbCopy(Data.szInfo, sizeof(Data.szInfo), Message);
	StringCbCopy(Data.szInfoTitle, sizeof(Data.szInfoTitle), Title ? Title : APP_NAME);

	Shell_NotifyIcon(NIM_MODIFY, &Data);
}

void UpdateTrayIcon(HWND Window)
{
	NOTIFYICONDATA Data = { 0 };
	Data.cbSize = sizeof(Data);
	Data.hWnd = Window;
	Data.uFlags = NIF_ICON;
	Data.hIcon = rcIcon;

	Shell_NotifyIcon(NIM_MODIFY, &Data);
}

void AddTrayIcon(HWND Window)
{
	NOTIFYICONDATA Data = { 0 };
	Data.cbSize = sizeof(Data);
	Data.hWnd = Window;
	Data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	Data.uCallbackMessage = WM_RC_COMMAND;
	Data.hIcon = rcIcon;

	StringCbCopy(Data.szTip, 128, APP_NAME);
	Shell_NotifyIcon(NIM_ADD, &Data);
}

void RemoveTrayIcon(HWND Window)
{
	NOTIFYICONDATA Data = { 0 };
	Data.cbSize = sizeof(Data);
	Data.hWnd = Window;

	Shell_NotifyIcon(NIM_DELETE, &Data);
}

BOOL DisableHotKeys(HWND Window)
{
	BOOL Success = TRUE;

	Success = Success && UnregisterHotKey(Window, TOGGLE_INPUT_EVENT);

	Success = Success && UnregisterHotKey(Window, SAVE_MOUSE_POSITION);

	return Success;
}

BOOL EnableHotKeys(HWND Window)
{
	BOOL Success = TRUE;
	
	if (gConfig.ShortcutClicker)
	Success = Success && RegisterHotKey(Window, TOGGLE_INPUT_EVENT
		, HOT_KEY_GET_MOD(gConfig.ShortcutClicker) | MOD_NOREPEAT
		, HOT_KEY_GET_KEY(gConfig.ShortcutClicker));

	if (gConfig.ShortcutSavePosition)
	Success = Success && RegisterHotKey(Window, SAVE_MOUSE_POSITION
		, HOT_KEY_GET_MOD(gConfig.ShortcutSavePosition) | MOD_NOREPEAT
		, HOT_KEY_GET_KEY(gConfig.ShortcutSavePosition));

	return Success;
}

void process_key_state(uint64_t VKCode, uint64_t KeyMessageFlags)
{
	uint16_t repeat_count = KeyMessageFlags & 0xFFFF;
	uint8_t scan_code = (KeyMessageFlags >> 16) & 0xFF;
	BOOL is_extended_key = (KeyMessageFlags >> 24) & 0b1;
	BOOL context_code = (KeyMessageFlags >> 29) & 0b1;
	BOOL previous_state = (KeyMessageFlags >> 30) & 0b1;
	BOOL transition_state = (KeyMessageFlags >> 31) & 0b1;

	switch (VKCode)
	{
		case VK_SPACE:
		{
			printf("space pressed\n");

			break;
		}
	}
}

UINT execute_input_event(DWORD InputEvent)
{
	INPUT* inputs;
	int inputs_size;

	if (InputEvent == CONFIG_INPUT_LEFTCLICK)
	{
		inputs = LeftClick;
		inputs_size = ARRAY_SIZE(LeftClick);
	}
	else if (InputEvent == CONFIG_INPUT_ENTERKEY)
	{
		inputs = EnterKey;
		inputs_size = ARRAY_SIZE(EnterKey);
	}
	else if (InputEvent == CONFIG_INPUT_MOVEMOUSE)
	{
		inputs = MoveMouse;
		inputs_size = ARRAY_SIZE(MoveMouse);
	}
	else
	{
		//throw error
		MessageBox(NULL,
			"Invalid Input Event - RC",
			APP_NAME,
			0);

		return -1;
	}

	return SendInput(inputs_size, inputs, sizeof(INPUT));
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_DESTROY:
		{
			RemoveTrayIcon(Window);
			PostQuitMessage(0);
			break;
		}
		case WM_ACTIVATEAPP:
		{
			UpdateWindow(Window);
			break;
		}
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			process_key_state(wParam, lParam);
			break;
		}
		case WM_CHAR:
		{
			InvalidateRect(Window, NULL, TRUE);
			UpdateWindow(Window);
			break;
		}
		case WM_CREATE:
		{
			HRESULT result = BufferedPaintInit();
			if (FAILED(result))
			{
				return result;
			}
			AddTrayIcon(Window);
			break;
		}
		case WM_RC_COMMAND:
		{
			switch (LOWORD(lParam))
			{
				case WM_RBUTTONUP:
				{
					HMENU Menu = CreatePopupMenu();

					AppendMenu(Menu, MF_STRING | (clicking ? MF_DISABLED : 0), SETTINGS, "Settings");
					AppendMenu(Menu, MF_SEPARATOR, (UINT_PTR)NULL, NULL);
					AppendMenu(Menu, MF_STRING | (clicking ? MF_DISABLED : 0), QUIT, "Quit");

					POINT Mouse;
					GetCursorPos(&Mouse);

					SetForegroundWindow(Window);
					int Command = TrackPopupMenu(Menu, TPM_RETURNCMD | TPM_NONOTIFY, Mouse.x, Mouse.y, 0, Window, NULL);

					if (Command == SETTINGS)
					{
						if (Config_ShowDialog(&gConfig))
						{
							/*Config_Save(&gConfig, gConfigPath);*/
							DisableHotKeys(Window);
							EnableHotKeys(Window);
						}
					}
					else if (Command == QUIT)
					{
						DestroyWindow(Window);
					}

					DestroyMenu(Menu);

					break;
				}
				case WM_LBUTTONDBLCLK:
				{
					if (Config_ShowDialog(&gConfig))
					{
						/*Config_Save(&gConfig, gConfigPath);*/
						DisableHotKeys(Window);
						EnableHotKeys(Window);
					}

					break;
				}
			}

			break;
		}
		case WM_HOTKEY:
		{
			switch (wParam)
			{
				case TOGGLE_INPUT_EVENT:
				{
					clicking = !clicking;

					PointClicking = FALSE;

					break;
				}
				case SAVE_MOUSE_POSITION:
				{
					PointClicking = !PointClicking;

					clicking = FALSE;

					GetCursorPos(&SavedCursorPosition);

					break;
				}
			}

			if (clicking || PointClicking)
				// ES_SYSTEM_REQUIRED will reset system idle timer, forcing system to be in working state
				SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
			else
				SetThreadExecutionState(ES_CONTINUOUS);

			break;
		}
		case WM_RC_ALREADY_RUNNING:
		{
			ShowNotification(Window, "RC is already running!", NULL, NIIF_INFO);
			break;
		}
		default:
		{
			if (Message == WM_TASKBARCREATED)
			{
				// This is used to add the icon back if explorer.exe crashes
				AddTrayIcon(Window);
				return 0;
			}
			else
			{
				return DefWindowProc(Window, Message, wParam, lParam);
			}
		}
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	uint64_t PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	rcIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);

	WM_TASKBARCREATED = RegisterWindowMessage("TaskbarCreated");

	WNDCLASSEX WindowClass = { 0 };
	WindowClass.cbSize = sizeof(WindowClass);
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	WindowClass.lpfnWndProc = WindowProc;
	WindowClass.hInstance = hInstance;
	WindowClass.hIcon = rcIcon;
	WindowClass.lpszClassName = "rc_rapid_clicker_window_class";

	HWND Existing = FindWindow(WindowClass.lpszClassName, NULL);
	if (Existing)
	{
		PostMessage(Existing, WM_RC_ALREADY_RUNNING, 0, 0);
		ExitProcess(0);
	}

	if (!RegisterClassEx(&WindowClass))
	{
		MessageBox(NULL,
			"Call to RegisterClassEx failed!",
			APP_NAME,
			0);

		return 1;
	}


	int window_x = CW_USEDEFAULT;
	int window_y = CW_USEDEFAULT;
	int window_width = 300;
	int window_height = 300;

	HWND Window = CreateWindowEx(
		0,
		WindowClass.lpszClassName,
		APP_NAME,
		WS_POPUP, // Used for performance reasons, saves display data under new window so it won't need to be redrawn
		window_x, window_y, window_width, window_height,
		NULL,
		NULL,
		hInstance,
		NULL
	);


	if (!Window)
	{
		MessageBox(NULL,
			"Call to CreateWindowEx failed!",
			APP_NAME,
			0);

		return 1;
	}

	HDC hdc = GetDC(Window);

	UpdateWindow(Window);

	// Config initialization
	Config_Defaults(&gConfig);

	if (!EnableHotKeys(Window))
	{
		MessageBox(NULL,
			"Cannot register keyboard shortcuts.\nSome other application might already use shortcuts.\nPlease check & adjust the settings!",
			APP_NAME, MB_ICONEXCLAMATION);
	}

	//temp
	Config_ShowDialog(&gConfig);


	MSG message = { 0 };
	BOOL bRet;
	do
	{
		LARGE_INTEGER Counter = { 0 };
		LARGE_INTEGER ClickCounter = { 0 };

		while (clicking || PointClicking)
		{
			if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}

			POINT ClickPosition = (PointClicking) ? SavedCursorPosition : message.pt;

			QueryPerformanceCounter(&Counter);

			uint64_t CounterElapsed = Counter.QuadPart - ClickCounter.QuadPart;
			float TimeElapsed = (float) CounterElapsed / (float) PerfCountFrequency;

			float TargetFrequencyPeriod = 1.0f / gConfig.InputEventFrequency;

			if (TimeElapsed >= TargetFrequencyPeriod) // Possibly integrate perfcountfrequency into TargetFrequencyPeriod so it isn't calculated ever pass
			{
				ClickCounter = Counter;

				execute_input_event(gConfig.InputEvent);
			}
		}

		bRet = GetMessage(&message, NULL, 0, 0);

		if (bRet >= 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else
		{
			MessageBox(NULL,
				"Message returned error!",
				APP_NAME,
				0);

			return bRet;
		}
	} while (bRet);

	DisableHotKeys(Window);
	ReleaseDC(Window, hdc);

	return 0;
}