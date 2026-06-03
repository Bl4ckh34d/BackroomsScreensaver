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
