void SetGameMenuVisible(bool visible) {
    if (!gApp || !gApp->gameShell) return;
    HWND controls[] = {
        gApp->gameTitle,
        gApp->gameSinglePlayer,
        gApp->gameSettings,
        gApp->gameDebug,
        gApp->gameExit
    };
    for (HWND control : controls) {
        if (control) ShowWindow(control, SW_HIDE);
    }
    if (gApp->hwnd) InvalidateRect(gApp->hwnd, nullptr, TRUE);
}
