ScreensaverWindowPlacement BuildScreensaverWindowPlacement(
    RunMode mode,
    HWND previewParent,
    bool diagnosticWindowMode,
    const std::vector<PlaybackMonitorRect>& playbackMonitors) {
    ScreensaverWindowPlacement placement{};
    placement.x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    placement.y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    placement.width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    placement.height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (!playbackMonitors.empty()) {
        const RECT& rc = playbackMonitors.front().rc;
        placement.x = rc.left;
        placement.y = rc.top;
        placement.width = std::max(1L, rc.right - rc.left);
        placement.height = std::max(1L, rc.bottom - rc.top);
    }

    if (diagnosticWindowMode) {
        placement.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        placement.exStyle = 0;
        placement.parent = nullptr;
        placement.x = 80;
        placement.y = 80;
        placement.width = 1100;
        placement.height = 820;
    } else if (mode == RunMode::Preview && previewParent) {
        RECT rc{};
        GetClientRect(previewParent, &rc);
        placement.x = 0;
        placement.y = 0;
        placement.width = std::max(1L, rc.right - rc.left);
        placement.height = std::max(1L, rc.bottom - rc.top);
        placement.style = WS_CHILD | WS_VISIBLE;
        placement.exStyle = 0;
        placement.parent = previewParent;
    }
    return placement;
}
