#pragma once

#include "windows.h"

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