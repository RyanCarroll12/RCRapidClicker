#include <windows.h>
#include <stdint.h>
#include <uxtheme.h>
#include <strsafe.h>

#include <iostream>
#include "resource.h"

#pragma comment (lib, "uxtheme.lib")

#define WM_RC_ALREADY_RUNNING	(WM_USER+1)
#define WM_RC_COMMAND			(WM_USER+4)

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

const char APP_NAME[] = "RC's Rapid Clicker";

const enum HotKeyID
{
	  TOGGLE_CLICKING = 1
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

float ClickSpeed = 1.0f/60.0f;

POINT SavedCursorPosition = {};
BOOL PointClicking = FALSE;


void ShowNotification(HWND Window, LPCSTR Message, LPCSTR Title, DWORD Flags)
{
	NOTIFYICONDATA Data = {};
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
	NOTIFYICONDATA Data = { };
	Data.cbSize = sizeof(Data);
	Data.hWnd = Window;
	Data.uFlags = NIF_ICON;
	Data.hIcon = rcIcon;

	Shell_NotifyIcon(NIM_MODIFY, &Data);
}

void AddTrayIcon(HWND Window)
{
	NOTIFYICONDATA Data = { };
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
	NOTIFYICONDATA Data = { };
	Data.cbSize = sizeof(Data);
	Data.hWnd = Window;

	Shell_NotifyIcon(NIM_DELETE, &Data);
}

BOOL DisableHotKeys(HWND Window)
{
	BOOL Success = TRUE;

	Success = Success && UnregisterHotKey(Window, HotKeyID::TOGGLE_CLICKING);

	Success = Success && UnregisterHotKey(Window, HotKeyID::SAVE_MOUSE_POSITION);

	return Success;
}

BOOL EnableHotKeys(HWND Window)
{
	BOOL Success = TRUE;
	
	Success = Success && RegisterHotKey(Window, HotKeyID::TOGGLE_CLICKING, MOD_CONTROL | MOD_NOREPEAT, VK_NUMPAD0);

	Success = Success && RegisterHotKey(Window, HotKeyID::SAVE_MOUSE_POSITION, MOD_CONTROL | MOD_NOREPEAT, VK_DECIMAL);

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
			std::cout << "space pressed" << std::endl;

			break;
		}
	}
}

INT_PTR Dlgproc(
	HWND unnamedParam1,
	UINT unnamedParam2,
	WPARAM unnamedParam3,
	LPARAM unnamedParam4
)
{
	return 0;
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

					AppendMenu(Menu, MF_STRING | (clicking ? MF_DISABLED : 0), MenuCMD::SETTINGS, "Settings");
					AppendMenu(Menu, MF_SEPARATOR, NULL, NULL);
					AppendMenu(Menu, MF_STRING | (clicking ? MF_DISABLED : 0), MenuCMD::QUIT, "Quit");

					POINT Mouse;
					GetCursorPos(&Mouse);

					SetForegroundWindow(Window);
					int Command = TrackPopupMenu(Menu, TPM_RETURNCMD | TPM_NONOTIFY, Mouse.x, Mouse.y, 0, Window, NULL);

					if (Command == MenuCMD::SETTINGS)
					{
						DLGTEMPLATE DTemplate;
						// CreateDialog(GetModuleHandle(NULL), DTemplate, Window, Dlgproc)
						break;
					}
					else if (Command == MenuCMD::QUIT)
					{
						DestroyWindow(Window);
					}

					DestroyMenu(Menu);

					break;
				}
				case WM_LBUTTONDBLCLK:
				{
					/*if (Config_ShowDialog())
					{
						Config_Save(&gConfig, gConfigPath);
						DisableHotKeys(Window);
						EnableHotKeys(Window);
					}*/

					break;
				}
			}

			break;
		}
		case WM_HOTKEY:
		{
			switch (wParam)
			{
				case HotKeyID::TOGGLE_CLICKING:
				{
					clicking = !clicking;

					PointClicking = FALSE;

					break;
				}
				case HotKeyID::SAVE_MOUSE_POSITION:
				{
					PointClicking = !PointClicking;

					clicking = FALSE;

					GetCursorPos(&SavedCursorPosition);

					break;
				}
			}

			if (clicking || PointClicking)
				SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
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

	WNDCLASSEX WindowClass = { };
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
			NULL);

		return 1;
	}


	int window_x = CW_USEDEFAULT;
	int window_y = CW_USEDEFAULT;
	int window_width = 300;
	int window_height = 300;

	HWND Window = CreateWindowEx(
		NULL,
		WindowClass.lpszClassName,
		APP_NAME,
		NULL,// WS_SYSMENU | WS_CAPTION | WS_VISIBLE,
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
			NULL);

		return 1;
	}

	HDC hdc = GetDC(Window);

	UpdateWindow(Window);

	if (!EnableHotKeys(Window))
	{
		MessageBox(NULL,
			"Cannot register keyboard shortcuts.\nSome other application might already use shortcuts.\nPlease check & adjust the settings!",
			APP_NAME, MB_ICONEXCLAMATION);
	}

	MSG message = { };
	BOOL bRet;
	do
	{
		LARGE_INTEGER Counter = {};
		LARGE_INTEGER ClickCounter = {};

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

			if (TimeElapsed >= ClickSpeed) // Possibly integrate perfcountfrequency into clickspeed so it isn't calculated ever pass
			{
				ClickCounter = Counter;

				mouse_event(MOUSEEVENTF_LEFTDOWN, ClickPosition.x, ClickPosition.y, 0, 0);
				mouse_event(MOUSEEVENTF_LEFTUP, ClickPosition.x, ClickPosition.y, 0, 0);

				/*INPUT left_down = {};
				left_down.type = INPUT_MOUSE;
				left_down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				INPUT left_up = {};
				left_up.type = INPUT_MOUSE;
				left_up.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
				SendInput(1, &left_down, sizeof(INPUT));
				SendInput(1, &left_up, sizeof(INPUT));*/
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
				NULL);

			return bRet;
		}
	} while (bRet);

	DisableHotKeys(Window);
	ReleaseDC(Window, hdc);

	return 0;
}