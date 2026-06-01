// Game shell, menu state transitions, mouse capture, and input collection.
// Included from main.cpp after App, Renderer, and ConfigDialogMode are declared.

void EnterGamePlay(HWND hwnd);
void EnterGameDebug(HWND hwnd);
void EnterGameSettings(HWND hwnd, ConfigDialogMode mode, GameState returnState);
void EnterGameCustomMenu(HWND hwnd);
void ExecuteGameMenuCommand(HWND hwnd, int id);
void ReleaseGameMouse();
void ExitGameCustomMenu(HWND hwnd);

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

void SetCustomControlVisible(HWND control, bool visible) {
    if (control) ShowWindow(control, visible ? SW_SHOW : SW_HIDE);
}

void SetCustomGameControlsVisible(bool visible) {
    if (!gApp) return;
    HWND controls[] = {
        gApp->customTitle,
        gApp->customLayerLabel,
        gApp->customLayer,
        gApp->customScaresLabel,
        gApp->customScareBrokenLamp,
        gApp->customScareAirVent,
        gApp->customScareWater,
        gApp->customScareBlood,
        gApp->customScareFlesh,
        gApp->customBossesLabel,
        gApp->customBossOmukade,
        gApp->customSizeLabel,
        gApp->customSizeXLabel,
        gApp->customSizeX,
        gApp->customSizeYLabel,
        gApp->customSizeY,
        gApp->customPages,
        gApp->customStart,
        gApp->customBack
    };
    for (HWND control : controls) SetCustomControlVisible(control, visible);
}

void LayoutCustomGameControls(HWND hwnd) {
    if (!gApp || !gApp->gameShell || !gApp->gameCustomMenuOpen) return;
    RECT panel{};
    bool projected = gApp->rendererInitialized &&
        gApp->renderer.RuntimeMode() == RendererRuntimeMode::MainMenu &&
        gApp->renderer.CustomGamePanelScreenRect(panel);
    if (!projected) {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        int w = std::max<LONG>(1, rc.right - rc.left);
        int h = std::max<LONG>(1, rc.bottom - rc.top);
        int panelW = std::clamp(w * 38 / 100, 320, 470);
        int panelH = std::clamp(h * 70 / 100, 430, 640);
        panel = {w / 2 - panelW / 2, h / 2 - panelH / 2, w / 2 + panelW / 2, h / 2 + panelH / 2};
    }

    int left = panel.left + 22;
    int top = panel.top + 18;
    int width = std::max(220L, panel.right - panel.left - 44);
    int row = 25;
    int y = top;
    auto place = [&](HWND control, int x, int cy, int cw, int ch = 22) {
        if (control) MoveWindow(control, x, cy, cw, ch, TRUE);
    };

    place(gApp->customTitle, left, y, width, 26); y += 34;
    place(gApp->customLayerLabel, left, y + 3, 82, 20);
    place(gApp->customLayer, left + 92, y, std::min(180, width - 92), 120); y += row + 6;
    place(gApp->customScaresLabel, left, y, width, 20); y += 23;
    place(gApp->customScareBrokenLamp, left, y, width, 22); y += row;
    place(gApp->customScareAirVent, left, y, width, 22); y += row;
    place(gApp->customScareWater, left, y, width, 22); y += row;
    place(gApp->customScareBlood, left, y, width, 22); y += row;
    place(gApp->customScareFlesh, left, y, width, 22); y += row + 4;
    place(gApp->customBossesLabel, left, y, width, 20); y += 23;
    place(gApp->customBossOmukade, left, y, width, 22); y += row + 6;
    place(gApp->customSizeLabel, left, y, width, 20); y += 24;
    int editW = 58;
    place(gApp->customSizeXLabel, left, y + 3, 20, 20);
    place(gApp->customSizeX, left + 24, y, editW, 24);
    place(gApp->customSizeYLabel, left + 96, y + 3, 20, 20);
    place(gApp->customSizeY, left + 120, y, editW, 24); y += row + 9;
    place(gApp->customPages, left, y, width, 22); y += row + 10;
    int buttonW = std::max(92, std::min(142, (width - 12) / 2));
    place(gApp->customStart, left, y, buttonW, 28);
    place(gApp->customBack, left + buttonW + 12, y, buttonW, 28);
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
        SetWindowTextW(gApp->gameSinglePlayer, canResume ? L"Resume" : L"New Game");
    }
}

