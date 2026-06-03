#include "game_settings_panel_internal.h"

std::wstring FormatSettingValue(float value) {
    wchar_t buffer[32]{};
    swprintf_s(buffer, L"%.2f", value);
    return buffer;
}

std::wstring FormatSettingValue(int value) {
    wchar_t buffer[32]{};
    swprintf_s(buffer, L"%d", value);
    return buffer;
}

std::wstring FormatResolution(int width, int height) {
    return FormatSettingValue(width) + L" x " + FormatSettingValue(height);
}

void BuildGameResolutionOptions(GameSettingsPanelState* state) {
    if (!state) return;
    state->resolutionOptions.clear();
    auto add = [&](int width, int height) {
        if (width < 640 || height < 360) return;
        POINT p{width, height};
        auto it = std::find_if(state->resolutionOptions.begin(), state->resolutionOptions.end(),
            [&](const POINT& existing) { return existing.x == p.x && existing.y == p.y; });
        if (it == state->resolutionOptions.end()) state->resolutionOptions.push_back(p);
    };

    DEVMODEW mode{};
    mode.dmSize = sizeof(mode);
    for (DWORD i = 0; EnumDisplaySettingsW(nullptr, i, &mode); ++i) {
        add(static_cast<int>(mode.dmPelsWidth), static_cast<int>(mode.dmPelsHeight));
    }
    add(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    add(3840, 2160);
    add(2560, 1440);
    add(1920, 1080);
    add(1600, 900);
    add(1366, 768);
    add(1280, 720);
    add(1280, 760);
    add(state->settings.gameResolutionWidth, state->settings.gameResolutionHeight);

    std::sort(state->resolutionOptions.begin(), state->resolutionOptions.end(), [](const POINT& a, const POINT& b) {
        if (a.x != b.x) return a.x > b.x;
        return a.y > b.y;
    });
}

void WriteIniFloat(const wchar_t* section, const wchar_t* key, float value) {
    WritePrivateProfileStringW(section, key, FormatSettingValue(value).c_str(), SettingsPath().c_str());
}

void WriteIniIntValue(const wchar_t* section, const wchar_t* key, int value) {
    WritePrivateProfileStringW(section, key, FormatSettingValue(value).c_str(), SettingsPath().c_str());
}

void SaveGameSettingsPanel(const GameSettingsPanelState* state) {
    if (!state) return;
    const Settings& s = state->settings;
    WriteIniIntValue(L"GameWindow", L"Fullscreen", s.gameFullscreen ? 1 : 0);
    WriteIniIntValue(L"GameWindow", L"ResolutionWidth", s.gameResolutionWidth);
    WriteIniIntValue(L"GameWindow", L"ResolutionHeight", s.gameResolutionHeight);
    WriteIniIntValue(L"GameWindow", L"FrameRateLimit", s.gameFrameRateLimit);
    WriteIniIntValue(L"Renderer", L"AllowWarpFallback", s.allowWarpFallback ? 1 : 0);
    WriteIniFloat(L"Lighting", L"Exposure", s.exposure);
    WriteIniFloat(L"Lighting", L"BloomAmount", s.bloomAmount);
    WriteIniFloat(L"Lighting", L"MotionBlurAmount", s.motionBlurAmount);
    WriteIniFloat(L"Atmosphere", L"AirParticleDensity", s.airParticleDensity);
    WriteIniIntValue(L"Monster", L"MonsterIgnorePlayer", s.monsterIgnorePlayer ? 1 : 0);
    WriteIniIntValue(L"Debug", L"InfiniteStamina", s.debugInfiniteStamina ? 1 : 0);
    WriteIniIntValue(L"Debug", L"Invincible", s.debugInvincible ? 1 : 0);
    WriteIniFloat(L"Controls", L"MouseSensitivity", s.mouseSensitivity);
    WriteIniIntValue(L"Controls", L"InvertMouseY", s.invertMouseY ? 1 : 0);
    for (const GameInputBindingDef& binding : kGameInputBindings) {
        WriteIniIntValue(L"Controls", binding.iniKey, GameActionKey(s, binding.action));
    }
    WriteIniIntValue(L"Audio", L"Muted", s.audioMuted ? 1 : 0);
    WriteIniFloat(L"Audio", L"MasterVolume", s.audioMasterVolume);
    WriteIniFloat(L"Audio", L"MusicVolume", s.audioMusicVolume);
    WriteIniFloat(L"Audio", L"EffectsVolume", s.audioEffectsVolume);
    WriteIniFloat(L"Audio", L"AmbienceVolume", s.audioAmbienceVolume);
    WriteIniFloat(L"Audio", L"MonsterVolume", s.audioMonsterVolume);
}
