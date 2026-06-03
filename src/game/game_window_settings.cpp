#include "../platform/platform_headers.h"
#include "../config/settings.h"

#include "game_window_settings.h"

void ApplyGameWindowSettings(HWND hwnd, const Settings& settings) {
    if (!hwnd) return;
    if (settings.gameFullscreen) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    } else {
        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        RECT wr{0, 0, settings.gameResolutionWidth, settings.gameResolutionHeight};
        AdjustWindowRect(&wr, style, FALSE);
        int w = wr.right - wr.left;
        int h = wr.bottom - wr.top;
        int x = std::max(0, (GetSystemMetrics(SM_CXSCREEN) - w) / 2);
        int y = std::max(0, (GetSystemMetrics(SM_CYSCREEN) - h) / 2);
        SetWindowLongPtrW(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, w, h, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}
