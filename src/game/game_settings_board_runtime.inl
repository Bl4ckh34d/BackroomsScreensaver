// In-world settings board state, hit testing, and command handling.

void PushSettingsBoardStateToRenderer(HWND hwnd) {
    if (!gApp || !gApp->gameSettingsBoardOpen || !hwnd) return;
    if (!gApp->rendererInitialized) return;
    int hover = gApp->gameMenuHasMouse ? HitTestSettingsBoard(hwnd, gApp->gameMenuMouse) : 0;
    gApp->renderer.SetSettingsBoardState(gApp->gameSettingsBoardSettings, hover, gApp->gameSettingsBoardTab,
        gApp->gameSettingsBoardCaptureAction);
}

void EnterGameSettingsBoard(HWND hwnd) {
    if (!gApp || !gApp->gameShell || !hwnd) return;
    ReleaseGameMouse();
    gApp->gameCustomMenuOpen = false;
    gApp->gameSettingsBoardOpen = true;
    gApp->gameSettingsBoardTab = 1;
    gApp->gameSettingsBoardCaptureAction = -1;
    gApp->gameSettingsBoardSettings = gApp->gameInputSettings;
    gApp->gameMenuHoverId = 0;
    gApp->gameMenuHasMouse = false;
    SetCustomGameControlsVisible(false);
    if (gApp->rendererInitialized) {
        gApp->renderer.SetMainMenuSettingsBoardView(true);
        gApp->renderer.SetSettingsBoardState(gApp->gameSettingsBoardSettings, 0, gApp->gameSettingsBoardTab,
            gApp->gameSettingsBoardCaptureAction);
    }
    SetWindowTextW(hwnd, L"Backrooms Maze - Settings");
    InvalidateRect(hwnd, nullptr, FALSE);
}

void ExitGameSettingsBoard(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    if (!gApp->gameSettingsBoardOpen) return;
    gApp->gameSettingsBoardOpen = false;
    gApp->gameSettingsBoardTab = 1;
    gApp->gameSettingsBoardCaptureAction = -1;
    gApp->gameSettingsKeyCaptureActive = false;
    gApp->gameSettingsEscapeConsumed = false;
    gApp->gameMenuHoverId = 0;
    if (gApp->rendererInitialized) {
        gApp->renderer.SetMainMenuSettingsBoardView(false);
    }
    SetWindowTextW(hwnd, L"Backrooms Maze");
    InvalidateRect(hwnd, nullptr, FALSE);
}

