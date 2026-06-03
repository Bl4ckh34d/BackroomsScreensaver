// BackroomsMazeGame.exe message pump and fixed-tick host loop.
// Included from game_app.inl after game shell helpers are available.

int RunGameMessageLoop(App& app, HWND hwnd) {
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

        bool inputWindowActive = GameWindowAcceptsInput(hwnd);
        int pauseVk = GameActionKey(app.gameInputSettings, GameInputAction::Pause);
        bool escapeDown = inputWindowActive && ((GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0);
        bool pauseDown = inputWindowActive && ((GetAsyncKeyState(pauseVk) & 0x8000) != 0);
        if (!escapeDown) app.gameSettingsEscapeConsumed = false;
        if (pauseDown && !escapeWasDown) {
            if (app.gameState == GameState::PlayGame || app.gameState == GameState::DebugScene) {
                EnterGameMainMenu(hwnd);
            } else if (app.gameState == GameState::Settings && app.gameConfig) {
                if (!app.gameSettingsKeyCaptureActive && !app.gameSettingsEscapeConsumed) {
                    DestroyWindow(app.gameConfig);
                }
            } else if (app.gameState == GameState::MainMenu && app.gameCustomMenuOpen) {
                ExitGameCustomMenu(hwnd);
            } else if (app.gameState == GameState::MainMenu && app.gameRunStarted && !app.gameDebugActive) {
                ActivateGameMenuCommand(hwnd, kGameResumeCurrentRunId);
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
