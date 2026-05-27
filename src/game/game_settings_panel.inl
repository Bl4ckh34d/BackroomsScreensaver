enum class GameSettingsControlKind {
    Tab,
    Button,
    Check,
    Slider,
    Dropdown,
    DropdownOption
};

struct GameSettingsHit {
    RECT rect{};
    int id = 0;
    GameSettingsControlKind kind = GameSettingsControlKind::Button;
};

struct GameSettingsPanelState {
    Settings settings;
    int tab = 0;
    int draggingSlider = 0;
    bool resolutionDropdownOpen = false;
    int capturingKeyAction = -1;
    std::vector<POINT> resolutionOptions;
    std::vector<GameSettingsHit> hits;
};

constexpr int kGameSettingsTabSystem = 100;
constexpr int kGameSettingsTabGraphics = 101;
constexpr int kGameSettingsTabGame = 102;
constexpr int kGameSettingsTabControls = 103;
constexpr int kGameSettingsTabAudio = 104;
constexpr int kGameSettingsSave = 200;
constexpr int kGameSettingsClose = 201;
constexpr int kGameSettingsFullscreen = 300;
constexpr int kGameSettingsWarp = 301;
constexpr int kGameSettingsInvertY = 302;
constexpr int kGameSettingsMuted = 303;
constexpr int kGameSettingsMonsterIgnorePlayer = 304;
constexpr int kGameSettingsInfiniteStamina = 305;
constexpr int kGameSettingsInvincible = 306;
constexpr int kGameSettingsResolutionDropdown = 400;
constexpr int kGameSettingsExposure = 402;
constexpr int kGameSettingsBloom = 403;
constexpr int kGameSettingsMotionBlur = 404;
constexpr int kGameSettingsAirDensity = 405;
constexpr int kGameSettingsMouseSensitivity = 406;
constexpr int kGameSettingsMasterVolume = 407;
constexpr int kGameSettingsEffectsVolume = 408;
constexpr int kGameSettingsAmbienceVolume = 409;
constexpr int kGameSettingsMonsterVolume = 410;
constexpr int kGameSettingsResolutionOptionBase = 800;
constexpr int kGameSettingsKeybindBase = 900;

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
    WriteIniFloat(L"Audio", L"EffectsVolume", s.audioEffectsVolume);
    WriteIniFloat(L"Audio", L"AmbienceVolume", s.audioAmbienceVolume);
    WriteIniFloat(L"Audio", L"MonsterVolume", s.audioMonsterVolume);
}

bool PtInRectInclusive(const RECT& rc, POINT p) {
    return p.x >= rc.left && p.x <= rc.right && p.y >= rc.top && p.y <= rc.bottom;
}

void FillSolid(HDC dc, const RECT& rc, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(dc, &rc, brush);
    DeleteObject(brush);
}

void DrawTextLine(HDC dc, const std::wstring& text, RECT rc, COLORREF color, UINT format = DT_LEFT | DT_VCENTER | DT_SINGLELINE) {
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    DrawTextW(dc, text.c_str(), -1, &rc, format);
}

void AddGameSettingsHit(GameSettingsPanelState* state, RECT rc, int id, GameSettingsControlKind kind) {
    if (state) state->hits.push_back({rc, id, kind});
}

int FindGameSettingsHitId(GameSettingsPanelState* state, POINT p) {
    if (!state) return 0;
    for (auto it = state->hits.rbegin(); it != state->hits.rend(); ++it) {
        if (PtInRectInclusive(it->rect, p)) return it->id;
    }
    return 0;
}

