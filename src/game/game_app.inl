// BackroomsMazeGame.exe window creation and main loop.
// Included from main.cpp after WndProc, config, and game shell helpers are available.

void ApplyDefaultGuiFont(HWND hwnd) {
    if (!hwnd) return;
    HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

int RunGame(HINSTANCE hInstance) {
    const wchar_t* cls = L"BackroomsMazeGameWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    App app;
    app.preview = true;
    app.gameShell = true;
    app.gameInstance = hInstance;
    gApp = &app;

    Settings launchSettings = LoadSettings();
    app.gameInputSettings = launchSettings;
    int w = std::clamp(launchSettings.gameResolutionWidth, 640, 7680);
    int h = std::clamp(launchSettings.gameResolutionHeight, 360, 4320);
    int x = std::max(0, (GetSystemMetrics(SM_CXSCREEN) - w) / 2);
    int y = std::max(0, (GetSystemMetrics(SM_CYSCREEN) - h) / 2);
    DWORD style = launchSettings.gameFullscreen
        ? (WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
        : (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    if (launchSettings.gameFullscreen) {
        x = 0;
        y = 0;
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);
    }
    HWND hwnd = CreateWindowExW(0, cls, L"Backrooms Maze", style,
        x, y, w, h, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) {
        gApp = nullptr;
        return 1;
    }
    app.hwnd = hwnd;

    app.gameTitle = CreateWindowW(L"STATIC", L"Backrooms Maze", WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.gameSinglePlayer = CreateWindowW(L"BUTTON", L"Single Player", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameSinglePlayerId)), hInstance, nullptr);
    app.gameSettings = CreateWindowW(L"BUTTON", L"Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameSettingsId)), hInstance, nullptr);
    app.gameDebug = CreateWindowW(L"BUTTON", L"Debug", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameDebugId)), hInstance, nullptr);
    app.gameExit = CreateWindowW(L"BUTTON", L"Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameExitId)), hInstance, nullptr);
    app.gameBack = CreateWindowW(L"BUTTON", L"Back", WS_CHILD | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameBackId)), hInstance, nullptr);

    app.debugPrevEffect = CreateWindowW(L"BUTTON", L"< Effect", WS_CHILD | BS_PUSHBUTTON,
        12, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevEffectId)), hInstance, nullptr);
    app.debugNextEffect = CreateWindowW(L"BUTTON", L"Effect >", WS_CHILD | BS_PUSHBUTTON,
        110, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextEffectId)), hInstance, nullptr);
    app.debugSize = CreateWindowW(L"BUTTON", L"Grid: 3x3", WS_CHILD | BS_PUSHBUTTON,
        210, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSizeId)), hInstance, nullptr);
    app.debugReset = CreateWindowW(L"BUTTON", L"Reset anim", WS_CHILD | BS_PUSHBUTTON,
        322, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugResetId)), hInstance, nullptr);
    app.debugPrevProp = CreateWindowW(L"BUTTON", L"< Prop", WS_CHILD | BS_PUSHBUTTON,
        434, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevPropId)), hInstance, nullptr);
    app.debugNextProp = CreateWindowW(L"BUTTON", L"Prop >", WS_CHILD | BS_PUSHBUTTON,
        526, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextPropId)), hInstance, nullptr);
    app.debugSettings = CreateWindowW(L"BUTTON", L"Debug settings", WS_CHILD | BS_PUSHBUTTON,
        618, 10, 126, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSettingsId)), hInstance, nullptr);

    HWND controls[] = {
        app.gameTitle, app.gameSinglePlayer, app.gameSettings, app.gameDebug, app.gameExit, app.gameBack,
        app.debugPrevEffect, app.debugNextEffect, app.debugSize, app.debugReset, app.debugPrevProp, app.debugNextProp,
        app.debugSettings
    };
    for (HWND control : controls) ApplyDefaultGuiFont(control);

    LayoutGameControls(hwnd);
    SetGameMenuVisible(true);
    UpdateGameMenuLabels();
    SetGameCursorVisible(false);
    app.gameMenuFadeStart = GetTickCount64();
    SetDebugControlsVisible(false);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    EnsureGameRenderer(hwnd, RendererRuntimeMode::MainMenu);
    app.gameMenuFadeIn = true;
    app.gameMenuFadeOut = false;
    app.gameMenuFadeStart = GetTickCount64();
    InvalidateRect(hwnd, nullptr, FALSE);

    MSG msg{};
    bool running = true;
    bool escapeWasDown = false;
    ULONGLONG lastTicks = GetTickCount64();
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

        ULONGLONG now = GetTickCount64();
        float dt = std::min(0.05f, static_cast<float>(now - lastTicks) / 1000.0f);
        lastTicks = now;

        int pauseVk = GameActionKey(app.gameInputSettings, GameInputAction::Pause);
        bool escapeDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
        bool pauseDown = (GetAsyncKeyState(pauseVk) & 0x8000) != 0;
        if (!escapeDown) app.gameSettingsEscapeConsumed = false;
        if (pauseDown && !escapeWasDown) {
            if (app.gameState == GameState::PlayGame || app.gameState == GameState::DebugScene) {
                EnterGameMainMenu(hwnd);
            } else if (app.gameState == GameState::Settings && app.gameConfig) {
                if (!app.gameSettingsKeyCaptureActive && !app.gameSettingsEscapeConsumed) {
                    DestroyWindow(app.gameConfig);
                }
            } else if (app.gameState == GameState::MainMenu && app.gameRunStarted && !app.gameDebugActive) {
                ActivateGameMenuCommand(hwnd, kGameSinglePlayerId);
            }
        }
        escapeWasDown = pauseDown;

        if (app.gameState == GameState::MainMenu) {
            PushGameMenuInteractionToRenderer(hwnd);
            bool rendererMenuScene = app.rendererInitialized &&
                app.renderer.RuntimeMode() == RendererRuntimeMode::MainMenu;
            if (rendererMenuScene) {
                app.renderer.TickFixed(dt);
            }
            HDC dc = GetDC(hwnd);
            if (dc) {
                PaintGameMainMenu(hwnd, dc);
                ReleaseDC(hwnd, dc);
            }
            UpdateGameMenuTransition(hwnd);
        }

        if (app.rendererInitialized &&
            (app.gameState == GameState::PlayGame || app.gameState == GameState::DebugScene)) {
            if (app.gameState == GameState::PlayGame) {
                GameInputSnapshot input = CollectGameInput();
                app.renderer.SetGameInput(input);
            } else {
                app.renderer.SetGameInput({});
            }
            app.renderer.TickFixed(dt);
            if (app.gameState == GameState::DebugScene) RedrawDebugSliceControls();
        } else if (app.gameState != GameState::MainMenu) {
            Sleep(10);
        }
        Sleep(1);
    }

    ReleaseGameMouse();
    gApp = nullptr;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    return static_cast<int>(msg.wParam);
}
