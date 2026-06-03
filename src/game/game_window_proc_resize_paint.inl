void ResizeGameConfigPanel(int width, int height) {
    if (gApp && gApp->gameConfig) {
        MoveWindow(gApp->gameConfig, 0, 0, std::max(1, width), std::max(1, height), TRUE);
    }
}

bool HandleGameShellResize(HWND hwnd, WPARAM wParam) {
    if (!gApp || !gApp->gameShell) return false;
    if (wParam == SIZE_MINIMIZED) {
        gApp->gameWindowActive = false;
        gApp->gameMouseDeltaX = 0.0f;
        gApp->gameMouseDeltaY = 0.0f;
        gApp->renderer.SetGameInput(GameInputSnapshot{});
        ReleaseGameMouse();
        if (gApp->gameState == GameState::PlayGame || gApp->gameState == GameState::DebugScene) {
            EnterGameMainMenu(hwnd);
        }
        return true;
    }
    gApp->gameWindowActive = true;
    LayoutGameControls(hwnd);
    LayoutCustomGameControls(hwnd);
    if (gApp->gameMouseCaptured) CaptureGameMouse(hwnd);
    if (gApp->gameState == GameState::MainMenu) InvalidateRect(hwnd, nullptr, TRUE);
    return false;
}

bool HandleGameEraseBackground(HWND hwnd) {
    return gApp && gApp->gameShell && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd;
}

bool HandleGamePaint(HWND hwnd) {
    if (!gApp || !gApp->gameShell || gApp->gameState != GameState::MainMenu || hwnd != gApp->hwnd) return false;
    PAINTSTRUCT ps{};
    HDC dc = BeginPaint(hwnd, &ps);
    PaintGameMainMenu(hwnd, dc);
    EndPaint(hwnd, &ps);
    return true;
}

bool HandleGameConfigClosedMessage(HWND hwnd) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return false;
    if (gApp->gameSettingsReturnState == GameState::DebugScene) {
        EnterGameDebug(hwnd);
    } else {
        EnterGameMainMenu(hwnd);
    }
    return true;
}
