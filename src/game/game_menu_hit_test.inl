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
