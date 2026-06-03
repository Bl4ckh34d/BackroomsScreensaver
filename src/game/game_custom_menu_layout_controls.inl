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
