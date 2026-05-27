// BackroomsMaze.scr fullscreen/preview/diagnostic host and self-test.
// Included from main.cpp after config, loading overlay, and shared window helpers.

struct PlaybackMonitorRect {
    RECT rc{};
    bool primary = false;
};

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

int RunScreensaver(HINSTANCE hInstance, RunMode mode, HWND previewParent) {
    bool monsterPreviewMode =
        mode == RunMode::MonsterPreview ||
        mode == RunMode::MonsterPreviewFront ||
        mode == RunMode::MonsterPreviewSide ||
        mode == RunMode::MonsterPreviewLeftSide ||
        mode == RunMode::MonsterPreviewTop;
    bool bloodDebugMode = mode == RunMode::BloodDebug;
    gEffectDebugViewer = bloodDebugMode;
    if (bloodDebugMode) {
        gDebugSliceEffect = DebugSliceEffect::Blood;
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    }
    gBloodDebugEveryWall = gEffectDebugViewer &&
        (gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect));
    bool diagnosticWindowMode = monsterPreviewMode || bloodDebugMode;
    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit;
    if (mode == RunMode::MonsterPreviewFront) monsterPreviewView = MonsterPreviewView::Front;
    else if (mode == RunMode::MonsterPreviewSide) monsterPreviewView = MonsterPreviewView::Side;
    else if (mode == RunMode::MonsterPreviewLeftSide) monsterPreviewView = MonsterPreviewView::LeftSide;
    else if (mode == RunMode::MonsterPreviewTop) monsterPreviewView = MonsterPreviewView::Top;

    const wchar_t* cls = L"BackroomsMazeScreensaverWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    App app;
    app.preview = mode == RunMode::Preview || diagnosticWindowMode;
    gApp = &app;

    std::vector<PlaybackMonitorRect> playbackMonitors;
    if (mode == RunMode::Fullscreen) {
        playbackMonitors = EnumeratePlaybackMonitors();
    }

    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_TOPMOST;
    HWND parent = nullptr;
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (!playbackMonitors.empty()) {
        const RECT& rc = playbackMonitors.front().rc;
        x = rc.left;
        y = rc.top;
        w = std::max(1L, rc.right - rc.left);
        h = std::max(1L, rc.bottom - rc.top);
    }

    if (diagnosticWindowMode) {
        style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        exStyle = 0;
        parent = nullptr;
        x = 80;
        y = 80;
        w = 1100;
        h = 820;
    } else if (mode == RunMode::Preview && previewParent) {
        RECT rc{};
        GetClientRect(previewParent, &rc);
        x = 0;
        y = 0;
        w = std::max(1L, rc.right - rc.left);
        h = std::max(1L, rc.bottom - rc.top);
        style = WS_CHILD | WS_VISIBLE;
        exStyle = 0;
        parent = previewParent;
    }

    HWND hwnd = CreateWindowExW(exStyle, cls, L"Backrooms Maze", style, x, y, w, h, parent, nullptr, hInstance, nullptr);
    if (!hwnd) return 1;
    app.hwnd = hwnd;

    if (!app.preview && playbackMonitors.size() > 1) {
        for (size_t i = 1; i < playbackMonitors.size(); ++i) {
            const RECT& rc = playbackMonitors[i].rc;
            auto clone = std::make_unique<App::CloneOutput>();
            clone->hwnd = CreateWindowExW(exStyle, cls, L"Backrooms Maze", style,
                rc.left, rc.top,
                std::max(1L, rc.right - rc.left),
                std::max(1L, rc.bottom - rc.top),
                nullptr, nullptr, hInstance, nullptr);
            if (clone->hwnd) {
                app.clones.push_back(std::move(clone));
            }
        }
    }

    if (gEffectDebugViewer) {
        gApp->debugPrevEffect = CreateWindowW(L"BUTTON", L"< Effect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            12, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevEffectId)), hInstance, nullptr);
        gApp->debugNextEffect = CreateWindowW(L"BUTTON", L"Effect >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            110, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextEffectId)), hInstance, nullptr);
        gApp->debugSize = CreateWindowW(L"BUTTON", L"Grid: 3x3", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            210, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSizeId)), hInstance, nullptr);
        gApp->debugReset = CreateWindowW(L"BUTTON", L"Reset anim", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            322, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugResetId)), hInstance, nullptr);
        gApp->debugPrevProp = CreateWindowW(L"BUTTON", L"< Prop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            434, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevPropId)), hInstance, nullptr);
        gApp->debugNextProp = CreateWindowW(L"BUTTON", L"Prop >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            526, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextPropId)), hInstance, nullptr);
        UpdateDebugSliceControls(hwnd);
        RedrawDebugSliceControls();
    }

    app.loadingOverlay = CreateLoadingOverlay(hwnd, hInstance);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    for (auto& clone : app.clones) {
        if (!clone || !clone->hwnd) continue;
        clone->loadingOverlay = CreateLoadingOverlay(clone->hwnd, hInstance);
        ShowWindow(clone->hwnd, SW_SHOW);
        UpdateWindow(clone->hwnd);
    }
    if (!app.preview) ShowCursor(FALSE);

    Settings fullscreenSettings;
    const Settings* rendererSettings = nullptr;
    if (mode == RunMode::Fullscreen) {
        fullscreenSettings = LoadSettings();
        fullscreenSettings.mazeSeed = ResolveRuntimeSeed(fullscreenSettings.mazeSeed);
        rendererSettings = &fullscreenSettings;
    }

    StartupProgressSink loadingProgress{LoadingProgressCallback, app.loadingOverlay};
    if (!app.renderer.Initialize(hwnd, rendererSettings, monsterPreviewMode, monsterPreviewView,
            app.loadingOverlay ? &loadingProgress : nullptr)) {
        if (app.loadingOverlay) {
            DestroyWindow(app.loadingOverlay);
            app.loadingOverlay = nullptr;
        }
        MessageBoxW(hwnd, L"Direct3D initialization failed.", L"Backrooms Maze", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        if (!app.preview) ShowCursor(TRUE);
        return 1;
    }
    for (size_t i = 0; i < app.clones.size(); ++i) {
        auto& clone = app.clones[i];
        if (!clone || !clone->hwnd) continue;
        if (app.loadingOverlay) {
            std::wstringstream detail;
            detail << L"Preparing cloned display " << (i + 2) << L"/" << (app.clones.size() + 1) << L".";
            SetLoadingOverlayStatus(app.loadingOverlay, L"Preparing displays", detail.str().c_str(), false);
        }
        StartupProgressSink cloneProgress{LoadingProgressCallback, clone->loadingOverlay};
        if (!clone->renderer.Initialize(clone->hwnd, rendererSettings, false, MonsterPreviewView::Orbit,
                clone->loadingOverlay ? &cloneProgress : nullptr)) {
            if (clone->loadingOverlay) {
                DestroyWindow(clone->loadingOverlay);
                clone->loadingOverlay = nullptr;
            }
            MessageBoxW(hwnd, L"Direct3D initialization failed on a cloned display.", L"Backrooms Maze", MB_OK | MB_ICONERROR);
            QuitScreensaver(hwnd);
            if (!app.preview) ShowCursor(TRUE);
            return 1;
        }
    }
    app.loadingWarmupPending = app.loadingOverlay != nullptr;
    if (app.loadingWarmupPending) {
        SetLoadingOverlayStatus(app.loadingOverlay, L"Warming first frame",
            L"Starting the first GPU frame.", false);
    }
    for (auto& clone : app.clones) {
        if (clone && clone->loadingOverlay) {
            clone->loadingWarmupPending = true;
            SetLoadingOverlayStatus(clone->loadingOverlay, L"Warming first frame",
                L"Starting the first GPU frame.", false);
        }
    }

    MSG msg{};
    bool running = true;
    ULONGLONG playbackLastTicks = GetTickCount64();
    auto warmupOutput = [](Renderer& renderer, HWND owner, HWND& overlay,
                           bool& pending, ULONGLONG& start, int& attempts) -> bool {
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
            DestroyWindow(overlay);
            overlay = nullptr;
        }
        pending = false;
        InvalidateRect(owner, nullptr, FALSE);
        UpdateWindow(owner);
        return false;
    };

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (running) {
            bool hadWarmup = app.loadingWarmupPending;
            bool warmupStillPending = warmupOutput(app.renderer, hwnd, app.loadingOverlay,
                app.loadingWarmupPending, app.loadingWarmupStart, app.loadingWarmupAttempts);
            for (auto& clone : app.clones) {
                if (!clone || !clone->hwnd) continue;
                hadWarmup = hadWarmup || clone->loadingWarmupPending;
                warmupStillPending = warmupOutput(clone->renderer, clone->hwnd, clone->loadingOverlay,
                    clone->loadingWarmupPending, clone->loadingWarmupStart, clone->loadingWarmupAttempts) || warmupStillPending;
            }
            if (hadWarmup) {
                playbackLastTicks = GetTickCount64();
                if (warmupStillPending) continue;
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
    }

    if (!app.preview) ShowCursor(TRUE);
    gApp = nullptr;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    return static_cast<int>(msg.wParam);
}

int RunSelfTest(HINSTANCE hInstance) {
    const wchar_t* cls = L"BackroomsMazeScreensaverSelfTest";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

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
