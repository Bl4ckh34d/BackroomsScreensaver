// Main-menu commands.

void ActivateGameMenuCommand(HWND hwnd, int id) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return;
    if (gApp->gameMenuFadeOut || gApp->gameMenuStartCinematic) return;
    gApp->gameMenuPendingCommand = id;
    if (id == kGameCustomGameId) {
        gApp->gameMenuPendingCommand = 0;
        ExecuteGameMenuCommand(hwnd, id);
        return;
    }
    if (id == kGameSinglePlayerId && (!gApp->gameRunStarted || gApp->gameDebugActive)) {
        gApp->gameMenuStartCinematic = true;
        gApp->gameMenuFadeIn = false;
        gApp->gameMenuFadeOut = false;
        gApp->gameMenuFadeStart = GetTickCount64();
        if (gApp->rendererInitialized) {
            gApp->renderer.BeginMainMenuStartTransition();
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
    gApp->gameMenuFadeOut = true;
    gApp->gameMenuFadeIn = false;
    gApp->gameMenuFadeStart = GetTickCount64();
    if (id == kGameSinglePlayerId) {
        if (gApp->gameMenuLampBurstStart == 0) {
            gApp->gameMenuLampBurstStart = gApp->gameMenuFadeStart;
            if (gApp->rendererInitialized) gApp->renderer.TriggerMainMenuLampBurst();
        }
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}
