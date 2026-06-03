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
