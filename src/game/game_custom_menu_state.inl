// Custom-game menu state.

void EnterGameCustomMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell || !hwnd) return;
    ReleaseGameMouse();
    gApp->gameCustomMenuOpen = true;
    gApp->gameCustomSelectedScare = -1;
    gApp->gameCustomMenuOpenStart = GetTickCount64();
    gApp->gameMenuHoverId = 0;
    gApp->gameMenuHasMouse = false;
    if (gApp->rendererInitialized) {
        gApp->renderer.SetMainMenuCustomGameView(true);
        gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, 0, gApp->gameCustomSelectedScare);
    }
    SetCustomGameControlsVisible(false);
    SetWindowTextW(hwnd, L"Backrooms Maze - Custom Game");
    InvalidateRect(hwnd, nullptr, FALSE);
}

void UpdateCustomGameControls(HWND hwnd) {
    if (!gApp || !gApp->gameCustomMenuOpen) return;
    SetCustomGameControlsVisible(false);
    if (gApp->rendererInitialized) {
        int hover = gApp->gameMenuHasMouse ? HitTestCustomGameMenu(hwnd, gApp->gameMenuMouse) : 0;
        gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, hover, gApp->gameCustomSelectedScare);
    }
}

void ExitGameCustomMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    if (!gApp->gameCustomMenuOpen) return;
    gApp->gameCustomSpec = ReadCustomGameSpecFromControls();
    gApp->gameCustomMenuOpen = false;
    gApp->gameCustomSelectedScare = -1;
    SetCustomGameControlsVisible(false);
    if (gApp->rendererInitialized) {
        gApp->renderer.SetMainMenuCustomGameView(false);
    }
    SetWindowTextW(hwnd, L"Backrooms Maze");
    InvalidateRect(hwnd, nullptr, FALSE);
}

void StartCustomGameFromMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    if (gApp->gameMenuFadeOut || gApp->gameMenuStartCinematic) return;
    gApp->gameCustomSpec = ReadCustomGameSpecFromControls();
    gApp->gameCustomMenuOpen = false;
    SetCustomGameControlsVisible(false);
    gApp->gameMenuPendingCommand = kGameCustomStartId;
    gApp->gameMenuStartCinematic = true;
    gApp->gameMenuFadeIn = false;
    gApp->gameMenuFadeOut = false;
    gApp->gameMenuFadeStart = GetTickCount64();
    gApp->gameCustomGamePending = true;
    gApp->gameForceNewRunPending = true;
    gApp->gameLoadSavedRunPending = false;
    if (gApp->rendererInitialized) {
        gApp->renderer.BeginMainMenuStartTransition();
    } else {
        ExecuteGameMenuCommand(hwnd, kGameCustomStartId);
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}
