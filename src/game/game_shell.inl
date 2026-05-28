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

void DrawGameMenuText(HDC dc, const std::wstring& text, RECT rc, COLORREF color,
    UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    DrawTextW(dc, text.c_str(), -1, &rc, format);
}

void FillGameMenuRect(HDC dc, const RECT& rc, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rc, brush);
    DeleteObject(brush);
}

void FillGameMenuPolygon(HDC dc, const POINT* points, int count, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    Polygon(dc, points, count);
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
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
}

void DrawGameMenuButton(HDC dc, const RECT& rc, const wchar_t* label, bool hover) {
    COLORREF outer = hover ? RGB(206, 169, 92) : RGB(112, 96, 64);
    COLORREF fill = hover ? RGB(59, 49, 36) : RGB(31, 29, 24);
    COLORREF inner = hover ? RGB(96, 76, 44) : RGB(55, 48, 35);
    FillGameMenuRect(dc, rc, RGB(7, 7, 6));
    RECT shadow{rc.left + 4, rc.top + 5, rc.right + 4, rc.bottom + 5};
    FillGameMenuRect(dc, shadow, RGB(0, 0, 0));
    FillGameMenuRect(dc, rc, outer);
    RECT body = rc;
    InflateRect(&body, -2, -2);
    FillGameMenuRect(dc, body, fill);
    RECT stripe{body.left, body.top, body.left + 5, body.bottom};
    FillGameMenuRect(dc, stripe, hover ? RGB(202, 151, 58) : RGB(126, 96, 42));
    HPEN pen = CreatePen(PS_SOLID, 1, inner);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    MoveToEx(dc, body.left + 14, body.bottom - 9, nullptr);
    LineTo(dc, body.right - 14, body.bottom - 9);
    SelectObject(dc, oldPen);
    DeleteObject(pen);

    RECT text = body;
    InflateRect(&text, -20, 0);
    DrawGameMenuText(dc, label, text, hover ? RGB(255, 238, 188) : RGB(231, 224, 204),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawGameMenuButtonLabel(HDC dc, const RECT& rc, const wchar_t* label, bool hover) {
    HPEN pen = CreatePen(PS_SOLID, hover ? 2 : 1, hover ? RGB(226, 185, 84) : RGB(114, 98, 63));
    HGDIOBJ oldPen = SelectObject(dc, pen);
    HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(pen);
    RECT text = rc;
    InflateRect(&text, -12, 0);
    DrawGameMenuText(dc, label, text, hover ? RGB(255, 236, 174) : RGB(218, 206, 174),
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void DrawGameMenuBloodStreak(HDC dc, int x, int top, int length, int width, int ageMs) {
    float age = Clamp01(static_cast<float>(ageMs) / 1600.0f);
    int visibleLength = static_cast<int>(std::round(length * age));
    if (visibleLength <= 1) return;
    RECT main{x, top, x + width, top + visibleLength};
    FillGameMenuRect(dc, main, RGB(74, 3, 2));
    RECT highlight{x + std::max(1, width / 3), top + 2, x + std::max(2, width / 3 + 1), top + visibleLength};
    FillGameMenuRect(dc, highlight, RGB(143, 15, 8));
    int drop = std::min(visibleLength + 8, length + 12);
    RECT bead{x - width / 2, top + drop - width, x + width + width / 2, top + drop + width / 2};
    FillGameMenuRect(dc, bead, RGB(96, 5, 3));
}

void DrawGameMenuBlood(HDC dc, const RECT& client, ULONGLONG now) {
    if (!gApp || gApp->gameMenuBloodStart == 0) return;
    int ageMs = static_cast<int>(std::min<ULONGLONG>(6000, now - gApp->gameMenuBloodStart));
    if (ageMs <= 0) return;
    RECT first = GameMenuButtonRect(client, 0);
    int wallTop = std::max<LONG>(130, first.top - 94);
    int wallBottom = GameMenuButtonRect(client, 3).bottom + 40;
    int seeds[] = {17, 43, 71, 109, 157, 193, 229};
    for (int i = 0; i < 7; ++i) {
        int x = first.left + (first.right - first.left) * (seeds[i] % 100) / 100;
        int top = wallTop + (seeds[(i + 2) % 7] % 46);
        int len = std::max(56, wallBottom - top - (i % 3) * 26);
        int width = 3 + (i % 4);
        DrawGameMenuBloodStreak(dc, x, top, len, width, ageMs - i * 120);
    }
    for (int i = 0; i < 4; ++i) {
        RECT br = GameMenuButtonRect(client, i);
        int localAge = ageMs - 220 - i * 105;
        if (localAge <= 0) continue;
        DrawGameMenuBloodStreak(dc, br.left + 30 + i * 37, br.top - 4, br.bottom - br.top + 24, 5, localAge);
        DrawGameMenuBloodStreak(dc, br.right - 44 - i * 23, br.top - 2, br.bottom - br.top + 18, 4, localAge - 80);
        RECT smear{br.left + 16, br.bottom - 10, br.right - 18, br.bottom - 6};
        FillGameMenuRect(dc, smear, RGB(48, 2, 2));
    }
}

void DrawGameMenuLampBurst(HDC dc, const RECT& client, ULONGLONG now) {
    if (!gApp || gApp->gameMenuLampBurstStart == 0) return;
    ULONGLONG elapsed = now - gApp->gameMenuLampBurstStart;
    if (elapsed > 520) return;
    RECT first = GameMenuButtonRect(client, 0);
    int cx = (first.left + first.right) / 2;
    int cy = std::max<LONG>(58, first.top - 116);
    float t = static_cast<float>(elapsed) / 520.0f;
    int sparkCount = 18;
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 207, 98));
    HGDIOBJ oldPen = SelectObject(dc, pen);
    for (int i = 0; i < sparkCount; ++i) {
        float a = static_cast<float>(i) * 2.399963f + t * 3.1f;
        float len = (28.0f + static_cast<float>((i * 17) % 44)) * (1.0f - t * 0.35f);
        int x1 = cx + static_cast<int>(std::cos(a) * 10.0f);
        int y1 = cy + static_cast<int>(std::sin(a) * 5.0f);
        int x2 = cx + static_cast<int>(std::cos(a) * len);
        int y2 = cy + static_cast<int>(std::sin(a) * len * 0.55f + t * 34.0f);
        MoveToEx(dc, x1, y1, nullptr);
        LineTo(dc, x2, y2);
    }
    SelectObject(dc, oldPen);
    DeleteObject(pen);
    RECT flash{cx - 90, cy - 30, cx + 90, cy + 36};
    FillGameMenuRect(dc, flash, elapsed < 90 ? RGB(86, 65, 31) : RGB(34, 24, 12));
}

void DrawGameMenuExitDoor(HDC dc, const RECT& client, ULONGLONG now) {
    if (!gApp) return;
    int h = std::max<LONG>(1, client.bottom - client.top);
    RECT door{client.right - 118, h / 2 - 92, client.right - 56, h / 2 + 118};
    bool open = gApp->gameMenuHoverId == kGameExitId;
    FillGameMenuRect(dc, door, open ? RGB(3, 3, 3) : RGB(24, 21, 15));
    RECT edge = door;
    edge.right = edge.left + 4;
    FillGameMenuRect(dc, edge, RGB(111, 89, 44));
    if (open) {
        POINT hall[4] = {{door.left + 6, door.top + 6}, {door.right + 44, door.top - 18},
            {door.right + 44, door.bottom + 16}, {door.left + 6, door.bottom - 6}};
        FillGameMenuPolygon(dc, hall, 4, RGB(1, 1, 1));
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(35, 30, 17));
        HGDIOBJ old = SelectObject(dc, pen);
        for (int i = 0; i < 5; ++i) {
            MoveToEx(dc, door.left + 12, door.top + 22 + i * 32, nullptr);
            LineTo(dc, door.right + 36, door.top + 10 + i * 38);
        }
        SelectObject(dc, old);
        DeleteObject(pen);
    } else {
        RECT knob{door.right - 16, (door.top + door.bottom) / 2 - 4, door.right - 8, (door.top + door.bottom) / 2 + 4};
        FillGameMenuRect(dc, knob, RGB(152, 122, 58));
    }
}

void DrawGameMenuFlashlight(HDC dc, const RECT& client, ULONGLONG now) {
    if (!gApp || !gApp->gameMenuHasMouse) return;
    int w = std::max<LONG>(1, client.right - client.left);
    int h = std::max<LONG>(1, client.bottom - client.top);
    float mx = static_cast<float>(gApp->gameMenuMouse.x) / static_cast<float>(w);
    float my = static_cast<float>(gApp->gameMenuMouse.y) / static_cast<float>(h);
    float jitterX = std::sin(static_cast<float>(now) * 0.0081f) * 0.018f + std::sin(static_cast<float>(now) * 0.021f) * 0.006f;
    float jitterY = std::cos(static_cast<float>(now) * 0.0067f) * 0.012f + std::sin(static_cast<float>(now) * 0.017f) * 0.005f;
    int cx = static_cast<int>((mx + jitterX) * w);
    int cy = static_cast<int>((my + jitterY) * h);
    bool flicker = GameMenuHoverIsButton(gApp->gameMenuHasMouse ? gApp->gameMenuHoverId : 0);
    int radius = flicker ? 168 : 205;
    int rings = 9;
    for (int i = rings; i >= 1; --i) {
        int r = radius * i / rings;
        int alphaShade = flicker && (now / 80) % 3 == 0 ? i * 3 : i * 4;
        COLORREF color = RGB(std::min(70, 18 + alphaShade), std::min(61, 15 + alphaShade), std::min(38, 9 + alphaShade / 2));
        RECT glow{cx - r, cy - r * 2 / 3, cx + r, cy + r * 2 / 3};
        FillGameMenuRect(dc, glow, color);
    }
}

void DrawGameMenuDust(HDC dc, const RECT& client, ULONGLONG now) {
    int w = std::max<LONG>(1, client.right - client.left);
    int h = std::max<LONG>(1, client.bottom - client.top);
    int focusX = gApp && gApp->gameMenuHasMouse ? gApp->gameMenuMouse.x : w / 2;
    int focusY = gApp && gApp->gameMenuHasMouse ? gApp->gameMenuMouse.y : h / 2;
    constexpr int kDustCount = 120;
    for (int i = 0; i < kDustCount; ++i) {
        float seed = static_cast<float>(i);
        float drift = static_cast<float>(now) * (0.000035f + std::fmod(seed * 0.017f, 0.00009f));
        float xNorm = std::fmod(seed * 0.6180339f + drift, 1.0f);
        float yNorm = std::fmod(seed * 0.3819660f + drift * (0.42f + std::fmod(seed, 5.0f) * 0.09f), 1.0f);
        int x = static_cast<int>(xNorm * static_cast<float>(w));
        int y = static_cast<int>(yNorm * static_cast<float>(h));
        float dx = static_cast<float>(x - focusX) / std::max(1.0f, static_cast<float>(w) * 0.42f);
        float dy = static_cast<float>(y - focusY) / std::max(1.0f, static_cast<float>(h) * 0.34f);
        float beam = Clamp01(1.0f - std::sqrt(dx * dx + dy * dy));
        if (beam <= 0.08f) continue;
        int size = 1 + (i % 4 == 0 ? 1 : 0);
        int shade = static_cast<int>(24.0f + beam * 46.0f);
        RECT mote{x, y, x + size, y + size};
        FillGameMenuRect(dc, mote, RGB(shade, shade - 4, std::max(8, shade - 18)));
    }
}

float GameMenuFadeAmount(ULONGLONG now) {
    if (!gApp) return 0.0f;
    constexpr float kFadeInMs = 850.0f;
    constexpr float kFadeOutMs = 360.0f;
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
        FillGameMenuRect(dc, rc, RGB(7, 7, 5));
        for (int y = 0; y < h; y += 22) {
        int shade = 10 + (y * 15 / std::max(1, h));
        RECT band{0, y, w, std::min(y + 22, h)};
        FillGameMenuRect(dc, band, RGB(shade, std::max(5, shade - 2), std::max(3, shade - 6)));
        }
        POINT floorPoly[4] = {{0, h}, {w, h}, {w * 62 / 100, h * 50 / 100}, {w * 38 / 100, h * 50 / 100}};
        POINT ceilingPoly[4] = {{0, 0}, {w, 0}, {w * 62 / 100, h * 30 / 100}, {w * 38 / 100, h * 30 / 100}};
        POINT leftWall[4] = {{0, 0}, {w * 38 / 100, h * 30 / 100}, {w * 38 / 100, h * 50 / 100}, {0, h}};
        POINT rightWall[4] = {{w, 0}, {w * 62 / 100, h * 30 / 100}, {w * 62 / 100, h * 50 / 100}, {w, h}};
        FillGameMenuPolygon(dc, ceilingPoly, 4, RGB(13, 12, 8));
        FillGameMenuPolygon(dc, floorPoly, 4, RGB(18, 16, 11));
        FillGameMenuPolygon(dc, leftWall, 4, RGB(10, 10, 7));
        FillGameMenuPolygon(dc, rightWall, 4, RGB(9, 9, 7));

        HPEN perspectivePen = CreatePen(PS_SOLID, 1, RGB(35, 30, 18));
        HGDIOBJ oldPerspectivePen = SelectObject(dc, perspectivePen);
        for (int i = 0; i <= 8; ++i) {
            int x0 = i * w / 8;
            MoveToEx(dc, x0, h, nullptr);
            LineTo(dc, w / 2, h * 47 / 100);
        }
        for (int i = 0; i < 8; ++i) {
            int yLine = h * (54 + i * 6) / 100;
            MoveToEx(dc, 0, yLine, nullptr);
            LineTo(dc, w, yLine);
        }
        SelectObject(dc, oldPerspectivePen);
        DeleteObject(perspectivePen);

        for (int i = 0; i < 12; ++i) {
            int x = (i * 137 + 43) % std::max(1, w);
            int panelW = 34 + (i % 4) * 18;
            RECT panel{x - panelW / 2, 0, x + panelW / 2, h};
            FillGameMenuRect(dc, panel, RGB(14 + i % 3, 13 + i % 2, 9));
        }
        for (int i = 0; i < 7; ++i) {
            int lx = w / 2 - 310 + i * 104;
            RECT lamp{lx, 28, lx + 54, 34};
            FillGameMenuRect(dc, lamp, RGB(121, 105, 66));
            RECT glow{lx - 24, 34, lx + 78, 124};
            FillGameMenuRect(dc, glow, RGB(23, 20, 12));
        }
        RECT vignetteLeft{0, 0, w / 18, h};
        RECT vignetteRight{w - w / 18, 0, w, h};
        RECT vignetteTop{0, 0, w, h / 18};
        RECT vignetteBottom{0, h - h / 12, w, h};
        FillGameMenuRect(dc, vignetteLeft, RGB(2, 2, 1));
        FillGameMenuRect(dc, vignetteRight, RGB(2, 2, 1));
        FillGameMenuRect(dc, vignetteTop, RGB(2, 2, 1));
        FillGameMenuRect(dc, vignetteBottom, RGB(3, 3, 2));
        DrawGameMenuFlashlight(dc, rc, now);
        DrawGameMenuDust(dc, rc, now);
        DrawGameMenuExitDoor(dc, rc, now);
        DrawGameMenuLampBurst(dc, rc, now);
    }

    HFONT titleFont = CreateFontW(46, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT bodyFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT buttonFont = CreateFontW(19, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(dc, titleFont);

    if (!rendererScene) {
    if (!rendererScene) {
        RECT title{0, 64, w, 124};
        DrawGameMenuText(dc, L"Backrooms Maze", title, RGB(239, 226, 182), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, bodyFont);
        RECT subtitle{0, 118, w, 150};
        DrawGameMenuText(dc, gApp->gameRunStarted && !gApp->gameDebugActive ? L"Paused" : L"Main Menu",
            subtitle, RGB(154, 139, 102), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        SelectObject(dc, bodyFont);
    }
    } else {
        SelectObject(dc, bodyFont);
    }

    SelectObject(dc, buttonFont);
    auto buttons = ActiveGameMenuButtons();
    for (int i = 0; i < static_cast<int>(buttons.size()); ++i) {
        if (rendererScene) continue;
        RECT br = GameMenuButtonRect(rc, i);
        bool hover = gApp->gameMenuHoverId == buttons[static_cast<size_t>(i)].id;
        DrawGameMenuButton(dc, br, buttons[static_cast<size_t>(i)].label, hover);
    }
    if (!rendererScene) DrawGameMenuBlood(dc, rc, now);

    SelectObject(dc, bodyFont);
    if (!rendererScene) {
        RECT footer{26, h - 48, w - 26, h - 20};
        DrawGameMenuText(dc, L"v0 prototype", footer, RGB(103, 94, 73), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }
    DrawGameMenuFade(dc, rc, GameMenuFadeAmount(now));

    SelectObject(dc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(bodyFont);
    DeleteObject(buttonFont);
}

void ActivateGameMenuCommand(HWND hwnd, int id) {
    if (!gApp || !gApp->gameShell || hwnd != gApp->hwnd) return;
    if (gApp->gameMenuFadeOut) return;
    gApp->gameMenuPendingCommand = id;
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
    if (gApp->gameMenuFadeIn && now - gApp->gameMenuFadeStart >= 850) {
        gApp->gameMenuFadeIn = false;
    }
    if (gApp->gameMenuFadeOut && now - gApp->gameMenuFadeStart >= 360) {
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
    StartupProgressSink loadingProgress{LoadingProgressCallback, gApp->loadingOverlay};
    if (!gApp->renderer.Initialize(hwnd, nullptr, false, MonsterPreviewView::Orbit,
            gApp->loadingOverlay ? &loadingProgress : nullptr)) {
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
    input.jump = down(GameActionKey(settings, GameInputAction::Jump));
    input.sprint = down(GameActionKey(settings, GameInputAction::Sprint));
    input.crouch = down(GameActionKey(settings, GameInputAction::Crouch));
    input.interact = down(GameActionKey(settings, GameInputAction::Interact));
    input.pause = down(GameActionKey(settings, GameInputAction::Pause));
    return input;
}