bool SettingsBoardControlActiveOnTab(SettingsBoardControl control, int tab) {
    switch (control) {
    case SettingsBoardControl::TabSystem:
    case SettingsBoardControl::TabGraphics:
    case SettingsBoardControl::TabGame:
    case SettingsBoardControl::TabControls:
    case SettingsBoardControl::TabAudio:
    case SettingsBoardControl::Save:
    case SettingsBoardControl::Back:
        return true;
    case SettingsBoardControl::Fullscreen:
    case SettingsBoardControl::ResolutionMinus:
    case SettingsBoardControl::ResolutionPlus:
    case SettingsBoardControl::FrameRateMinus:
    case SettingsBoardControl::FrameRatePlus:
    case SettingsBoardControl::Warp:
        return tab == 0;
    case SettingsBoardControl::RenderScaleMinus:
    case SettingsBoardControl::RenderScalePlus:
    case SettingsBoardControl::AntiAliasingMinus:
    case SettingsBoardControl::AntiAliasingPlus:
    case SettingsBoardControl::AnisotropyMinus:
    case SettingsBoardControl::AnisotropyPlus:
    case SettingsBoardControl::ExposureMinus:
    case SettingsBoardControl::ExposurePlus:
    case SettingsBoardControl::BloomMinus:
    case SettingsBoardControl::BloomPlus:
    case SettingsBoardControl::MotionBlurMinus:
    case SettingsBoardControl::MotionBlurPlus:
    case SettingsBoardControl::AirDensityMinus:
    case SettingsBoardControl::AirDensityPlus:
        return tab == 1;
    case SettingsBoardControl::MonsterIgnorePlayer:
    case SettingsBoardControl::InfiniteStamina:
    case SettingsBoardControl::Invincible:
        return tab == 2;
    case SettingsBoardControl::MouseSensitivityMinus:
    case SettingsBoardControl::MouseSensitivityPlus:
    case SettingsBoardControl::InvertMouseY:
    case SettingsBoardControl::KeyMoveForward:
    case SettingsBoardControl::KeyMoveBackward:
    case SettingsBoardControl::KeyMoveLeft:
    case SettingsBoardControl::KeyMoveRight:
    case SettingsBoardControl::KeySprint:
    case SettingsBoardControl::KeyCrouch:
    case SettingsBoardControl::KeyInteract:
    case SettingsBoardControl::KeyFlashlight:
    case SettingsBoardControl::KeyPause:
        return tab == 3;
    case SettingsBoardControl::MuteAudio:
    case SettingsBoardControl::MasterMinus:
    case SettingsBoardControl::MasterPlus:
    case SettingsBoardControl::MusicMinus:
    case SettingsBoardControl::MusicPlus:
    case SettingsBoardControl::EffectsMinus:
    case SettingsBoardControl::EffectsPlus:
    case SettingsBoardControl::AmbienceMinus:
    case SettingsBoardControl::AmbiencePlus:
    case SettingsBoardControl::MonsterMinus:
    case SettingsBoardControl::MonsterPlus:
        return tab == 4;
    default:
        return false;
    }
}

int HitTestSettingsBoard(HWND hwnd, POINT p) {
    if (!gApp || !gApp->rendererInitialized || !gApp->gameSettingsBoardOpen || hwnd != gApp->hwnd) return 0;
    const SettingsBoardControl controls[] = {
        SettingsBoardControl::TabSystem,
        SettingsBoardControl::TabGraphics,
        SettingsBoardControl::TabGame,
        SettingsBoardControl::TabControls,
        SettingsBoardControl::TabAudio,
        SettingsBoardControl::Fullscreen,
        SettingsBoardControl::ResolutionMinus,
        SettingsBoardControl::ResolutionPlus,
        SettingsBoardControl::FrameRateMinus,
        SettingsBoardControl::FrameRatePlus,
        SettingsBoardControl::Warp,
        SettingsBoardControl::RenderScaleMinus,
        SettingsBoardControl::RenderScalePlus,
        SettingsBoardControl::AntiAliasingMinus,
        SettingsBoardControl::AntiAliasingPlus,
        SettingsBoardControl::AnisotropyMinus,
        SettingsBoardControl::AnisotropyPlus,
        SettingsBoardControl::ExposureMinus,
        SettingsBoardControl::ExposurePlus,
        SettingsBoardControl::BloomMinus,
        SettingsBoardControl::BloomPlus,
        SettingsBoardControl::MotionBlurMinus,
        SettingsBoardControl::MotionBlurPlus,
        SettingsBoardControl::AirDensityMinus,
        SettingsBoardControl::AirDensityPlus,
        SettingsBoardControl::MonsterIgnorePlayer,
        SettingsBoardControl::InfiniteStamina,
        SettingsBoardControl::Invincible,
        SettingsBoardControl::MouseSensitivityMinus,
        SettingsBoardControl::MouseSensitivityPlus,
        SettingsBoardControl::InvertMouseY,
        SettingsBoardControl::KeyMoveForward,
        SettingsBoardControl::KeyMoveBackward,
        SettingsBoardControl::KeyMoveLeft,
        SettingsBoardControl::KeyMoveRight,
        SettingsBoardControl::KeySprint,
        SettingsBoardControl::KeyCrouch,
        SettingsBoardControl::KeyInteract,
        SettingsBoardControl::KeyFlashlight,
        SettingsBoardControl::KeyPause,
        SettingsBoardControl::MuteAudio,
        SettingsBoardControl::MasterMinus,
        SettingsBoardControl::MasterPlus,
        SettingsBoardControl::MusicMinus,
        SettingsBoardControl::MusicPlus,
        SettingsBoardControl::EffectsMinus,
        SettingsBoardControl::EffectsPlus,
        SettingsBoardControl::AmbienceMinus,
        SettingsBoardControl::AmbiencePlus,
        SettingsBoardControl::MonsterMinus,
        SettingsBoardControl::MonsterPlus,
        SettingsBoardControl::Save,
        SettingsBoardControl::Back
    };
    int tab = std::clamp(gApp->gameSettingsBoardTab, 0, 4);
    for (SettingsBoardControl control : controls) {
        if (!SettingsBoardControlActiveOnTab(control, tab)) continue;
        RECT rc{};
        int id = static_cast<int>(control);
        if (!gApp->renderer.SettingsBoardControlScreenRect(id, rc)) continue;
        if (PtInRect(&rc, p)) return id;
    }
    return 0;
}

