// Screensaver renderer initialization smoke test entry.
// Included from screensaver_app.inl after shared screensaver window helpers.

int RunSelfTest(HINSTANCE hInstance) {
    const wchar_t* cls = L"BackroomsMazeScreensaverSelfTest";
    RegisterScreensaverWindowClass(hInstance, cls);

    auto appHolder = std::make_unique<App>();
    App& app = *appHolder;
    app.preview = true;
    gApp = &app;
    HWND hwnd = CreateWindowExW(0, cls, L"Backrooms Maze Self Test", WS_POPUP, 0, 0, 320, 180, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) {
        gApp = nullptr;
        return 2;
    }
    bool ok = app.renderer.Initialize(hwnd);
    StartupProfileLine(L"SelfTest after Initialize");
    app.renderer.SetPresentSyncInterval(0);
    app.renderer.SetPresentEnabled(false);
    StartupProfileLine(L"SelfTest before DestroyWindow");
    DestroyWindow(hwnd);
    StartupProfileLine(L"SelfTest after DestroyWindow");
    gApp = nullptr;
    appHolder.release();
    StartupProfileLine(L"SelfTest before return");
    return ok ? 0 : 3;
}
