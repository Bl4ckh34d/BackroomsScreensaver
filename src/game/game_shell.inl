// Game shell, menu state transitions, mouse capture, and input collection.
// Included from main.cpp after App, Renderer, and ConfigDialogMode are declared.

void EnterGamePlay(HWND hwnd);
void EnterGameDebug(HWND hwnd);
void EnterGameSettings(HWND hwnd, ConfigDialogMode mode, GameState returnState);
void ReleaseGameMouse();

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

void UpdateGameMenuLabels() {
    if (!gApp || !gApp->gameShell) return;
    bool canResume = gApp->gameRunStarted && !gApp->gameDebugActive;
    if (gApp->gameSinglePlayer) {
        SetWindowTextW(gApp->gameSinglePlayer, canResume ? L"Resume" : L"Single Player");
    }
}

struct GameMenuButtonSpec {
    int id;
    const wchar_t* label;
};

std::array<GameMenuButtonSpec, 3> ActiveGameMenuButtons() {
    bool canResume = gApp && gApp->gameRunStarted && !gApp->gameDebugActive;
    return {{
        {kGameSinglePlayerId, canResume ? L"Resume" : L"Single Player"},
        {kGameSettingsId, L"Settings"},
        {kGameDebugId, L"Debug"}
    }};
}

RECT GameMenuButtonRect(const RECT& client, int index) {
    int w = std::max<LONG>(1, client.right - client.left);
    int h = std::max<LONG>(1, client.bottom - client.top);
    bool rendererScene = gApp && gApp->rendererInitialized && gApp->gameState == GameState::MainMenu &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu;
    if (rendererScene) {
        RECT projected{};
        if (gApp->renderer.MenuButtonScreenRect(index, projected)) return projected;
    }
    int buttonW = std::clamp(w * 34 / 100, 260, 420);
    int buttonH = 48;
    int gap = 13;
    int left = std::clamp(w / 2 - buttonW / 2, 34, std::max(34, w - buttonW - 34));
    int top = std::max(178, h / 2 - 82);
    top = std::min(top, std::max(118, h - 78 - (buttonH + gap) * 4));
    return {left, top + index * (buttonH + gap), left + buttonW, top + index * (buttonH + gap) + buttonH};
}

RECT GameMenuExitDoorRect(const RECT& client) {
    if (gApp && gApp->rendererInitialized && gApp->gameState == GameState::MainMenu &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu) {
        RECT projected{};
        if (gApp->renderer.MenuExitDoorScreenRect(projected)) return projected;
    }
    int h = std::max<LONG>(1, client.bottom - client.top);
    return {client.right - 118, h / 2 - 92, client.right - 56, h / 2 + 118};
}

int HitTestGameMenu(HWND hwnd, POINT p) {
    if (!gApp || !gApp->gameShell || gApp->gameState != GameState::MainMenu) return 0;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    auto buttons = ActiveGameMenuButtons();
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        RECT br = GameMenuButtonRect(rc, i);
        if (p.x >= br.left && p.x <= br.right && p.y >= br.top && p.y <= br.bottom) {
            return buttons[static_cast<size_t>(i)].id;
        }
    }
    RECT door = GameMenuExitDoorRect(rc);
    if (p.x >= door.left && p.x <= door.right && p.y >= door.top && p.y <= door.bottom) {
        return kGameExitId;
    }
    return 0;
}

void FillGameMenuRect(HDC dc, const RECT& rc, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rc, brush);
    DeleteObject(brush);
}

bool GameMenuUsesRendererScene() {
    return gApp && gApp->gameShell && gApp->rendererInitialized &&
        gApp->gameState == GameState::MainMenu &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu;
}

int GameMenuHoverButtonIndex(int hoverId) {
    const auto buttons = ActiveGameMenuButtons();
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        if (hoverId == buttons[static_cast<size_t>(i)].id) return i;
    }
    return -1;
}

bool GameMenuHoverIsButton(int hoverId) {
    return GameMenuHoverButtonIndex(hoverId) >= 0;
}

