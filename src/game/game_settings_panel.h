#pragma once

struct Settings;

struct GameSettingsPanelHost {
    void* context = nullptr;
    void (*onSave)(void* context, const Settings& settings) = nullptr;
    void (*onKeyCaptureChanged)(void* context, bool active, bool escapeConsumed) = nullptr;
    void (*onClosed)(void* context, HWND hwnd) = nullptr;
};

HWND CreateGameSettingsPanel(HWND parent, GameSettingsPanelHost host = {});
void SaveSettingsToIni(const Settings& settings);
