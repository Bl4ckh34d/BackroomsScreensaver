// Game settings panel callbacks and embedded settings transition.

void OnGameSettingsPanelSaved(void*, const Settings& settings) {
    if (!gApp) return;
    gApp->gameInputSettings = settings;
    if (gApp->rendererInitialized) gApp->renderer.ApplyGameSettings(settings);
    ApplyGameWindowSettings(gApp->hwnd, settings);
}

void OnGameSettingsPanelKeyCaptureChanged(void*, bool active, bool escapeConsumed) {
    if (!gApp) return;
    gApp->gameSettingsKeyCaptureActive = active;
    gApp->gameSettingsEscapeConsumed = escapeConsumed;
}

void OnGameSettingsPanelClosed(void*, HWND hwnd) {
    if (!gApp) return;
    if (gApp->gameConfig == hwnd) {
        gApp->gameConfig = nullptr;
        PostMessageW(gApp->hwnd, kGameConfigClosedMessage, 0, 0);
    }
}

GameSettingsPanelHost BuildGameSettingsPanelHost(HWND hwnd) {
    return {
        hwnd,
        OnGameSettingsPanelSaved,
        OnGameSettingsPanelKeyCaptureChanged,
        OnGameSettingsPanelClosed
    };
}

void EnterGameDebug(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    gEffectDebugViewer = true;
    gDebugSliceEffect = DebugSliceEffect::Blood;
    gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    gBloodDebugEveryWall = true;
    if (!EnsureGameRenderer(hwnd, RendererRuntimeMode::DebugViewer)) return;
    gApp->renderer.EnterDebugViewer(gDebugSliceEffect, gDebugSliceTiles);
    gApp->gameState = GameState::DebugScene;
    gApp->gameDebugActive = true;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(true);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_SHOW);
    ReleaseGameMouse();
    UpdateDebugSliceControls(hwnd);
    RedrawDebugSliceControls();
}

void EnterGameSettings(HWND hwnd, ConfigDialogMode mode, GameState returnState) {
    if (!gApp || !gApp->gameShell) return;
    ReleaseGameMouse();
    if (gApp->gameConfig) {
        DestroyWindow(gApp->gameConfig);
        gApp->gameConfig = nullptr;
    }
    gApp->gameSettingsReturnState = returnState;
    gApp->gameState = GameState::Settings;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    gApp->gameConfig = mode == ConfigDialogMode::Game
        ? CreateGameSettingsPanel(hwnd, BuildGameSettingsPanelHost(hwnd))
        : CreateEmbeddedConfig(hwnd, mode);
    if (gApp->gameConfig) {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        MoveWindow(gApp->gameConfig, 0, 0, std::max(1L, rc.right - rc.left), std::max(1L, rc.bottom - rc.top), TRUE);
        ShowWindow(gApp->gameConfig, SW_SHOW);
        SetFocus(gApp->gameConfig);
        SetWindowTextW(hwnd, mode == ConfigDialogMode::Debug ? L"Backrooms Maze - Debug Settings" : L"Backrooms Maze - Settings");
    } else if (returnState == GameState::DebugScene) {
        EnterGameDebug(hwnd);
    } else {
        EnterGameMainMenu(hwnd);
    }
}
