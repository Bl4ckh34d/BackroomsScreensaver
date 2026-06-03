#include "game_settings_panel_internal.h"

void ApplyGameSettingsSlider(GameSettingsPanelState* state, int id, int x) {
    if (!state) return;
    RECT track{322, 0, 592, 0};
    auto sliderValue = [&](float minValue, float maxValue) {
        return SliderValueFromX(track, x, minValue, maxValue);
    };
    Settings& s = state->settings;
    switch (id) {
    case kGameSettingsExposure:
        s.exposure = std::clamp(sliderValue(0.25f, 3.0f), 0.25f, 3.0f);
        break;
    case kGameSettingsBloom:
        s.bloomAmount = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    case kGameSettingsMotionBlur:
        s.motionBlurAmount = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    case kGameSettingsAirDensity:
        s.airParticleDensity = std::clamp(sliderValue(0.0f, 2.0f), 0.0f, 2.0f);
        break;
    case kGameSettingsMouseSensitivity:
        s.mouseSensitivity = std::clamp(sliderValue(0.2f, 3.0f), 0.2f, 3.0f);
        break;
    case kGameSettingsFrameRateLimit:
        s.gameFrameRateLimit = std::clamp(static_cast<int>(std::round(sliderValue(15.0f, 144.0f))), 15, 144);
        break;
    case kGameSettingsMasterVolume:
        s.audioMasterVolume = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    case kGameSettingsMusicVolume:
        s.audioMusicVolume = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    case kGameSettingsEffectsVolume:
        s.audioEffectsVolume = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    case kGameSettingsAmbienceVolume:
        s.audioAmbienceVolume = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    case kGameSettingsMonsterVolume:
        s.audioMonsterVolume = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
        break;
    default:
        break;
    }
}

void ToggleGameSetting(GameSettingsPanelState* state, int id) {
    if (!state) return;
    Settings& s = state->settings;
    switch (id) {
    case kGameSettingsFullscreen: s.gameFullscreen = !s.gameFullscreen; break;
    case kGameSettingsWarp: s.allowWarpFallback = !s.allowWarpFallback; break;
    case kGameSettingsInvertY: s.invertMouseY = !s.invertMouseY; break;
    case kGameSettingsMuted: s.audioMuted = !s.audioMuted; break;
    case kGameSettingsMonsterIgnorePlayer: s.monsterIgnorePlayer = !s.monsterIgnorePlayer; break;
    case kGameSettingsInfiniteStamina: s.debugInfiniteStamina = !s.debugInfiniteStamina; break;
    case kGameSettingsInvincible: s.debugInvincible = !s.debugInvincible; break;
    default: break;
    }
}

void NotifyGameSettingsKeyCapture(GameSettingsPanelState* state, bool active, bool escapeConsumed) {
    if (state && state->host.onKeyCaptureChanged) {
        state->host.onKeyCaptureChanged(state->host.context, active, escapeConsumed);
    }
}

void SaveAndCloseGameSettingsPanel(HWND hwnd, GameSettingsPanelState* state) {
    if (!state) return;
    SaveGameSettingsPanel(state);
    if (state->host.onSave) state->host.onSave(state->host.context, state->settings);
    DestroyWindow(hwnd);
}
