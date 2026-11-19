
    MSG msg{};
    bool running = true;
    bool escapeWasDown = false;
    using GameClock = std::chrono::steady_clock;
    using TimerTicks = std::chrono::duration<LONGLONG, std::ratio<1, 10000000>>;
    auto lastTicks = GameClock::now();
    auto lastFrameLimitTicks = lastTicks;
    HANDLE frameTimer = CreateWaitableTimerExW(nullptr, nullptr,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_MODIFY_STATE | SYNCHRONIZE);
    auto limitGameFrameRate = [&]() {
        const int frameRateLimit = std::clamp(app.gameInputSettings.gameFrameRateLimit, 15, 144);
        const auto interval = std::chrono::duration<double>(1.0 / frameRateLimit);
        const auto remaining = interval - (GameClock::now() - lastFrameLimitTicks);
        const auto ticks = std::chrono::duration_cast<TimerTicks>(remaining).count();
        if (ticks > 0) {
            LARGE_INTEGER due{};
            due.QuadPart = -ticks;
            if (frameTimer && SetWaitableTimer(frameTimer, &due, 0, nullptr, nullptr, FALSE)) {
                WaitForSingleObject(frameTimer, INFINITE);
            } else {
                Sleep(static_cast<DWORD>((ticks + 9999) / 10000));
            }
        }
        lastFrameLimitTicks = GameClock::now();
    };
    while (running) {
