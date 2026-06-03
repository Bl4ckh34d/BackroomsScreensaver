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