void AdjustSettingsBoardInt(int& value, int delta, int minValue, int maxValue) {
    value = std::clamp(value + delta, minValue, maxValue);
}

void AdjustSettingsBoardFloat(float& value, float delta, float minValue, float maxValue) {
    value = std::clamp(value + delta, minValue, maxValue);
}

void StepSettingsBoardAntiAliasing(int& value, int direction) {
    constexpr int values[] = {0, 1, 2, 4, 8, 16};
    value = NormalizeAntiAliasingMode(value);
    int index = 1;
    for (int i = 0; i < static_cast<int>(std::size(values)); ++i) {
        if (values[i] == value) {
            index = i;
            break;
        }
    }
    index = std::clamp(index + direction, 0, static_cast<int>(std::size(values)) - 1);
    value = values[index];
}

void StepSettingsBoardAnisotropy(int& value, int direction) {
    constexpr int values[] = {1, 2, 4, 8, 16};
    value = NormalizeTextureAnisotropy(value);
    int index = 3;
    for (int i = 0; i < static_cast<int>(std::size(values)); ++i) {
        if (values[i] == value) {
            index = i;
            break;
        }
    }
    index = std::clamp(index + direction, 0, static_cast<int>(std::size(values)) - 1);
    value = values[index];
}

std::vector<POINT> SettingsBoardResolutionOptions(const Settings& settings) {
    std::vector<POINT> options;
    auto add = [&](int width, int height) {
        if (width < 640 || height < 360) return;
        POINT p{width, height};
        auto it = std::find_if(options.begin(), options.end(),
            [&](const POINT& existing) { return existing.x == p.x && existing.y == p.y; });
        if (it == options.end()) options.push_back(p);
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
    add(settings.gameResolutionWidth, settings.gameResolutionHeight);
    std::sort(options.begin(), options.end(), [](const POINT& a, const POINT& b) {
        if (a.x != b.x) return a.x > b.x;
        return a.y > b.y;
    });
    return options;
}

void StepSettingsBoardResolution(Settings& settings, int direction) {
    std::vector<POINT> options = SettingsBoardResolutionOptions(settings);
    if (options.empty()) return;
    int index = 0;
    for (int i = 0; i < static_cast<int>(options.size()); ++i) {
        if (options[static_cast<size_t>(i)].x == settings.gameResolutionWidth &&
            options[static_cast<size_t>(i)].y == settings.gameResolutionHeight) {
            index = i;
            break;
        }
    }
    index = std::clamp(index + direction, 0, static_cast<int>(options.size()) - 1);
    settings.gameResolutionWidth = options[static_cast<size_t>(index)].x;
    settings.gameResolutionHeight = options[static_cast<size_t>(index)].y;
}

int SettingsBoardKeyActionIndex(SettingsBoardControl control) {
    switch (control) {
    case SettingsBoardControl::KeyMoveForward: return static_cast<int>(GameInputAction::MoveForward);
    case SettingsBoardControl::KeyMoveBackward: return static_cast<int>(GameInputAction::MoveBackward);
    case SettingsBoardControl::KeyMoveLeft: return static_cast<int>(GameInputAction::MoveLeft);
    case SettingsBoardControl::KeyMoveRight: return static_cast<int>(GameInputAction::MoveRight);
    case SettingsBoardControl::KeySprint: return static_cast<int>(GameInputAction::Sprint);
    case SettingsBoardControl::KeyCrouch: return static_cast<int>(GameInputAction::Crouch);
    case SettingsBoardControl::KeyInteract: return static_cast<int>(GameInputAction::Interact);
    case SettingsBoardControl::KeyFlashlight: return static_cast<int>(GameInputAction::Flashlight);
    case SettingsBoardControl::KeyPause: return static_cast<int>(GameInputAction::Pause);
    default: return -1;
    }
}

void SaveSettingsBoard(HWND hwnd) {
    if (!gApp) return;
    SaveSettingsToIni(gApp->gameSettingsBoardSettings);
    gApp->gameInputSettings = gApp->gameSettingsBoardSettings;
    if (gApp->rendererInitialized) gApp->renderer.ApplyGameSettings(gApp->gameSettingsBoardSettings);
    ApplyGameWindowSettings(gApp->hwnd, gApp->gameSettingsBoardSettings);
    ExitGameSettingsBoard(hwnd);
}

void ActivateSettingsBoardCommand(HWND hwnd, int control) {
    if (!gApp || !gApp->gameSettingsBoardOpen) return;
    Settings& s = gApp->gameSettingsBoardSettings;
    switch (static_cast<SettingsBoardControl>(control)) {
    case SettingsBoardControl::TabSystem: gApp->gameSettingsBoardTab = 0; break;
    case SettingsBoardControl::TabGraphics: gApp->gameSettingsBoardTab = 1; break;
    case SettingsBoardControl::TabGame: gApp->gameSettingsBoardTab = 2; break;
    case SettingsBoardControl::TabControls: gApp->gameSettingsBoardTab = 3; break;
    case SettingsBoardControl::TabAudio: gApp->gameSettingsBoardTab = 4; break;
    case SettingsBoardControl::Fullscreen: s.gameFullscreen = !s.gameFullscreen; break;
    case SettingsBoardControl::ResolutionMinus: StepSettingsBoardResolution(s, -1); break;
    case SettingsBoardControl::ResolutionPlus: StepSettingsBoardResolution(s, 1); break;
    case SettingsBoardControl::FrameRateMinus: AdjustSettingsBoardInt(s.gameFrameRateLimit, -5, 15, 144); break;
    case SettingsBoardControl::FrameRatePlus: AdjustSettingsBoardInt(s.gameFrameRateLimit, 5, 15, 144); break;
    case SettingsBoardControl::Warp: s.allowWarpFallback = !s.allowWarpFallback; break;
    case SettingsBoardControl::RenderScaleMinus: AdjustSettingsBoardInt(s.renderScalePercent, -5, 50, 100); break;
    case SettingsBoardControl::RenderScalePlus: AdjustSettingsBoardInt(s.renderScalePercent, 5, 50, 100); break;
    case SettingsBoardControl::AntiAliasingMinus: StepSettingsBoardAntiAliasing(s.antiAliasing, -1); s.fxaaEnabled = AntiAliasingUsesFxaa(s.antiAliasing); break;
    case SettingsBoardControl::AntiAliasingPlus: StepSettingsBoardAntiAliasing(s.antiAliasing, 1); s.fxaaEnabled = AntiAliasingUsesFxaa(s.antiAliasing); break;
    case SettingsBoardControl::AnisotropyMinus: StepSettingsBoardAnisotropy(s.textureAnisotropy, -1); break;
    case SettingsBoardControl::AnisotropyPlus: StepSettingsBoardAnisotropy(s.textureAnisotropy, 1); break;
    case SettingsBoardControl::ExposureMinus: AdjustSettingsBoardFloat(s.exposure, -0.05f, 0.25f, 3.0f); break;
    case SettingsBoardControl::ExposurePlus: AdjustSettingsBoardFloat(s.exposure, 0.05f, 0.25f, 3.0f); break;
    case SettingsBoardControl::BloomMinus: AdjustSettingsBoardFloat(s.bloomAmount, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::BloomPlus: AdjustSettingsBoardFloat(s.bloomAmount, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MotionBlurMinus: AdjustSettingsBoardFloat(s.motionBlurAmount, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MotionBlurPlus: AdjustSettingsBoardFloat(s.motionBlurAmount, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::AirDensityMinus: AdjustSettingsBoardFloat(s.airParticleDensity, -0.05f, 0.0f, 2.0f); break;
    case SettingsBoardControl::AirDensityPlus: AdjustSettingsBoardFloat(s.airParticleDensity, 0.05f, 0.0f, 2.0f); break;
    case SettingsBoardControl::MonsterIgnorePlayer: s.monsterIgnorePlayer = !s.monsterIgnorePlayer; break;
    case SettingsBoardControl::InfiniteStamina: s.debugInfiniteStamina = !s.debugInfiniteStamina; break;
    case SettingsBoardControl::Invincible: s.debugInvincible = !s.debugInvincible; break;
    case SettingsBoardControl::MouseSensitivityMinus: AdjustSettingsBoardFloat(s.mouseSensitivity, -0.05f, 0.2f, 3.0f); break;
    case SettingsBoardControl::MouseSensitivityPlus: AdjustSettingsBoardFloat(s.mouseSensitivity, 0.05f, 0.2f, 3.0f); break;
    case SettingsBoardControl::InvertMouseY: s.invertMouseY = !s.invertMouseY; break;
    case SettingsBoardControl::MuteAudio: s.audioMuted = !s.audioMuted; break;
    case SettingsBoardControl::MasterMinus: AdjustSettingsBoardFloat(s.audioMasterVolume, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MasterPlus: AdjustSettingsBoardFloat(s.audioMasterVolume, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MusicMinus: AdjustSettingsBoardFloat(s.audioMusicVolume, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MusicPlus: AdjustSettingsBoardFloat(s.audioMusicVolume, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::EffectsMinus: AdjustSettingsBoardFloat(s.audioEffectsVolume, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::EffectsPlus: AdjustSettingsBoardFloat(s.audioEffectsVolume, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::AmbienceMinus: AdjustSettingsBoardFloat(s.audioAmbienceVolume, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::AmbiencePlus: AdjustSettingsBoardFloat(s.audioAmbienceVolume, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MonsterMinus: AdjustSettingsBoardFloat(s.audioMonsterVolume, -0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::MonsterPlus: AdjustSettingsBoardFloat(s.audioMonsterVolume, 0.05f, 0.0f, 1.0f); break;
    case SettingsBoardControl::Save: SaveSettingsBoard(hwnd); return;
    case SettingsBoardControl::Back: ExitGameSettingsBoard(hwnd); return;
    default: {
        int actionIndex = SettingsBoardKeyActionIndex(static_cast<SettingsBoardControl>(control));
        if (actionIndex >= 0) {
            gApp->gameSettingsBoardCaptureAction = actionIndex;
            gApp->gameSettingsKeyCaptureActive = true;
            gApp->gameSettingsEscapeConsumed = false;
        }
        break;
    }
    }
    PushSettingsBoardStateToRenderer(hwnd);
    InvalidateRect(hwnd, nullptr, FALSE);
}