void PushGameMenuInteractionToRenderer(HWND hwnd) {
    if (!GameMenuUsesRendererScene() || !hwnd) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int w = std::max<LONG>(1, rc.right - rc.left);
    int h = std::max<LONG>(1, rc.bottom - rc.top);
    float x = gApp->gameMenuHasMouse ? static_cast<float>(gApp->gameMenuMouse.x) / static_cast<float>(w) : 0.5f;
    float y = gApp->gameMenuHasMouse ? static_cast<float>(gApp->gameMenuMouse.y) / static_cast<float>(h) : 0.5f;
    int hoverId = gApp->gameMenuHasMouse ? gApp->gameMenuHoverId : 0;
    int hoverIndex = GameMenuHoverButtonIndex(hoverId);
    gApp->renderer.SetMenuInteraction(x, y,
        hoverIndex >= 0,
        hoverId == kGameExitId,
        hoverIndex == 0);
    gApp->renderer.SetMenuHoverButtonIndex(hoverIndex);
    gApp->renderer.SetMenuResumeLabel(gApp->gameRunStarted && !gApp->gameDebugActive);
}

float GameMenuFadeAmount(ULONGLONG now) {
    if (!gApp) return 0.0f;
    constexpr float kFadeInMs = 1350.0f;
    constexpr float kFadeOutMs = 950.0f;
    if (gApp->gameMenuFadeOut) {
        return Clamp01(static_cast<float>(now - gApp->gameMenuFadeStart) / kFadeOutMs);
    }
    if (gApp->gameMenuFadeIn) {
        return 1.0f - Clamp01(static_cast<float>(now - gApp->gameMenuFadeStart) / kFadeInMs);
    }
    return 0.0f;
}

void DrawGameMenuFade(HDC dc, const RECT& rc, float amount) {
    amount = Clamp01(amount);
    if (amount <= 0.001f) return;
    int w = std::max<LONG>(1, rc.right - rc.left);
    int h = std::max<LONG>(1, rc.bottom - rc.top);
    int alpha = std::clamp(static_cast<int>(std::round(amount * 255.0f)), 1, 255);
    HDC memDc = CreateCompatibleDC(dc);
    if (!memDc) {
        FillGameMenuRect(dc, rc, RGB(0, 0, 0));
        return;
    }
    HBITMAP bmp = CreateCompatibleBitmap(dc, w, h);
    if (!bmp) {
        DeleteDC(memDc);
        FillGameMenuRect(dc, rc, RGB(0, 0, 0));
        return;
    }
    HGDIOBJ oldBmp = SelectObject(memDc, bmp);
    RECT local{0, 0, w, h};
    FillGameMenuRect(memDc, local, RGB(0, 0, 0));
    BLENDFUNCTION blend{AC_SRC_OVER, 0, static_cast<BYTE>(alpha), 0};
    AlphaBlend(dc, rc.left, rc.top, w, h, memDc, 0, 0, w, h, blend);
    SelectObject(memDc, oldBmp);
    DeleteObject(bmp);
    DeleteDC(memDc);
}

void PaintGameMainMenu(HWND hwnd, HDC dc) {
    if (!gApp || !gApp->gameShell) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int w = std::max<LONG>(1, rc.right - rc.left);
    int h = std::max<LONG>(1, rc.bottom - rc.top);
    ULONGLONG now = GetTickCount64();
    bool rendererScene = GameMenuUsesRendererScene();
    if (!rendererScene) {
        FillGameMenuRect(dc, rc, RGB(0, 0, 0));
        return;
    }
    (void)w;
    (void)h;
    DrawGameMenuFade(dc, rc, GameMenuFadeAmount(now));
}

void ActivateGameMenuCommand(HWND hwnd, int id) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return;
    if (gApp->gameMenuFadeOut || gApp->gameMenuStartCinematic) return;
    gApp->gameMenuPendingCommand = id;
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
        EnterGamePlay(hwnd);
    } else if (id == kGameSettingsId) {
        EnterGameSettings(hwnd, ConfigDialogMode::Game, GameState::MainMenu);
    } else if (id == kGameDebugId) {
        EnterGameDebug(hwnd);
    } else if (id == kGameExitId) {
        ReleaseGameMouse();
        DestroyWindow(hwnd);
    }
}

