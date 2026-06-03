// Custom-game menu layout.

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
