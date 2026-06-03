static bool HandleConfigCommand(HWND hwnd, ConfigState* state, WPARAM wParam) {
    int id = LOWORD(wParam);
    if (id == kConfigSaveId && state) {
        SaveConfigControls(state);
        if (state->mode == ConfigDialogMode::Debug && gApp
#if defined(BACKROOMS_GAME_EXE)
            && gApp->rendererInitialized
#endif
            ) {
            gApp->renderer.ApplyGameSettings(LoadSettings());
        }
        const wchar_t* message = state->mode == ConfigDialogMode::Game
            ? L"Game settings saved. Display settings apply next launch; start a new run to reload level-generation settings."
            : (state->mode == ConfigDialogMode::Debug
                ? L"Debug settings saved. Re-enter Debug or update the preview to reload scene-generation settings."
                : L"Settings saved. Restart the screensaver to use changed values.");
        MessageBoxW(hwnd, message, L"Backrooms Maze", MB_OK | MB_ICONINFORMATION);
        return true;
    }
    if (id == kConfigResetId && state) {
        const wchar_t* prompt = state->mode == ConfigDialogMode::Full
            ? L"Reset all settings to defaults?"
            : L"Reset the visible settings in this view to defaults?";
        if (MessageBoxW(hwnd, prompt, L"Backrooms Maze", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            if (state->mode == ConfigDialogMode::Full) {
                WriteTextFile(SettingsPath(), DefaultConfigText());
                LoadConfigControls(state, true);
            } else {
                ResetVisibleConfigControls(state);
            }
            ScheduleConfigPreview(state, 0);
        }
        return true;
    }
    if (id == kConfigOpenId) {
        std::wstring arg = L"\"" + SettingsPath().wstring() + L"\"";
        ShellExecuteW(hwnd, L"open", L"notepad.exe", arg.c_str(), nullptr, SW_SHOWNORMAL);
        return true;
    }
    if (id == kConfigPreviewUpdateId && state) {
        RestartConfigPreview(state);
        return true;
    }
    if (id == IDCANCEL) {
        DestroyWindow(hwnd);
        return true;
    }
    if (state) {
        ConfigFieldUi* field = FindConfigFieldById(state, id);
        WORD code = HIWORD(wParam);
        if (field && (code == EN_CHANGE || code == BN_CLICKED)) {
            ScheduleConfigPreview(state);
            return true;
        }
    }
    return false;
}