void UpdateGameMenuTransition(HWND hwnd) {
    if (!gApp || !gApp->gameShell || gApp->gameState != GameState::MainMenu) return;
    ULONGLONG now = GetTickCount64();
    if (gApp->gameMenuStartCinematic) {
        if (gApp->rendererInitialized && gApp->renderer.MainMenuStartTransitionComplete()) {
            int pending = gApp->gameMenuPendingCommand;
            gApp->gameMenuStartCinematic = false;
            gApp->gameSkipNextLoadingOverlay = pending == kGameSinglePlayerId;
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
    if (gApp->rendererInitialized) {
        if (mode == RendererRuntimeMode::MainMenu) {
            gApp->renderer.EnterMainMenuScene();
        } else {
            gApp->renderer.SetRuntimeMode(mode);
        }
        return true;
    }
    gApp->renderer.SetRuntimeMode(mode);
    if (!gApp->loadingOverlay) {
        gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance, mode == RendererRuntimeMode::MainMenu);
    }
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay,
            mode == RendererRuntimeMode::MainMenu ? L"NeuralForge Solutions" : L"Loading level",
            mode == RendererRuntimeMode::MainMenu ? L"Prewarming renderer, shaders, textures, and menu scene." : L"Preparing renderer and maze.",
            false);
        UpdateWindow(gApp->loadingOverlay);
    }
    if (mode == RendererRuntimeMode::MainMenu) {
        gApp->renderer.PrepareAudio(gApp->gameInputSettings);
        WaitForLoadingOverlayIntro(gApp->loadingOverlay);
    }
    StartupProgressSink loadingProgress{LoadingProgressCallback, gApp->loadingOverlay};
    int oldThreadPriority = THREAD_PRIORITY_ERROR_RETURN;
    bool loweredStartupPriority = LoadingOverlayHasIndependentSplash(gApp->loadingOverlay);
    if (loweredStartupPriority) {
        oldThreadPriority = GetThreadPriority(GetCurrentThread());
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    }
    bool initialized = gApp->renderer.Initialize(hwnd, nullptr, false, MonsterPreviewView::Orbit,
        gApp->loadingOverlay ? &loadingProgress : nullptr);
    if (oldThreadPriority != THREAD_PRIORITY_ERROR_RETURN) {
        SetThreadPriority(GetCurrentThread(), oldThreadPriority);
    }
    if (!initialized) {
        if (gApp->loadingOverlay) {
            CloseLoadingOverlayWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        MessageBoxW(hwnd, L"Direct3D initialization failed.", L"Backrooms Maze Game", MB_OK | MB_ICONERROR);
        return false;
    }
    gApp->rendererInitialized = true;
    if (mode == RendererRuntimeMode::MainMenu) gApp->renderer.EnterMainMenuScene();
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay,
            mode == RendererRuntimeMode::MainMenu ? L"Ready" : L"Ready",
            mode == RendererRuntimeMode::MainMenu ? L"Entering main menu." : L"Entering maze.",
            true);
        FinishLoadingOverlay(gApp->loadingOverlay);
        gApp->loadingOverlay = nullptr;
    }
    return true;
}

void EnterGameMainMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    bool pausePlayableRun = gApp->gameState == GameState::PlayGame &&
        gApp->gameRunStarted && !gApp->gameDebugActive;
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
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    if (gApp->rendererInitialized) {
        if (pausePlayableRun) {
            gApp->renderer.EnterPausedMainMenuScene();
        } else {
            gApp->renderer.EnterMainMenuScene();
        }
    }
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
        bool skipLoadingOverlay = gApp->gameSkipNextLoadingOverlay;
        gApp->gameSkipNextLoadingOverlay = false;
        if (!skipLoadingOverlay && !gApp->loadingOverlay) {
            gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance);
        }
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Loading level", L"Generating maze and scene geometry.", false);
            UpdateWindow(gApp->loadingOverlay);
        }
        gApp->renderer.RestartGameRun();
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
    input.sprint = down(GameActionKey(settings, GameInputAction::Sprint));
    input.crouch = down(GameActionKey(settings, GameInputAction::Crouch));
    input.interact = down(GameActionKey(settings, GameInputAction::Interact));
    input.flashlight = down(GameActionKey(settings, GameInputAction::Flashlight));
    input.pause = down(GameActionKey(settings, GameInputAction::Pause));
    return input;
}
