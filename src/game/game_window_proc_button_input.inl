bool HandleGameButtonOrKeyDown(HWND hwnd, UINT msg, LPARAM lParam) {
    if (!gApp || !gApp->gameShell) return false;
    if (msg == WM_LBUTTONDOWN && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd) {
        POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
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
