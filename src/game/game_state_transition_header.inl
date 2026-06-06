// Game shell state transitions for main menu, play, and debug.

void EnterGameMainMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    bool pausePlayableRun = gApp->gameState == GameState::PlayGame &&
        gApp->gameRunStarted && !gApp->gameDebugActive &&
        (!gApp->rendererInitialized || !gApp->renderer.PlayableRunFinished());
    ReleaseGameMouse();
    gApp->gameState = GameState::MainMenu;
    gApp->gameMenuFadeIn = true;
    gApp->gameMenuFadeOut = false;
    gApp->gameMenuStartCinematic = false;
    gApp->gameSkipNextLoadingOverlay = false;
    gApp->gameMenuPendingCommand = 0;
    gApp->gameMenuFadeStart = GetTickCount64();
    gApp->gameMenuHoverId = 0;
    gApp->gameMenuHasMouse = false;
    gApp->gameMenuTrackingMouse = false;
    gApp->gameMenuBloodStart = 0;
    gApp->gameMenuLampBurstStart = 0;
    gApp->gameCustomMenuOpen = false;
    gApp->gameSettingsBoardOpen = false;
    SetCustomGameControlsVisible(false);
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    bool finishedPlayableRun = gApp->rendererInitialized && gApp->renderer.PlayableRunFinished();
    if (gApp->rendererInitialized) {
        if (pausePlayableRun) {
            gApp->renderer.EnterPausedMainMenuScene();
        } else {
            gApp->renderer.EnterMainMenuScene();
            if (finishedPlayableRun) {
                gApp->gameRunStarted = false;
            }
        }
    }
    SetGameMenuVisible(true);
    UpdateGameMenuLabels();
    SetGameCursorVisible(true);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    SetWindowTextW(hwnd, L"Backrooms Maze");
}
