#include "WindowUtil.h"
#include <psapi.h>

struct win_scope_handle
{
    HANDLE handle = nullptr;

    win_scope_handle(HANDLE h = nullptr)
        : handle(h)
    {
    }

    ~win_scope_handle()
    {
        ::CloseHandle(this->handle);
    }

    win_scope_handle& operator=(HANDLE h)
    {
        if (this->handle)
        {
            ::CloseHandle(this->handle);
        }
        this->handle = h;
        return *this;
    }

    operator bool() const
    {
        return !!handle;
    }
};

enum window_search_mode
{
    INCLUDE_MINIMIZED = 0,
    EXCLUDE_MINIMIZED = 1
};

static bool check_window_valid(HWND window, window_search_mode mode)
{
    DWORD styles, ex_styles;
    RECT rect;

    if (!IsWindowVisible(window) || (mode == window_search_mode::EXCLUDE_MINIMIZED && IsIconic(window)))
        return false;

    GetClientRect(window, &rect);
    styles = (DWORD)GetWindowLongPtr(window, GWL_STYLE);
    ex_styles = (DWORD)GetWindowLongPtr(window, GWL_EXSTYLE);

    if (ex_styles & WS_EX_TOOLWINDOW)
        return false;
    if (styles & WS_CHILD)
        return false;
    if (mode == window_search_mode::EXCLUDE_MINIMIZED && (rect.bottom == 0 || rect.right == 0))
        return false;

    return true;
}

static inline HWND next_window(HWND window, window_search_mode mode)
{
    while (true)
    {
        window = GetNextWindow(window, GW_HWNDNEXT);
        if (!window || check_window_valid(window, mode))
            break;
    }

    return window;
}

static inline HWND first_window(window_search_mode mode)
{
    HWND window = GetWindow(GetDesktopWindow(), GW_CHILD);
    if (!check_window_valid(window, mode))
        window = next_window(window, mode);
    return window;
}

static void get_window_title(std::wstring& name, HWND hwnd)
{
    int len = GetWindowTextLengthW(hwnd);
    if (!len)
        return;
    name.resize(len);
    GetWindowTextW(hwnd, const_cast<wchar_t*>(name.c_str()), len + 1);
}

static bool fill_window_info(WindowInfo& info, HWND hwnd)
{
    wchar_t wname[MAX_PATH];
    win_scope_handle process;
    DWORD processId = 0;
    DWORD threadId = GetWindowThreadProcessId(hwnd, &processId);
    if (!threadId)
        return false;

    if (threadId == GetCurrentProcessId())
        return false;

    process = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!process)
    {
        // std::cout << "err:" << GetLastError() << std::endl;
        return false;
    }

    if (!GetProcessImageFileNameW(process.handle, wname, MAX_PATH))
        return false;

    info.hwnd = hwnd;
    info.processId = processId;
    info.threadId = threadId;
    info.processName = wname;

    get_window_title(info.windowTitle, hwnd);

    return true;
}

static inline bool is_64bit_windows()
{
#ifdef _WIN64
    return true;
#else
    BOOL x86 = false;
    bool success = !!IsWow64Process(GetCurrentProcess(), &x86);
    return success && !!x86;
#endif
}

static inline bool is_64bit_process(HANDLE process)
{
    BOOL x86 = true;
    if (is_64bit_windows())
    {
        bool success = !!IsWow64Process(process, &x86);
        if (!success)
        {
            return false;
        }
    }

    return !x86;
}

static bool is_64bit_process(DWORD processId)
{
    win_scope_handle process = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        false, processId);
    if (!process)
    {
        return false;
    }
    return is_64bit_process(process.handle);
}

bool getWindowInfo(std::wstring window_name, WindowInfo& window_info)
{
    bool include_minimized = true;

    const auto mode = include_minimized ? window_search_mode::INCLUDE_MINIMIZED : window_search_mode::EXCLUDE_MINIMIZED;
    auto window = first_window(mode);
    while (window)
    {
        WindowInfo info = { 0 };
        if (fill_window_info(info, window))
        {
            if (info.processName.find(window_name) != std::wstring::npos)
            {
                window_info = info;
                return true;
            }
        }
        window = next_window(window, mode);
    }

    return false;
}
