bool HandleGameButtonOrKeyDown(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (!gApp || !gApp->gameShell) return false;
    if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) &&
        gApp->gameState == GameState::MainMenu &&
        gApp->gameSettingsBoardOpen &&
        gApp->gameSettingsBoardCaptureAction >= 0 &&
        hwnd == gApp->hwnd) {
        int actionIndex = std::clamp(gApp->gameSettingsBoardCaptureAction, 0, kGameInputActionCount - 1);
        if (wParam == VK_ESCAPE) {
            gApp->gameSettingsBoardCaptureAction = -1;
            gApp->gameSettingsKeyCaptureActive = false;
            gApp->gameSettingsEscapeConsumed = true;
        } else if (wParam > 0 && wParam < 256) {
            AssignGameActionKey(gApp->gameSettingsBoardSettings,
                static_cast<GameInputAction>(actionIndex),
                static_cast<int>(wParam));
            gApp->gameSettingsBoardCaptureAction = -1;
            gApp->gameSettingsKeyCaptureActive = false;
            gApp->gameSettingsEscapeConsumed = false;
        }
        PushSettingsBoardStateToRenderer(hwnd);
        InvalidateRect(hwnd, nullptr, FALSE);
        return true;
    }
    if (msg == WM_LBUTTONDOWN && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd) {
        POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        if (gApp->gameSettingsBoardOpen) {
            int control = HitTestSettingsBoard(hwnd, p);
            if (control != 0) ActivateSettingsBoardCommand(hwnd, control);
            return true;
        }
        if (gApp->gameCustomMenuOpen) {
            int control = HitTestCustomGameMenu(hwnd, p);
            if (control != 0) ActivateCustomGameMenuCommand(hwnd, control);
            return true;
        }
        int id = HitTestGameMenu(hwnd, p);
        if (id != 0) ActivateGameMenuCommand(hwnd, id);
    }
    return true;
}
