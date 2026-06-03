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
