// BackroomsMazeGame.exe target-specific window message helpers.
// Included from app_runtime.inl before the shared window procedure.

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

void ResizeGameConfigPanel(int width, int height) {
    if (gApp && gApp->gameConfig) {
        MoveWindow(gApp->gameConfig, 0, 0, std::max(1, width), std::max(1, height), TRUE);
    }
}

bool HandleGameShellResize(HWND hwnd, WPARAM wParam) {
    if (!gApp || !gApp->gameShell) return false;
    if (wParam == SIZE_MINIMIZED) {
        gApp->gameWindowActive = false;
        gApp->gameMouseDeltaX = 0.0f;
        gApp->gameMouseDeltaY = 0.0f;
        gApp->renderer.SetGameInput(GameInputSnapshot{});
        ReleaseGameMouse();
        if (gApp->gameState == GameState::PlayGame || gApp->gameState == GameState::DebugScene) {
            EnterGameMainMenu(hwnd);
        }
        return true;
    }
    gApp->gameWindowActive = true;
    LayoutGameControls(hwnd);
    LayoutCustomGameControls(hwnd);
    if (gApp->gameMouseCaptured) CaptureGameMouse(hwnd);
    if (gApp->gameState == GameState::MainMenu) InvalidateRect(hwnd, nullptr, TRUE);
    return false;
}

bool HandleGameEraseBackground(HWND hwnd) {
    return gApp && gApp->gameShell && gApp->gameState == GameState::MainMenu && hwnd == gApp->hwnd;
}

bool HandleGamePaint(HWND hwnd) {
    if (!gApp || !gApp->gameShell || gApp->gameState != GameState::MainMenu || hwnd != gApp->hwnd) return false;
    PAINTSTRUCT ps{};
    HDC dc = BeginPaint(hwnd, &ps);
    PaintGameMainMenu(hwnd, dc);
    EndPaint(hwnd, &ps);
    return true;
}

bool HandleGameConfigClosedMessage(HWND hwnd) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return false;
    if (gApp->gameSettingsReturnState == GameState::DebugScene) {
        EnterGameDebug(hwnd);
    } else {
        EnterGameMainMenu(hwnd);
    }
    return true;
}

bool HandleGameSetCursor() {
    if (!gApp || !gApp->gameShell) return false;
    if (gApp->gameState == GameState::PlayGame) {
        SetCursor(nullptr);
        return true;
    }
    SetCursor(LoadCursorW(nullptr, IDC_ARROW));
    return true;
}

bool HandleGameMouseMove(HWND hwnd, LPARAM lParam) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return false;
    POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    if (gApp->gameState == GameState::PlayGame) {
        if (gApp->gameRecenteringMouse) {
            gApp->gameRecenteringMouse = false;
            return true;
        }
        gApp->gameMouseDeltaX += static_cast<float>(p.x - gApp->gameMouseCenter.x);
        gApp->gameMouseDeltaY += static_cast<float>(p.y - gApp->gameMouseCenter.y);
        POINT center = gApp->gameMouseCenter;
        ClientToScreen(hwnd, &center);
        gApp->gameRecenteringMouse = true;
        SetCursorPos(center.x, center.y);
        return true;
    }
    if (gApp->gameState != GameState::MainMenu) return false;
    if (!gApp->gameMenuTrackingMouse) {
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hwnd;
        if (TrackMouseEvent(&tme)) gApp->gameMenuTrackingMouse = true;
    }
    int hover = HitTestGameMenu(hwnd, p);
    gApp->gameMenuMouse = p;
    gApp->gameMenuHasMouse = true;
    if (hover == kGameSinglePlayerId && gApp->gameMenuBloodStart == 0) {
        gApp->gameMenuBloodStart = GetTickCount64();
        gApp->gameMenuLampBurstStart = gApp->gameMenuBloodStart;
        if (gApp->rendererInitialized) gApp->renderer.TriggerMainMenuLampBurst();
    }
    if (hover != gApp->gameMenuHoverId) {
        gApp->gameMenuHoverId = hover;
        InvalidateRect(hwnd, nullptr, FALSE);
    }
    InvalidateRect(hwnd, nullptr, FALSE);
    return true;
}

bool HandleGameMouseLeave(HWND hwnd) {
    if (!gApp || !gApp->gameShell || gApp->gameState != GameState::MainMenu || hwnd != gApp->hwnd) return false;
    gApp->gameMenuTrackingMouse = false;
    gApp->gameMenuHasMouse = false;
    gApp->gameMenuHoverId = 0;
    if (GameMenuUsesRendererScene()) {
        gApp->renderer.SetMenuInteraction(0.5f, 0.5f, false, false, false);
        gApp->renderer.SetMenuHoverButtonIndex(-1);
    }
    InvalidateRect(hwnd, nullptr, FALSE);
    return true;
}

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

void HandleGameWindowDeactivation() {
    if (!gApp || !gApp->gameShell) return;
    gApp->gameWindowActive = false;
    gApp->gameMouseDeltaX = 0.0f;
    gApp->gameMouseDeltaY = 0.0f;
    gApp->renderer.SetGameInput(GameInputSnapshot{});
    ReleaseGameMouse();
    if (gApp->gameState == GameState::PlayGame || gApp->gameState == GameState::DebugScene) {
        EnterGameMainMenu(gApp->hwnd);
    }
}

bool HandleGameActivateApp(WPARAM wParam) {
    if (!gApp || !gApp->gameShell) return false;
    bool active = wParam != FALSE;
    gApp->gameWindowActive = active;
    if (!active) {
        HandleGameWindowDeactivation();
    } else if (!IsIconic(gApp->hwnd)) {
        if (gApp->gameState == GameState::PlayGame) CaptureGameMouse(gApp->hwnd);
        else if (gApp->gameState == GameState::MainMenu) SetGameCursorVisible(true);
    }
    return true;
}
