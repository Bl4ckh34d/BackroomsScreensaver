// Screensaver loading warmup and playback message loop.
// Included from screensaver_app.inl after renderer startup helpers.

bool WarmupScreensaverOutput(
    Renderer& renderer,
    HWND owner,
    HWND& overlay,
    bool& pending,
    ULONGLONG& start,
    int& attempts) {
    if (!pending) return false;
    if (start == 0) start = GetTickCount64();
    renderer.SetPresentSyncInterval(0);
    renderer.SetPresentFlags(DXGI_PRESENT_DO_NOT_WAIT);
    renderer.TickFixed(0.0f);
    renderer.SetPresentFlags(0);
    renderer.SetPresentSyncInterval(1);
    ++attempts;
    ULONGLONG warmupElapsed = GetTickCount64() - start;
    if (!renderer.LastPresentCompleted() && attempts < 3 && warmupElapsed < 1500) {
        Sleep(50);
        return true;
    }
    if (overlay) {
        SetLoadingOverlayStatus(overlay, L"Ready", L"Entering maze.", true);
        CloseLoadingOverlayWindow(overlay);
        overlay = nullptr;
    }
    pending = false;
    InvalidateRect(owner, nullptr, FALSE);
    UpdateWindow(owner);
    return false;
}
