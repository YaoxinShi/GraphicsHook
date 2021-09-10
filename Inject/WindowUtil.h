#pragma once

#include <Windows.h>
#include <string>
#include <vector>

struct WindowInfo
{
	HWND hwnd;
	DWORD processId;
	DWORD threadId;
	std::wstring windowTitle;
	std::wstring processName;
};

bool getWindowInfo(std::wstring window_name, WindowInfo& window_info);