struct GameMenuButtonSpec {
    int id;
    const wchar_t* label;
};

std::vector<GameMenuButtonSpec> ActiveGameMenuButtons() {
    bool canResume = gApp && gApp->gameRunStarted && !gApp->gameDebugActive;
    bool canResumeSaved = std::filesystem::exists(GameSavePath());
    std::vector<GameMenuButtonSpec> buttons;
    buttons.reserve(6);
    if (canResume) buttons.push_back({kGameResumeCurrentRunId, L"Resume"});
    if (canResumeSaved) buttons.push_back({kGameResumeSavedRunId, L"Resume Saved Run"});
    buttons.push_back({kGameSinglePlayerId, L"New Game"});
    buttons.push_back({kGameCustomGameId, L"Custom Game"});
    buttons.push_back({kGameSettingsId, L"Settings"});
    buttons.push_back({kGameDebugId, L"Debug"});
    return buttons;
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
    int count = std::max(1, static_cast<int>(ActiveGameMenuButtons().size()));
    top = std::min(top, std::max(118, h - 78 - (buttonH + gap) * count));
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
    if (gApp->gameCustomMenuOpen) return 0;
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

int HitTestCustomGameMenu(HWND hwnd, POINT p) {
    if (!gApp || !gApp->gameShell || !gApp->gameCustomMenuOpen || !hwnd) return 0;
    std::vector<int> controls;
    if (gApp->gameCustomSelectedScare >= 0) {
        controls = {
            static_cast<int>(CustomGameMenuControl::ScareChanceMinus),
            static_cast<int>(CustomGameMenuControl::ScareChancePlus),
            static_cast<int>(CustomGameMenuControl::ScareStartMinMinus),
            static_cast<int>(CustomGameMenuControl::ScareStartMinPlus),
            static_cast<int>(CustomGameMenuControl::ScareStartMaxMinus),
            static_cast<int>(CustomGameMenuControl::ScareStartMaxPlus),
            static_cast<int>(CustomGameMenuControl::ScareDetailBack)
        };
    } else if (gApp->gameCustomSelectedScare == -2) {
        controls = {
            static_cast<int>(CustomGameMenuControl::EnvDirtMinus),
            static_cast<int>(CustomGameMenuControl::EnvDirtPlus),
            static_cast<int>(CustomGameMenuControl::EnvPaperMinus),
            static_cast<int>(CustomGameMenuControl::EnvPaperPlus),
            static_cast<int>(CustomGameMenuControl::EnvPropMinus),
            static_cast<int>(CustomGameMenuControl::EnvPropPlus),
            static_cast<int>(CustomGameMenuControl::EnvLampOnMinus),
            static_cast<int>(CustomGameMenuControl::EnvLampOnPlus),
            static_cast<int>(CustomGameMenuControl::EnvLampFlickerMinus),
            static_cast<int>(CustomGameMenuControl::EnvLampFlickerPlus),
            static_cast<int>(CustomGameMenuControl::EnvLampSparkMinus),
            static_cast<int>(CustomGameMenuControl::EnvLampSparkPlus),
            static_cast<int>(CustomGameMenuControl::EnvFogStartMinus),
            static_cast<int>(CustomGameMenuControl::EnvFogStartPlus),
            static_cast<int>(CustomGameMenuControl::EnvFogEndMinus),
            static_cast<int>(CustomGameMenuControl::EnvFogEndPlus),
            static_cast<int>(CustomGameMenuControl::EnvFogDarkMinus),
            static_cast<int>(CustomGameMenuControl::EnvFogDarkPlus),
            static_cast<int>(CustomGameMenuControl::ScareDetailBack)
        };
    } else {
        controls = {
            static_cast<int>(CustomGameMenuControl::BrokenLampScareDetails),
            static_cast<int>(CustomGameMenuControl::AirVentScareDetails),
            static_cast<int>(CustomGameMenuControl::WaterScareDetails),
            static_cast<int>(CustomGameMenuControl::BloodWorldScareDetails),
            static_cast<int>(CustomGameMenuControl::FleshWorldScareDetails),
            static_cast<int>(CustomGameMenuControl::BrokenLampScares),
            static_cast<int>(CustomGameMenuControl::AirVentScares),
            static_cast<int>(CustomGameMenuControl::WaterScares),
            static_cast<int>(CustomGameMenuControl::BloodWorldScares),
            static_cast<int>(CustomGameMenuControl::FleshWorldScares),
            static_cast<int>(CustomGameMenuControl::OmukadeBoss),
            static_cast<int>(CustomGameMenuControl::SizeXMinus),
            static_cast<int>(CustomGameMenuControl::SizeXPlus),
            static_cast<int>(CustomGameMenuControl::SizeYMinus),
            static_cast<int>(CustomGameMenuControl::SizeYPlus),
            static_cast<int>(CustomGameMenuControl::RoomCountMinus),
            static_cast<int>(CustomGameMenuControl::RoomCountPlus),
            static_cast<int>(CustomGameMenuControl::EnvironmentDetails),
            static_cast<int>(CustomGameMenuControl::EightPages),
            static_cast<int>(CustomGameMenuControl::Start),
            static_cast<int>(CustomGameMenuControl::Back)
        };
    }
    for (int control : controls) {
        RECT rc{};
        if (!gApp->renderer.CustomGameControlScreenRect(control, rc)) continue;
        if (p.x >= rc.left && p.x <= rc.right && p.y >= rc.top && p.y <= rc.bottom) return control;
    }
    return 0;
}

void PushGameMenuInteractionToRenderer(HWND hwnd) {
    if (!GameMenuUsesRendererScene() || !hwnd) return;
    if (gApp->gameCustomMenuOpen) {
        int hover = 0;
        if (gApp->gameMenuHasMouse) hover = HitTestCustomGameMenu(hwnd, gApp->gameMenuMouse);
        RECT rc{};
        GetClientRect(hwnd, &rc);
        int w = std::max<LONG>(1, rc.right - rc.left);
        int h = std::max<LONG>(1, rc.bottom - rc.top);
        float x = gApp->gameMenuHasMouse ? static_cast<float>(gApp->gameMenuMouse.x) / static_cast<float>(w) : 0.5f;
        float y = gApp->gameMenuHasMouse ? static_cast<float>(gApp->gameMenuMouse.y) / static_cast<float>(h) : 0.5f;
        gApp->renderer.SetMenuInteraction(x, y, false, false, false);
        gApp->renderer.SetMenuHoverButtonIndex(-1);
        gApp->renderer.SetMenuButtonLayout(gApp->gameRunStarted && !gApp->gameDebugActive,
            std::filesystem::exists(GameSavePath()));
        gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, hover, gApp->gameCustomSelectedScare);
        return;
    }
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
        hoverId == kGameSinglePlayerId);
    gApp->renderer.SetMenuHoverButtonIndex(hoverIndex);
    gApp->renderer.SetMenuButtonLayout(gApp->gameRunStarted && !gApp->gameDebugActive,
        std::filesystem::exists(GameSavePath()));
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

