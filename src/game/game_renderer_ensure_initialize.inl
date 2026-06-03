    StartupProgressSink loadingProgress{LoadingProgressCallback, gApp->loadingOverlay};
    int oldThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
    bool loweredStartupPriority = LoadingOverlayHasIndependentSplash(gApp->loadingOverlay);
    if (loweredStartupPriority) {
        oldThreadPriority = GetThreadPriority(GetCurrentThread());
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    }
    bool initialized = gApp->renderer.Initialize(hwnd, nullptr, false, MonsterPreviewView::Orbit,
        gApp->loadingOverlay ? &loadingProgress : nullptr);
    if (oldThreadPriority != THREAD_PRIORITY_ERROR_RETURN) {
        SetThreadPriority(GetCurrentThread(), oldThreadPriority);
    }
    if (!initialized) {
        if (gApp->loadingOverlay) {
            CloseLoadingOverlayWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        std::wstring detail = gApp->renderer.LastInitializeError();
        std::wstring message = detail.empty()
            ? L"Direct3D initialization failed."
            : L"Direct3D initialization failed.\r\n\r\n" + detail;
        MessageBoxW(hwnd, message.c_str(), L"Backrooms Maze Game", MB_OK | MB_ICONERROR);
        return false;
    }
