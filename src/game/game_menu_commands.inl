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

void ExecuteGameMenuCommand(HWND hwnd, int id) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return;
    if (id == kGameSinglePlayerId) {
        gApp->gameForceNewRunPending = true;
        gApp->gameLoadSavedRunPending = false;
        gApp->gameCustomGamePending = false;
        EnterGamePlay(hwnd);
    } else if (id == kGameResumeCurrentRunId) {
        gApp->gameForceNewRunPending = false;
        gApp->gameLoadSavedRunPending = false;
        gApp->gameCustomGamePending = false;
        EnterGamePlay(hwnd);
    } else if (id == kGameResumeSavedRunId) {
        gApp->gameForceNewRunPending = false;
        gApp->gameLoadSavedRunPending = true;
        gApp->gameCustomGamePending = false;
        EnterGamePlay(hwnd);
    } else if (id == kGameCustomStartId) {
        gApp->gameForceNewRunPending = true;
        gApp->gameLoadSavedRunPending = false;
        gApp->gameCustomGamePending = true;
        EnterGamePlay(hwnd);
    } else if (id == kGameCustomGameId) {
        EnterGameCustomMenu(hwnd);
    } else if (id == kGameSettingsId) {
        EnterGameSettings(hwnd, ConfigDialogMode::Game, GameState::MainMenu);
    } else if (id == kGameDebugId) {
        EnterGameDebug(hwnd);
    } else if (id == kGameExitId) {
        ReleaseGameMouse();
        DestroyWindow(hwnd);
    }
}
