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