int CustomGameEditInt(HWND edit, int fallback) {
    if (!edit) return fallback;
    wchar_t text[32]{};
    GetWindowTextW(edit, text, static_cast<int>(std::size(text)));
    wchar_t* end = nullptr;
    long value = std::wcstol(text, &end, 10);
    return end != text ? static_cast<int>(value) : fallback;
}

bool CustomGameChecked(HWND control) {
    return control && SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

CustomGameSpec ReadCustomGameSpecFromControls() {
    CustomGameSpec spec = gApp ? gApp->gameCustomSpec : CustomGameSpec{};
    spec.layer = 1;
    spec.mazeWidth = std::clamp(spec.mazeWidth | 1, 3, 151);
    spec.mazeHeight = std::clamp(spec.mazeHeight | 1, 3, 151);
    spec.roomCount = std::clamp(spec.roomCount, 0, 80);
    spec.mapDirtPercent = std::clamp(spec.mapDirtPercent, 0, 100);
    spec.paperDensityPercent = std::clamp(spec.paperDensityPercent, 0, 400);
    spec.propDensityPercent = std::clamp(spec.propDensityPercent, 0, 400);
    spec.lampOnPercent = std::clamp(spec.lampOnPercent, 0, 100);
    spec.lampFlickerPercent = std::clamp(spec.lampFlickerPercent, 0, 100);
    spec.lampSparkPercent = std::clamp(spec.lampSparkPercent, 0, 100);
    spec.fogStartMeters = std::clamp(spec.fogStartMeters, 0, 200);
    spec.fogEndMeters = std::clamp(spec.fogEndMeters, spec.fogStartMeters + 1, 300);
    spec.fogDarknessPercent = std::clamp(spec.fogDarknessPercent, 0, 100);
    spec.jumpscareChancePercent = std::clamp(spec.jumpscareChancePercent, 0, 100);
    spec.jumpscareStartMinSeconds = std::clamp(spec.jumpscareStartMinSeconds, 0, 600);
    spec.jumpscareStartMaxSeconds = std::clamp(spec.jumpscareStartMaxSeconds, spec.jumpscareStartMinSeconds, 600);
    for (size_t i = 0; i < CustomGameSpec::kScareTypeCount; ++i) {
        spec.scareChancePercent[i] = std::clamp(spec.scareChancePercent[i], 0, 100);
        spec.scareStartMinSeconds[i] = std::clamp(spec.scareStartMinSeconds[i], 0, 600);
        spec.scareStartMaxSeconds[i] = std::clamp(spec.scareStartMaxSeconds[i], spec.scareStartMinSeconds[i], 600);
    }
    return spec;
}

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

void AdjustCustomGameSize(int& value, int delta) {
    value = std::clamp((value + delta) | 1, 3, 151);
}

void AdjustCustomGameRoomCount(int& value, int delta) {
    value = std::clamp(value + delta, 0, 80);
}

void AdjustCustomGameScareChance(int& value, int delta) {
    value = std::clamp(value + delta, 0, 100);
}

void AdjustCustomGamePercent(int& value, int delta, int maxValue = 100) {
    value = std::clamp(value + delta, 0, maxValue);
}

void AdjustCustomGameMeters(int& value, int delta, int minValue, int maxValue) {
    value = std::clamp(value + delta, minValue, maxValue);
}

void AdjustCustomGameScareStartWindow(CustomGameSpec& spec, bool minValue, int delta) {
    if (minValue) {
        spec.jumpscareStartMinSeconds = std::clamp(spec.jumpscareStartMinSeconds + delta, 0, 600);
        spec.jumpscareStartMaxSeconds = std::max(spec.jumpscareStartMaxSeconds, spec.jumpscareStartMinSeconds);
    } else {
        spec.jumpscareStartMaxSeconds = std::clamp(spec.jumpscareStartMaxSeconds + delta, 0, 600);
        spec.jumpscareStartMinSeconds = std::min(spec.jumpscareStartMinSeconds, spec.jumpscareStartMaxSeconds);
    }
}

int CustomGameScareIndexFromControl(CustomGameMenuControl control) {
    switch (control) {
    case CustomGameMenuControl::BrokenLampScareDetails: return 0;
    case CustomGameMenuControl::AirVentScareDetails: return 1;
    case CustomGameMenuControl::WaterScareDetails: return 2;
    case CustomGameMenuControl::BloodWorldScareDetails: return 3;
    case CustomGameMenuControl::FleshWorldScareDetails: return 4;
    default: return -1;
    }
}

void AdjustSelectedCustomScareChance(int delta) {
    if (!gApp) return;
    int index = std::clamp(gApp->gameCustomSelectedScare, 0, CustomGameSpec::kScareTypeCount - 1);
    gApp->gameCustomSpec.scareChancePercent[static_cast<size_t>(index)] =
        std::clamp(gApp->gameCustomSpec.scareChancePercent[static_cast<size_t>(index)] + delta, 0, 100);
}

void AdjustSelectedCustomScareStartWindow(bool minValue, int delta) {
    if (!gApp) return;
    int index = std::clamp(gApp->gameCustomSelectedScare, 0, CustomGameSpec::kScareTypeCount - 1);
    size_t i = static_cast<size_t>(index);
    if (minValue) {
        gApp->gameCustomSpec.scareStartMinSeconds[i] = std::clamp(gApp->gameCustomSpec.scareStartMinSeconds[i] + delta, 0, 600);
        gApp->gameCustomSpec.scareStartMaxSeconds[i] = std::max(gApp->gameCustomSpec.scareStartMaxSeconds[i], gApp->gameCustomSpec.scareStartMinSeconds[i]);
    } else {
        gApp->gameCustomSpec.scareStartMaxSeconds[i] = std::clamp(gApp->gameCustomSpec.scareStartMaxSeconds[i] + delta, 0, 600);
        gApp->gameCustomSpec.scareStartMinSeconds[i] = std::min(gApp->gameCustomSpec.scareStartMinSeconds[i], gApp->gameCustomSpec.scareStartMaxSeconds[i]);
    }
}

void ActivateCustomGameMenuCommand(HWND hwnd, int control) {
    if (!gApp || !gApp->gameCustomMenuOpen) return;
    if (gApp->gameCustomSelectedScare == -2) {
        switch (static_cast<CustomGameMenuControl>(control)) {
        case CustomGameMenuControl::EnvDirtMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.mapDirtPercent, -5); break;
        case CustomGameMenuControl::EnvDirtPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.mapDirtPercent, 5); break;
        case CustomGameMenuControl::EnvPaperMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.paperDensityPercent, -10, 400); break;
        case CustomGameMenuControl::EnvPaperPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.paperDensityPercent, 10, 400); break;
        case CustomGameMenuControl::EnvPropMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.propDensityPercent, -10, 400); break;
        case CustomGameMenuControl::EnvPropPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.propDensityPercent, 10, 400); break;
        case CustomGameMenuControl::EnvLampOnMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampOnPercent, -5); break;
        case CustomGameMenuControl::EnvLampOnPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampOnPercent, 5); break;
        case CustomGameMenuControl::EnvLampFlickerMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampFlickerPercent, -5); break;
        case CustomGameMenuControl::EnvLampFlickerPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampFlickerPercent, 5); break;
        case CustomGameMenuControl::EnvLampSparkMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampSparkPercent, -5); break;
        case CustomGameMenuControl::EnvLampSparkPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.lampSparkPercent, 5); break;
        case CustomGameMenuControl::EnvFogStartMinus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogStartMeters, -1, 0, 200); break;
        case CustomGameMenuControl::EnvFogStartPlus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogStartMeters, 1, 0, 200); break;
        case CustomGameMenuControl::EnvFogEndMinus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogEndMeters, -1, 1, 300); break;
        case CustomGameMenuControl::EnvFogEndPlus: AdjustCustomGameMeters(gApp->gameCustomSpec.fogEndMeters, 1, 1, 300); break;
        case CustomGameMenuControl::EnvFogDarkMinus: AdjustCustomGamePercent(gApp->gameCustomSpec.fogDarknessPercent, -5); break;
        case CustomGameMenuControl::EnvFogDarkPlus: AdjustCustomGamePercent(gApp->gameCustomSpec.fogDarknessPercent, 5); break;
        case CustomGameMenuControl::ScareDetailBack: gApp->gameCustomSelectedScare = -1; break;
        default: break;
        }
        gApp->gameCustomSpec.fogEndMeters = std::max(gApp->gameCustomSpec.fogEndMeters, gApp->gameCustomSpec.fogStartMeters + 1);
        if (gApp->rendererInitialized) {
            gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, control, gApp->gameCustomSelectedScare);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
    if (gApp->gameCustomSelectedScare >= 0) {
        switch (static_cast<CustomGameMenuControl>(control)) {
        case CustomGameMenuControl::ScareChanceMinus: AdjustSelectedCustomScareChance(-5); break;
        case CustomGameMenuControl::ScareChancePlus: AdjustSelectedCustomScareChance(5); break;
        case CustomGameMenuControl::ScareStartMinMinus: AdjustSelectedCustomScareStartWindow(true, -5); break;
        case CustomGameMenuControl::ScareStartMinPlus: AdjustSelectedCustomScareStartWindow(true, 5); break;
        case CustomGameMenuControl::ScareStartMaxMinus: AdjustSelectedCustomScareStartWindow(false, -5); break;
        case CustomGameMenuControl::ScareStartMaxPlus: AdjustSelectedCustomScareStartWindow(false, 5); break;
        case CustomGameMenuControl::ScareDetailBack: gApp->gameCustomSelectedScare = -1; break;
        default: break;
        }
        if (gApp->rendererInitialized) {
            gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, control, gApp->gameCustomSelectedScare);
        }
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
    switch (static_cast<CustomGameMenuControl>(control)) {
    case CustomGameMenuControl::BrokenLampScareDetails:
    case CustomGameMenuControl::AirVentScareDetails:
    case CustomGameMenuControl::WaterScareDetails:
    case CustomGameMenuControl::BloodWorldScareDetails:
    case CustomGameMenuControl::FleshWorldScareDetails:
        gApp->gameCustomSelectedScare = CustomGameScareIndexFromControl(static_cast<CustomGameMenuControl>(control));
        break;
    case CustomGameMenuControl::EnvironmentDetails:
        gApp->gameCustomSelectedScare = -2;
        break;
    case CustomGameMenuControl::BrokenLampScares: gApp->gameCustomSpec.brokenLampScares = !gApp->gameCustomSpec.brokenLampScares; break;
    case CustomGameMenuControl::AirVentScares: gApp->gameCustomSpec.airVentScares = !gApp->gameCustomSpec.airVentScares; break;
    case CustomGameMenuControl::WaterScares: gApp->gameCustomSpec.waterScares = !gApp->gameCustomSpec.waterScares; break;
    case CustomGameMenuControl::BloodWorldScares: gApp->gameCustomSpec.bloodWorldScares = !gApp->gameCustomSpec.bloodWorldScares; break;
    case CustomGameMenuControl::FleshWorldScares: gApp->gameCustomSpec.fleshWorldScares = !gApp->gameCustomSpec.fleshWorldScares; break;
    case CustomGameMenuControl::OmukadeBoss: gApp->gameCustomSpec.omukadeBoss = !gApp->gameCustomSpec.omukadeBoss; break;
    case CustomGameMenuControl::SizeXMinus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeWidth, -2); break;
    case CustomGameMenuControl::SizeXPlus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeWidth, 2); break;
    case CustomGameMenuControl::SizeYMinus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeHeight, -2); break;
    case CustomGameMenuControl::SizeYPlus: AdjustCustomGameSize(gApp->gameCustomSpec.mazeHeight, 2); break;
    case CustomGameMenuControl::RoomCountMinus: AdjustCustomGameRoomCount(gApp->gameCustomSpec.roomCount, -1); break;
    case CustomGameMenuControl::RoomCountPlus: AdjustCustomGameRoomCount(gApp->gameCustomSpec.roomCount, 1); break;
    case CustomGameMenuControl::ScareChanceMinus: AdjustCustomGameScareChance(gApp->gameCustomSpec.jumpscareChancePercent, -5); break;
    case CustomGameMenuControl::ScareChancePlus: AdjustCustomGameScareChance(gApp->gameCustomSpec.jumpscareChancePercent, 5); break;
    case CustomGameMenuControl::ScareStartMinMinus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, true, -5); break;
    case CustomGameMenuControl::ScareStartMinPlus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, true, 5); break;
    case CustomGameMenuControl::ScareStartMaxMinus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, false, -5); break;
    case CustomGameMenuControl::ScareStartMaxPlus: AdjustCustomGameScareStartWindow(gApp->gameCustomSpec, false, 5); break;
    case CustomGameMenuControl::EightPages: gApp->gameCustomSpec.eightPages = !gApp->gameCustomSpec.eightPages; break;
    case CustomGameMenuControl::Start: StartCustomGameFromMenu(hwnd); return;
    case CustomGameMenuControl::Back: ExitGameCustomMenu(hwnd); return;
    default: break;
    }
    if (gApp->rendererInitialized) {
        gApp->renderer.SetCustomGameMenuState(gApp->gameCustomSpec, control, gApp->gameCustomSelectedScare);
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

bool GameWindowAcceptsInput(HWND hwnd) {
    if (!gApp || hwnd == nullptr || !gApp->gameWindowActive || IsIconic(hwnd)) return false;
    HWND foreground = GetForegroundWindow();
    if (foreground == hwnd) return true;
    if (!foreground) return false;
    if (IsChild(hwnd, foreground)) return true;
    HWND root = GetAncestor(foreground, GA_ROOT);
    HWND rootOwner = GetAncestor(foreground, GA_ROOTOWNER);
    return root == hwnd || rootOwner == hwnd;
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
        std::wstring detail = gApp->renderer.LastInitializeError();
        std::wstring message = detail.empty()
            ? L"Direct3D initialization failed."
            : L"Direct3D initialization failed.\r\n\r\n" + detail;
        MessageBoxW(hwnd, message.c_str(), L"Backrooms Maze Game", MB_OK | MB_ICONERROR);
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
    if (!GameWindowAcceptsInput(gApp->hwnd)) {
        gApp->gameMouseDeltaX = 0.0f;
        gApp->gameMouseDeltaY = 0.0f;
        return input;
    }
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
