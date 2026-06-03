// Game settings panel callbacks and embedded settings transition.

void OnGameSettingsPanelSaved(void*, const Settings& settings) {
    if (!gApp) return;
    gApp->gameInputSettings = settings;
    if (gApp->rendererInitialized) gApp->renderer.ApplyGameSettings(settings);
    ApplyGameWindowSettings(gApp->hwnd, settings);
}

void OnGameSettingsPanelKeyCaptureChanged(void*, bool active, bool escapeConsumed) {
    if (!gApp) return;
    gApp->gameSettingsKeyCaptureActive = active;
    gApp->gameSettingsEscapeConsumed = escapeConsumed;
}

void OnGameSettingsPanelClosed(void*, HWND hwnd) {
    if (!gApp) return;
    if (gApp->gameConfig == hwnd) {
        gApp->gameConfig = nullptr;
        PostMessageW(gApp->hwnd, kGameConfigClosedMessage, 0, 0);
    }
}
