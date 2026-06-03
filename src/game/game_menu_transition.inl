// Main-menu transition.

void UpdateGameMenuTransition(HWND hwnd) {
    if (!gApp || !gApp->gameShell || gApp->gameState != GameState::MainMenu) return;
    ULONGLONG now = GetTickCount64();
    if (gApp->gameMenuStartCinematic) {
        if (gApp->rendererInitialized && gApp->renderer.MainMenuStartTransitionComplete()) {
            int pending = gApp->gameMenuPendingCommand;
            gApp->gameMenuStartCinematic = false;
            gApp->gameSkipNextLoadingOverlay = pending == kGameSinglePlayerId || pending == kGameCustomStartId;
            gApp->gameMenuPendingCommand = 0;
            ExecuteGameMenuCommand(hwnd, pending);
            return;
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
    if (gApp->gameMenuFadeIn && now - gApp->gameMenuFadeStart >= 1350) {
        gApp->gameMenuFadeIn = false;
    }
    if (gApp->gameMenuFadeOut && now - gApp->gameMenuFadeStart >= 950) {
        int pending = gApp->gameMenuPendingCommand;
        gApp->gameMenuFadeOut = false;
        gApp->gameMenuPendingCommand = 0;
        ExecuteGameMenuCommand(hwnd, pending);
        return;
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}
