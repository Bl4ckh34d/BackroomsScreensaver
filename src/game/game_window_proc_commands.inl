bool HandleGameCommand(HWND hwnd, WPARAM wParam) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return false;
    int id = LOWORD(wParam);
    if (id == kGameSinglePlayerId || id == kGameSettingsId || id == kGameDebugId || id == kGameExitId) {
        ActivateGameMenuCommand(hwnd, id);
        return true;
    }
    if (id == kGameCustomStartId) {
        StartCustomGameFromMenu(hwnd);
        return true;
    }
    if (id == kGameCustomBackId) {
        ExitGameCustomMenu(hwnd);
        return true;
    }
    if (id == kGameBackId) {
        if (gApp->gameState == GameState::Settings && gApp->gameConfig) {
            DestroyWindow(gApp->gameConfig);
            return true;
        }
        EnterGameMainMenu(hwnd);
        return true;
    }
    if (id == kDebugSettingsId) {
        EnterGameSettings(hwnd, ConfigDialogMode::Debug, GameState::DebugScene);
        return true;
    }
    return false;
}
