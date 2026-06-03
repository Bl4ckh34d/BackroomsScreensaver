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

void EnterGamePlay(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    if (!EnsureGameRenderer(hwnd, RendererRuntimeMode::PlayableGame)) return;
    bool loadSaved = gApp->gameLoadSavedRunPending;
    bool forceNew = gApp->gameForceNewRunPending;
    bool customGame = gApp->gameCustomGamePending;
    gApp->gameLoadSavedRunPending = false;
    gApp->gameForceNewRunPending = false;
    gApp->gameCustomGamePending = false;
    if (forceNew) {
        gApp->gameRunStarted = false;
        gApp->gameDebugActive = false;
        gApp->renderer.DeleteSavedGameRun();
    }
    if (loadSaved) {
        bool loaded = gApp->renderer.LoadSavedGameRun();
        if (!loaded) {
            gApp->renderer.RestartGameRun();
        }
        gApp->gameRunStarted = true;
    } else if (!gApp->gameRunStarted || gApp->gameDebugActive) {
        bool skipLoadingOverlay = gApp->gameSkipNextLoadingOverlay;
        gApp->gameSkipNextLoadingOverlay = false;
        if (!skipLoadingOverlay && !gApp->loadingOverlay) {
            gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance);
        }
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Loading level", L"Generating maze and scene geometry.", false);
            UpdateWindow(gApp->loadingOverlay);
        }
        if (customGame) {
            gApp->renderer.RestartCustomGameRun(gApp->gameCustomSpec);
        } else {
            gApp->renderer.RestartGameRun();
        }
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Ready", L"Entering maze.", true);
            CloseLoadingOverlayWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        gApp->gameRunStarted = true;
    } else {
        if (!gApp->renderer.RestorePausedGameRun()) {
            gApp->renderer.SetRuntimeMode(RendererRuntimeMode::PlayableGame);
        }
    }
    gApp->gameState = GameState::PlayGame;
    gApp->gameDebugActive = false;
    gApp->gameWindowActive = !IsIconic(hwnd);
    gApp->gameMouseDeltaX = 0.0f;
    gApp->gameMouseDeltaY = 0.0f;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    SetActiveWindow(hwnd);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    CaptureGameMouse(hwnd);
    SetWindowTextW(hwnd, L"Backrooms Maze - Single Player");
}
