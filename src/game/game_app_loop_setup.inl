
    MSG msg{};
    bool running = true;
    bool escapeWasDown = false;
    ULONGLONG lastTicks = GetTickCount64();
    ULONGLONG lastFrameLimitTicks = lastTicks;
    auto limitGameFrameRate = [&]() {
        int frameRateLimit = std::clamp(app.gameInputSettings.gameFrameRateLimit, 15, 144);
        ULONGLONG now = GetTickCount64();
        ULONGLONG targetMs = static_cast<ULONGLONG>(std::max(1.0, 1000.0 / static_cast<double>(frameRateLimit)));
        ULONGLONG elapsedMs = now - lastFrameLimitTicks;
        if (elapsedMs < targetMs) {
            Sleep(static_cast<DWORD>(targetMs - elapsedMs));
            now = GetTickCount64();
        }
        lastFrameLimitTicks = now;
    };
    while (running) {
