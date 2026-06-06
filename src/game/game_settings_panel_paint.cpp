#include "game_settings_panel_internal.h"

void PaintGameSettingsPanel(HWND hwnd, HDC dc, GameSettingsPanelState* state) {
    if (!state) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    state->hits.clear();
    FillSolid(dc, rc, RGB(16, 15, 13));

    HFONT titleFont = CreateFontW(26, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT bodyFont = CreateFontW(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(dc, titleFont);
    RECT title{38, 26, rc.right - 38, 62};
    DrawTextLine(dc, L"Settings", title, RGB(238, 229, 204));
    SelectObject(dc, bodyFont);

    const wchar_t* tabs[] = {L"System", L"Graphics", L"Game", L"Controls", L"Audio"};
    int tabX = 38;
    for (int i = 0; i < 5; ++i) {
        RECT tabRc{tabX, 78, tabX + 118, 116};
        DrawGameSettingsButton(dc, state, tabRc, kGameSettingsTabSystem + i, tabs[i], state->tab == i);
        AddGameSettingsHit(state, tabRc, kGameSettingsTabSystem + i, GameSettingsControlKind::Tab);
        tabX += 126;
    }

    RECT panel{38, 134, rc.right - 38, rc.bottom - 88};
    FillSolid(dc, panel, RGB(26, 24, 21));
    FrameRect(dc, &panel, GetSysColorBrush(COLOR_GRAYTEXT));

    int x = panel.left + 34;
    int y = panel.top + 30;
    Settings& s = state->settings;
    if (state->tab == 0) {
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsFullscreen, L"Fullscreen", s.gameFullscreen); y += 44;
        DrawGameSettingsDropdown(dc, state, x, y, kGameSettingsResolutionDropdown, L"Window resolution",
            FormatResolution(s.gameResolutionWidth, s.gameResolutionHeight)); y += state->resolutionDropdownOpen ? 282 : 50;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsFrameRateLimit, L"Frame rate limit", static_cast<float>(s.gameFrameRateLimit), 15.0f, 144.0f, FormatSettingValue(s.gameFrameRateLimit)); y += 42;
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsWarp, L"Allow WARP fallback", s.allowWarpFallback);
        RECT note{x, y + 52, panel.right - 34, y + 88};
        DrawTextLine(dc, L"Save applies fullscreen and window size immediately, then returns to the menu.", note, RGB(174, 166, 142), DT_LEFT | DT_WORDBREAK);
    } else if (state->tab == 1) {
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsRenderScale, L"Render scale", static_cast<float>(s.renderScalePercent), 50.0f, 100.0f, FormatSettingValue(s.renderScalePercent) + L"%"); y += 42;
        DrawGameSettingsDropdown(dc, state, x, y, kGameSettingsAntiAliasingDropdown, L"Anti-aliasing",
            FormatAntiAliasing(s.antiAliasing)); y += state->antiAliasingDropdownOpen ? 174 : 50;
        DrawGameSettingsDropdown(dc, state, x, y, kGameSettingsAnisotropyDropdown, L"Texture anisotropy",
            FormatTextureAnisotropy(s.textureAnisotropy)); y += state->anisotropyDropdownOpen ? 174 : 50;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsExposure, L"Exposure", s.exposure, 0.25f, 3.0f, FormatSettingValue(s.exposure)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsBloom, L"Bloom", s.bloomAmount, 0.0f, 1.0f, FormatSettingValue(s.bloomAmount)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsMotionBlur, L"Motion blur", s.motionBlurAmount, 0.0f, 1.0f, FormatSettingValue(s.motionBlurAmount)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsAirDensity, L"Air mote density", s.airParticleDensity, 0.0f, 2.0f, FormatSettingValue(s.airParticleDensity));
    } else if (state->tab == 2) {
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsMonsterIgnorePlayer, L"Monster ignores player", s.monsterIgnorePlayer); y += 48;
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsInfiniteStamina, L"Infinite stamina", s.debugInfiniteStamina); y += 48;
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsInvincible, L"Invincible", s.debugInvincible); y += 48;
        RECT text{x, y + 8, panel.right - 34, y + 96};
        DrawTextLine(dc, L"Debug cheats are for gameplay testing. Monster ignore disables hearing, chase, panic-lock, and kills, but the face can still look at the player when it has line of sight.", text, RGB(174, 166, 142), DT_LEFT | DT_WORDBREAK);
    } else if (state->tab == 3) {
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsMouseSensitivity, L"Mouse sensitivity", s.mouseSensitivity, 0.2f, 3.0f, FormatSettingValue(s.mouseSensitivity)); y += 48;
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsInvertY, L"Invert Y axis", s.invertMouseY); y += 48;
        for (int i = 0; i < kGameInputActionCount; ++i) {
            DrawGameSettingsKeybind(dc, state, x, y, i);
            y += 36;
        }
        RECT text{x, y + 8, panel.right - 34, y + 48};
        DrawTextLine(dc, L"Click an action, then press and hold the new key. Escape cancels the capture and keeps the previous key.", text, RGB(174, 166, 142), DT_LEFT | DT_WORDBREAK);
    } else {
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsMuted, L"Mute audio", s.audioMuted); y += 48;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsMasterVolume, L"Master volume", s.audioMasterVolume, 0.0f, 1.0f, FormatSettingValue(s.audioMasterVolume)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsMusicVolume, L"Music volume", s.audioMusicVolume, 0.0f, 1.0f, FormatSettingValue(s.audioMusicVolume)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsEffectsVolume, L"Effects volume", s.audioEffectsVolume, 0.0f, 1.0f, FormatSettingValue(s.audioEffectsVolume)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsAmbienceVolume, L"Ambience volume", s.audioAmbienceVolume, 0.0f, 1.0f, FormatSettingValue(s.audioAmbienceVolume)); y += 42;
        DrawGameSettingsSlider(dc, state, x, y, kGameSettingsMonsterVolume, L"Monster volume", s.audioMonsterVolume, 0.0f, 1.0f, FormatSettingValue(s.audioMonsterVolume));
    }

    RECT save{rc.right - 300, rc.bottom - 58, rc.right - 178, rc.bottom - 24};
    RECT close{rc.right - 160, rc.bottom - 58, rc.right - 38, rc.bottom - 24};
    DrawGameSettingsButton(dc, state, save, kGameSettingsSave, L"Save");
    DrawGameSettingsButton(dc, state, close, kGameSettingsClose, L"Close");

    SelectObject(dc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(bodyFont);
}
