
    MSG msg{};
    bool running = true;
    bool escapeWasDown = false;
    ULONGLONG lastTicks = GetTickCount64();
    while (running) {
