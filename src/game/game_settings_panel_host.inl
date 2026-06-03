GameSettingsPanelHost BuildGameSettingsPanelHost(HWND hwnd) {
    return {
        hwnd,
        OnGameSettingsPanelSaved,
        OnGameSettingsPanelKeyCaptureChanged,
        OnGameSettingsPanelClosed
    };
}
