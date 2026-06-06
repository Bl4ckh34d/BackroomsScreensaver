void PushGameMenuInteractionToRenderer(HWND hwnd) {
    if (!GameMenuUsesRendererScene() || !hwnd) return;
    if (gApp->gameSettingsBoardOpen) {
        int hover = 0;
        if (gApp->gameMenuHasMouse) hover = HitTestSettingsBoard(hwnd, gApp->gameMenuMouse);
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
        gApp->renderer.SetSettingsBoardState(gApp->gameSettingsBoardSettings, hover, gApp->gameSettingsBoardTab,
            gApp->gameSettingsBoardCaptureAction);
        return;
    }
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
