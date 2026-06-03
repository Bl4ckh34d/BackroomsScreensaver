void HandleGameWindowDeactivation() {
    if (!gApp || !gApp->gameShell) return;
    gApp->gameWindowActive = false;
    gApp->gameMouseDeltaX = 0.0f;
    gApp->gameMouseDeltaY = 0.0f;
    gApp->renderer.SetGameInput(GameInputSnapshot{});
    ReleaseGameMouse();
    if (gApp->gameState == GameState::PlayGame || gApp->gameState == GameState::DebugScene) {
        EnterGameMainMenu(gApp->hwnd);
    }
}

bool HandleGameActivateApp(WPARAM wParam) {
    if (!gApp || !gApp->gameShell) return false;
    bool active = wParam != FALSE;
    gApp->gameWindowActive = active;
    if (!active) {
        HandleGameWindowDeactivation();
    } else if (!IsIconic(gApp->hwnd)) {
        if (gApp->gameState == GameState::PlayGame) CaptureGameMouse(gApp->hwnd);
        else if (gApp->gameState == GameState::MainMenu) SetGameCursorVisible(true);
    }
    return true;
}
