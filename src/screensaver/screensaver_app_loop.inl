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

bool WarmupScreensaverOutputs(App& app, HWND hwnd) {
    bool hadWarmup = app.loadingWarmupPending;
    bool warmupStillPending = WarmupScreensaverOutput(app.renderer, hwnd, app.loadingOverlay,
        app.loadingWarmupPending, app.loadingWarmupStart, app.loadingWarmupAttempts);
    for (auto& clone : app.clones) {
        if (!clone || !clone->hwnd) continue;
        hadWarmup = hadWarmup || clone->loadingWarmupPending;
        warmupStillPending = WarmupScreensaverOutput(clone->renderer, clone->hwnd, clone->loadingOverlay,
            clone->loadingWarmupPending, clone->loadingWarmupStart, clone->loadingWarmupAttempts) || warmupStillPending;
    }
    return hadWarmup || warmupStillPending;
}

int RunScreensaverMessageLoop(App& app, HWND hwnd) {
    MSG msg{};
    bool running = true;
    ULONGLONG playbackLastTicks = GetTickCount64();
    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;

        if (WarmupScreensaverOutputs(app, hwnd)) {
            playbackLastTicks = GetTickCount64();
            continue;
        }

        ULONGLONG now = GetTickCount64();
        float dt = std::min(0.05f, static_cast<float>(now - playbackLastTicks) / 1000.0f);
        playbackLastTicks = now;
        if (app.clones.empty()) {
            app.renderer.Tick();
        } else {
            app.renderer.TickFixed(dt);
            for (auto& clone : app.clones) {
                if (clone && clone->hwnd) clone->renderer.TickFixed(dt);
            }
        }
        RedrawDebugSliceControls();
        Sleep(1);
    }
    return static_cast<int>(msg.wParam);
}
