// Game shell, menu state transitions, mouse capture, and input collection.
// Included from main.cpp after App, Renderer, and ConfigDialogMode are declared.

void LayoutGameControls(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int w = std::max(1L, rc.right - rc.left);
    int h = std::max(1L, rc.bottom - rc.top);
    int centerX = w / 2;
    int buttonW = 220;
    int buttonH = 38;
    int gap = 12;
    int top = std::max(90, h / 2 - 120);
    if (gApp->gameTitle) MoveWindow(gApp->gameTitle, centerX - 240, top - 76, 480, 42, TRUE);
    if (gApp->gameSinglePlayer) MoveWindow(gApp->gameSinglePlayer, centerX - buttonW / 2, top, buttonW, buttonH, TRUE);
    if (gApp->gameSettings) MoveWindow(gApp->gameSettings, centerX - buttonW / 2, top + (buttonH + gap), buttonW, buttonH, TRUE);
    if (gApp->gameDebug) MoveWindow(gApp->gameDebug, centerX - buttonW / 2, top + (buttonH + gap) * 2, buttonW, buttonH, TRUE);
    if (gApp->gameExit) MoveWindow(gApp->gameExit, centerX - buttonW / 2, top + (buttonH + gap) * 3, buttonW, buttonH, TRUE);
    if (gApp->gameBack) MoveWindow(gApp->gameBack, 12, 10, 104, 28, TRUE);
}

void SetGameMenuVisible(bool visible) {
    if (!gApp || !gApp->gameShell) return;
    int show = visible ? SW_SHOW : SW_HIDE;
    HWND controls[] = {
        gApp->gameTitle,
        gApp->gameSinglePlayer,
        gApp->gameSettings,
        gApp->gameDebug,
        gApp->gameExit
    };
    for (HWND control : controls) {
        if (control) ShowWindow(control, show);
    }
}

void UpdateGameMenuLabels() {
    if (!gApp || !gApp->gameShell) return;
    bool canResume = gApp->gameRunStarted && !gApp->gameDebugActive;
    if (gApp->gameSinglePlayer) {
        SetWindowTextW(gApp->gameSinglePlayer, canResume ? L"Resume" : L"Single Player");
    }
}

void SetDebugControlsVisible(bool visible) {
    if (!gApp) return;
    int show = visible ? SW_SHOW : SW_HIDE;
    HWND controls[] = {
        gApp->debugPrevEffect,
        gApp->debugNextEffect,
        gApp->debugSize,
        gApp->debugReset,
        gApp->debugPrevProp,
        gApp->debugNextProp
    };
    for (HWND control : controls) {
        if (control) ShowWindow(control, show);
    }
}

void SetGameCursorVisible(bool visible) {
    if (visible) {
        while (ShowCursor(TRUE) < 0) {}
    } else {
        while (ShowCursor(FALSE) >= 0) {}
    }
}

void ReleaseGameMouse() {
    if (!gApp) return;
    ClipCursor(nullptr);
    if (gApp->gameMouseCaptured) {
        ReleaseCapture();
    }
    SetGameCursorVisible(true);
    gApp->gameMouseCaptured = false;
}

void CaptureGameMouse(HWND hwnd) {
    if (!gApp || !gApp->gameShell || !hwnd) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    POINT tl{rc.left, rc.top};
    POINT br{rc.right, rc.bottom};
    ClientToScreen(hwnd, &tl);
    ClientToScreen(hwnd, &br);
    RECT clip{tl.x, tl.y, br.x, br.y};
    ClipCursor(&clip);
    if (!gApp->gameMouseCaptured) {
        SetCapture(hwnd);
        SetGameCursorVisible(false);
    }
    gApp->gameMouseCaptured = true;
    gApp->gameMouseCenter = {(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2};
    POINT center = gApp->gameMouseCenter;
    ClientToScreen(hwnd, &center);
    gApp->gameRecenteringMouse = true;
    SetCursorPos(center.x, center.y);
}

bool EnsureGameRenderer(HWND hwnd, RendererRuntimeMode mode) {
    if (!gApp || !gApp->gameShell) return false;
    if (gApp->rendererInitialized) return true;
    gApp->renderer.SetRuntimeMode(mode);
    if (!gApp->loadingOverlay) {
        gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance);
    }
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay, L"Loading level", L"Preparing renderer and maze.", false);
        UpdateWindow(gApp->loadingOverlay);
    }
    StartupProgressSink loadingProgress{LoadingProgressCallback, gApp->loadingOverlay};
    if (!gApp->renderer.Initialize(hwnd, nullptr, false, MonsterPreviewView::Orbit,
            gApp->loadingOverlay ? &loadingProgress : nullptr)) {
        if (gApp->loadingOverlay) {
            DestroyWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        MessageBoxW(hwnd, L"Direct3D initialization failed.", L"Backrooms Maze Game", MB_OK | MB_ICONERROR);
        return false;
    }
    gApp->rendererInitialized = true;
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay, L"Ready", L"Entering maze.", true);
        DestroyWindow(gApp->loadingOverlay);
        gApp->loadingOverlay = nullptr;
    }
    return true;
}

void EnterGameMainMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    ReleaseGameMouse();
    gApp->gameState = GameState::MainMenu;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    SetGameMenuVisible(true);
    UpdateGameMenuLabels();
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    SetWindowTextW(hwnd, L"Backrooms Maze");
}

void EnterGamePlay(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    if (!EnsureGameRenderer(hwnd, RendererRuntimeMode::PlayableGame)) return;
    if (!gApp->gameRunStarted || gApp->gameDebugActive) {
        if (!gApp->loadingOverlay) {
            gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance);
        }
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Loading level", L"Generating maze and scene geometry.", false);
            UpdateWindow(gApp->loadingOverlay);
        }
        gApp->renderer.RestartGameRun();
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Ready", L"Entering maze.", true);
            DestroyWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        gApp->gameRunStarted = true;
    } else {
        gApp->renderer.SetRuntimeMode(RendererRuntimeMode::PlayableGame);
    }
    gApp->gameState = GameState::PlayGame;
    gApp->gameDebugActive = false;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    CaptureGameMouse(hwnd);
    SetWindowTextW(hwnd, L"Backrooms Maze - Single Player");
}

void EnterGameDebug(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    gEffectDebugViewer = true;
    gDebugSliceEffect = DebugSliceEffect::Blood;
    gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    gBloodDebugEveryWall = true;
    if (!EnsureGameRenderer(hwnd, RendererRuntimeMode::DebugViewer)) return;
    gApp->renderer.EnterDebugViewer(gDebugSliceEffect, gDebugSliceTiles);
    gApp->gameState = GameState::DebugScene;
    gApp->gameDebugActive = true;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(true);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_SHOW);
    ReleaseGameMouse();
    UpdateDebugSliceControls(hwnd);
    RedrawDebugSliceControls();
}

void EnterGameSettings(HWND hwnd, ConfigDialogMode mode, GameState returnState) {
    if (!gApp || !gApp->gameShell) return;
    ReleaseGameMouse();
    if (gApp->gameConfig) {
        DestroyWindow(gApp->gameConfig);
        gApp->gameConfig = nullptr;
    }
    gApp->gameSettingsReturnState = returnState;
    gApp->gameState = GameState::Settings;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    gApp->gameConfig = mode == ConfigDialogMode::Game
        ? CreateGameSettingsPanel(hwnd)
        : CreateEmbeddedConfig(hwnd, mode);
    if (gApp->gameConfig) {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        MoveWindow(gApp->gameConfig, 0, 0, std::max(1L, rc.right - rc.left), std::max(1L, rc.bottom - rc.top), TRUE);
        ShowWindow(gApp->gameConfig, SW_SHOW);
        SetFocus(gApp->gameConfig);
        SetWindowTextW(hwnd, mode == ConfigDialogMode::Debug ? L"Backrooms Maze - Debug Settings" : L"Backrooms Maze - Settings");
    } else if (returnState == GameState::DebugScene) {
        EnterGameDebug(hwnd);
    } else {
        EnterGameMainMenu(hwnd);
    }
}

GameInputSnapshot CollectGameInput() {
    GameInputSnapshot input{};
    if (!gApp || gApp->gameState != GameState::PlayGame) return input;
    auto down = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
    const Settings& settings = gApp->gameInputSettings;
    input.moveX =
        (down(GameActionKey(settings, GameInputAction::MoveRight)) ? 1.0f : 0.0f) +
        (down(GameActionKey(settings, GameInputAction::MoveLeft)) ? -1.0f : 0.0f);
    input.moveZ =
        (down(GameActionKey(settings, GameInputAction::MoveForward)) ? 1.0f : 0.0f) +
        (down(GameActionKey(settings, GameInputAction::MoveBackward)) ? -1.0f : 0.0f);
    input.lookDeltaX = gApp->gameMouseDeltaX;
    input.lookDeltaY = gApp->gameMouseDeltaY;
    gApp->gameMouseDeltaX = 0.0f;
    gApp->gameMouseDeltaY = 0.0f;
    input.jump = down(GameActionKey(settings, GameInputAction::Jump));
    input.sprint = down(GameActionKey(settings, GameInputAction::Sprint));
    input.crouch = down(GameActionKey(settings, GameInputAction::Crouch));
    input.interact = down(GameActionKey(settings, GameInputAction::Interact));
    input.pause = down(GameActionKey(settings, GameInputAction::Pause));
    return input;
}
