#include "../platform/platform_headers.h"

#include "screensaver_monitor_layout.h"

BOOL CALLBACK EnumPlaybackMonitorProc(HMONITOR monitor, HDC, LPRECT, LPARAM lParam) {
    auto* monitors = reinterpret_cast<std::vector<PlaybackMonitorRect>*>(lParam);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) return TRUE;
    RECT rc = info.rcMonitor;
    if (rc.right <= rc.left || rc.bottom <= rc.top) return TRUE;
    monitors->push_back({rc, (info.dwFlags & MONITORINFOF_PRIMARY) != 0});
    return TRUE;
}

std::vector<PlaybackMonitorRect> EnumeratePlaybackMonitors() {
    std::vector<PlaybackMonitorRect> monitors;
    EnumDisplayMonitors(nullptr, nullptr, EnumPlaybackMonitorProc, reinterpret_cast<LPARAM>(&monitors));
    if (monitors.empty()) {
        RECT rc{
            GetSystemMetrics(SM_XVIRTUALSCREEN),
            GetSystemMetrics(SM_YVIRTUALSCREEN),
            GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN),
            GetSystemMetrics(SM_YVIRTUALSCREEN) + GetSystemMetrics(SM_CYVIRTUALSCREEN)
        };
        monitors.push_back({rc, true});
    }
    std::stable_sort(monitors.begin(), monitors.end(), [](const PlaybackMonitorRect& a, const PlaybackMonitorRect& b) {
        if (a.primary != b.primary) return a.primary;
        if (a.rc.top != b.rc.top) return a.rc.top < b.rc.top;
        return a.rc.left < b.rc.left;
    });
    return monitors;
}