void DrawGameSettingsButton(HDC dc, GameSettingsPanelState* state, RECT rc, int id, const wchar_t* label, bool active = false) {
    FillSolid(dc, rc, active ? RGB(126, 96, 48) : RGB(46, 43, 37));
    FrameRect(dc, &rc, GetSysColorBrush(COLOR_GRAYTEXT));
    RECT textRc = rc;
    InflateRect(&textRc, -10, 0);
    DrawTextLine(dc, label, textRc, RGB(235, 229, 210), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddGameSettingsHit(state, rc, id, GameSettingsControlKind::Button);
}

void DrawGameSettingsCheck(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label, bool checked) {
    RECT box{x, y, x + 18, y + 18};
    FillSolid(dc, box, RGB(25, 24, 21));
    FrameRect(dc, &box, GetSysColorBrush(COLOR_GRAYTEXT));
    if (checked) {
        HPEN pen = CreatePen(PS_SOLID, 3, RGB(220, 176, 84));
        HGDIOBJ old = SelectObject(dc, pen);
        MoveToEx(dc, x + 4, y + 9, nullptr);
        LineTo(dc, x + 8, y + 14);
        LineTo(dc, x + 15, y + 4);
        SelectObject(dc, old);
        DeleteObject(pen);
    }
    RECT labelRc{x + 28, y - 3, x + 280, y + 23};
    DrawTextLine(dc, label, labelRc, RGB(226, 221, 205));
    RECT hit{x, y - 6, x + 300, y + 26};
    AddGameSettingsHit(state, hit, id, GameSettingsControlKind::Check);
}

float SliderValueFromX(const RECT& track, int x, float minValue, float maxValue) {
    float t = std::clamp(static_cast<float>(x - track.left) / static_cast<float>(std::max<LONG>(1, track.right - track.left)), 0.0f, 1.0f);
    return Lerp(minValue, maxValue, t);
}

void DrawGameSettingsSlider(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label,
    float value, float minValue, float maxValue, const std::wstring& displayValue) {
    RECT labelRc{x, y, x + 230, y + 24};
    DrawTextLine(dc, label, labelRc, RGB(226, 221, 205));
    RECT track{x + 250, y + 8, x + 520, y + 14};
    FillSolid(dc, track, RGB(38, 36, 31));
    float t = Clamp01((value - minValue) / std::max(0.0001f, maxValue - minValue));
    int knobX = track.left + static_cast<int>(std::round(t * (track.right - track.left)));
    RECT fill{track.left, track.top, knobX, track.bottom};
    FillSolid(dc, fill, RGB(172, 126, 48));
    RECT knob{knobX - 6, y + 2, knobX + 6, y + 20};
    FillSolid(dc, knob, RGB(218, 188, 118));
    RECT valueRc{x + 536, y, x + 620, y + 24};
    DrawTextLine(dc, displayValue, valueRc, RGB(226, 221, 205), DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    RECT hit{track.left - 8, y - 6, track.right + 8, y + 28};
    AddGameSettingsHit(state, hit, id, GameSettingsControlKind::Slider);
}

void DrawGameSettingsDropdown(HDC dc, GameSettingsPanelState* state, int x, int y, int id, const wchar_t* label,
    const std::wstring& value) {
    RECT labelRc{x, y, x + 230, y + 28};
    DrawTextLine(dc, label, labelRc, RGB(226, 221, 205));
    RECT box{x + 250, y, x + 520, y + 30};
    FillSolid(dc, box, RGB(32, 30, 26));
    FrameRect(dc, &box, GetSysColorBrush(COLOR_GRAYTEXT));
    RECT valueRc{box.left + 12, box.top, box.right - 36, box.bottom};
    DrawTextLine(dc, value, valueRc, RGB(235, 229, 210));
    RECT arrow{box.right - 28, box.top + 7, box.right - 10, box.bottom - 7};
    DrawTextLine(dc, L"v", arrow, RGB(220, 176, 84), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddGameSettingsHit(state, box, id, GameSettingsControlKind::Dropdown);

    if (state && state->resolutionDropdownOpen) {
        int maxVisible = std::min<int>(static_cast<int>(state->resolutionOptions.size()), 8);
        for (int i = 0; i < maxVisible; ++i) {
            RECT option{box.left, box.bottom + i * 28, box.right, box.bottom + (i + 1) * 28};
            POINT res = state->resolutionOptions[static_cast<size_t>(i)];
            bool selected = res.x == state->settings.gameResolutionWidth && res.y == state->settings.gameResolutionHeight;
            FillSolid(dc, option, selected ? RGB(126, 96, 48) : RGB(36, 34, 29));
            FrameRect(dc, &option, GetSysColorBrush(COLOR_GRAYTEXT));
            RECT textRc{option.left + 12, option.top, option.right - 12, option.bottom};
            DrawTextLine(dc, FormatResolution(res.x, res.y), textRc, RGB(235, 229, 210));
            AddGameSettingsHit(state, option, kGameSettingsResolutionOptionBase + i, GameSettingsControlKind::DropdownOption);
        }
    }
}

void DrawGameSettingsKeybind(HDC dc, GameSettingsPanelState* state, int x, int y, int actionIndex) {
    if (!state || actionIndex < 0 || actionIndex >= kGameInputActionCount) return;
    const GameInputBindingDef& binding = kGameInputBindings[static_cast<size_t>(actionIndex)];
    RECT labelRc{x, y, x + 230, y + 30};
    DrawTextLine(dc, binding.label, labelRc, RGB(226, 221, 205));

    RECT button{x + 250, y, x + 430, y + 30};
    bool capturing = state->capturingKeyAction == actionIndex;
    FillSolid(dc, button, capturing ? RGB(132, 70, 48) : RGB(46, 43, 37));
    FrameRect(dc, &button, GetSysColorBrush(COLOR_GRAYTEXT));
    RECT textRc = button;
    InflateRect(&textRc, -10, 0);
    std::wstring text = capturing
        ? L"Press a key..."
        : KeyDisplayName(GameActionKey(state->settings, binding.action));
    DrawTextLine(dc, text, textRc, RGB(235, 229, 210), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    AddGameSettingsHit(state, button, kGameSettingsKeybindBase + actionIndex, GameSettingsControlKind::Button);
}

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
    case kGameSettingsMasterVolume:
        s.audioMasterVolume = std::clamp(sliderValue(0.0f, 1.0f), 0.0f, 1.0f);
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

void ApplyGameWindowSettings(const Settings& settings) {
    if (!gApp || !gApp->gameShell || !gApp->hwnd) return;
    HWND hwnd = gApp->hwnd;
    if (settings.gameFullscreen) {
        SetWindowLongPtrW(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        SetWindowPos(hwnd, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    } else {
        DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        RECT wr{0, 0, settings.gameResolutionWidth, settings.gameResolutionHeight};
        AdjustWindowRect(&wr, style, FALSE);
        int w = wr.right - wr.left;
        int h = wr.bottom - wr.top;
        int x = std::max(0, (GetSystemMetrics(SM_CXSCREEN) - w) / 2);
        int y = std::max(0, (GetSystemMetrics(SM_CYSCREEN) - h) / 2);
        SetWindowLongPtrW(hwnd, GWL_STYLE, style);
        SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, w, h, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}

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
        DrawGameSettingsCheck(dc, state, x, y, kGameSettingsWarp, L"Allow WARP fallback", s.allowWarpFallback);
        RECT note{x, y + 52, panel.right - 34, y + 88};
        DrawTextLine(dc, L"Save applies fullscreen and window size immediately, then returns to the menu.", note, RGB(174, 166, 142), DT_LEFT | DT_WORDBREAK);
    } else if (state->tab == 1) {
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

LRESULT CALLBACK GameSettingsPanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<GameSettingsPanelState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_CREATE:
        state = new GameSettingsPanelState();
        state->settings = LoadSettings();
        BuildGameResolutionOptions(state);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
        return 0;
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        PaintGameSettingsPanel(hwnd, dc, state);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        if (!state) break;
        HDC hitDc = GetDC(hwnd);
        if (hitDc) {
            PaintGameSettingsPanel(hwnd, hitDc, state);
            ReleaseDC(hwnd, hitDc);
        }
        POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        RECT client{};
        GetClientRect(hwnd, &client);
        RECT saveRect{client.right - 300, client.bottom - 58, client.right - 178, client.bottom - 24};
        RECT closeRect{client.right - 160, client.bottom - 58, client.right - 38, client.bottom - 24};
        if (PtInRectInclusive(saveRect, p)) {
            SaveGameSettingsPanel(state);
            if (gApp) gApp->gameInputSettings = state->settings;
            if (gApp && gApp->rendererInitialized) gApp->renderer.ApplyGameSettings(state->settings);
            ApplyGameWindowSettings(state->settings);
            DestroyWindow(hwnd);
            return 0;
        }
        if (PtInRectInclusive(closeRect, p)) {
            DestroyWindow(hwnd);
            return 0;
        }
        int directId = FindGameSettingsHitId(state, p);
        if (directId == kGameSettingsSave) {
            SaveGameSettingsPanel(state);
            if (gApp) gApp->gameInputSettings = state->settings;
            if (gApp && gApp->rendererInitialized) gApp->renderer.ApplyGameSettings(state->settings);
            ApplyGameWindowSettings(state->settings);
            DestroyWindow(hwnd);
            return 0;
        }
        if (directId == kGameSettingsClose) {
            DestroyWindow(hwnd);
            return 0;
        }
        for (const auto& hit : state->hits) {
            if (!PtInRectInclusive(hit.rect, p)) continue;
            if (hit.kind == GameSettingsControlKind::Tab ||
                (hit.id >= kGameSettingsTabSystem && hit.id <= kGameSettingsTabAudio)) {
                state->tab = std::clamp(hit.id - kGameSettingsTabSystem, 0, 4);
                state->resolutionDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Check) {
                ToggleGameSetting(state, hit.id);
                state->resolutionDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Slider) {
                state->draggingSlider = hit.id;
                SetCapture(hwnd);
                ApplyGameSettingsSlider(state, hit.id, p.x);
                state->resolutionDropdownOpen = false;
            } else if (hit.kind == GameSettingsControlKind::Dropdown && hit.id == kGameSettingsResolutionDropdown) {
                state->resolutionDropdownOpen = !state->resolutionDropdownOpen;
            } else if (hit.kind == GameSettingsControlKind::DropdownOption) {
                int index = hit.id - kGameSettingsResolutionOptionBase;
                if (index >= 0 && index < static_cast<int>(state->resolutionOptions.size())) {
                    POINT res = state->resolutionOptions[static_cast<size_t>(index)];
                    state->settings.gameResolutionWidth = res.x;
                    state->settings.gameResolutionHeight = res.y;
                }
                state->resolutionDropdownOpen = false;
            } else if (hit.id >= kGameSettingsKeybindBase &&
                hit.id < kGameSettingsKeybindBase + kGameInputActionCount) {
                state->capturingKeyAction = hit.id - kGameSettingsKeybindBase;
                state->resolutionDropdownOpen = false;
                if (gApp) {
                    gApp->gameSettingsKeyCaptureActive = true;
                    gApp->gameSettingsEscapeConsumed = false;
                }
                SetFocus(hwnd);
            } else if (hit.id == kGameSettingsSave) {
                SaveGameSettingsPanel(state);
                if (gApp) gApp->gameInputSettings = state->settings;
                if (gApp && gApp->rendererInitialized) gApp->renderer.ApplyGameSettings(state->settings);
                ApplyGameWindowSettings(state->settings);
                DestroyWindow(hwnd);
                return 0;
            } else if (hit.id == kGameSettingsClose) {
                DestroyWindow(hwnd);
                return 0;
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        state->resolutionDropdownOpen = false;
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (state && state->capturingKeyAction >= 0) {
            if (wParam == VK_ESCAPE) {
                state->capturingKeyAction = -1;
                if (gApp) {
                    gApp->gameSettingsKeyCaptureActive = false;
                    gApp->gameSettingsEscapeConsumed = true;
                }
            } else if (wParam > 0 && wParam < 256) {
                AssignGameActionKey(state->settings,
                    static_cast<GameInputAction>(state->capturingKeyAction),
                    static_cast<int>(wParam));
                state->capturingKeyAction = -1;
                if (gApp) {
                    gApp->gameSettingsKeyCaptureActive = false;
                    gApp->gameSettingsEscapeConsumed = false;
                }
            }
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        break;
    case WM_MOUSEMOVE:
        if (state && state->draggingSlider != 0 && (wParam & MK_LBUTTON)) {
            ApplyGameSettingsSlider(state, state->draggingSlider, GET_X_LPARAM(lParam));
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        break;
    case WM_LBUTTONUP:
        if (state && state->draggingSlider != 0) {
            state->draggingSlider = 0;
            if (GetCapture() == hwnd) ReleaseCapture();
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        if (gApp) {
            gApp->gameSettingsKeyCaptureActive = false;
            gApp->gameSettingsEscapeConsumed = false;
        }
        delete state;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        if (gApp && gApp->gameConfig == hwnd) {
            gApp->gameConfig = nullptr;
            PostMessageW(gApp->hwnd, kGameConfigClosedMessage, 0, 0);
        }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

HWND CreateGameSettingsPanel(HWND parent) {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"BackroomsMazeGameSettingsPanel";
    WNDCLASSW wc{};
    wc.lpfnWndProc = GameSettingsPanelWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    return CreateWindowExW(0, cls, L"", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 10, 10, parent, nullptr, hInstance, nullptr);
}
