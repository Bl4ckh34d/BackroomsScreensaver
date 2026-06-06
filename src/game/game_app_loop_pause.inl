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
            } else if (app.gameState == GameState::MainMenu && app.gameSettingsBoardOpen) {
                ExitGameSettingsBoard(hwnd);
            } else if (app.gameState == GameState::MainMenu && app.gameCustomMenuOpen) {
                ExitGameCustomMenu(hwnd);
            } else if (app.gameState == GameState::MainMenu && app.gameRunStarted && !app.gameDebugActive) {
                ActivateGameMenuCommand(hwnd, kGameResumeCurrentRunId);
            }
        }
        escapeWasDown = pauseDown;
