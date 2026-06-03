// Main-menu presentation.

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
