// Main-menu layout.

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
