// Legacy Win32 settings dialog and embedded game/debug settings host.
// Included from main.cpp after Renderer, app state, and shared window helpers.

#include "config_dialog_model.inl"

std::wstring ControlText(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    std::wstring text(static_cast<size_t>(len + 1), L'\0');
    GetWindowTextW(hwnd, text.data(), len + 1);
    text.resize(static_cast<size_t>(len));
    return text;
}

bool IsEyeConfigField(const ConfigFieldDef& def) {
    return std::wcscmp(def.section, L"Monster") == 0 &&
        (std::wcsstr(def.key, L"EyeX") || std::wcsstr(def.key, L"EyeY") || std::wcsstr(def.key, L"EyeZ"));
}

void EyeFieldRange(const ConfigFieldDef& def, float& minValue, float& maxValue) {
    if (std::wcsstr(def.key, L"EyeY")) {
        minValue = -0.45f;
        maxValue = 0.20f;
    } else if (std::wcsstr(def.key, L"EyeZ")) {
        minValue = -0.35f;
        maxValue = 0.20f;
    } else {
        minValue = -0.45f;
        maxValue = 0.45f;
    }
}

std::wstring FormatConfigFloat(float value) {
    wchar_t buffer[64]{};
    swprintf_s(buffer, L"%.3f", value);
    return buffer;
}

float ParseLooseFloat(const std::wstring& value, float fallback) {
    wchar_t* end = nullptr;
    float parsed = std::wcstof(value.c_str(), &end);
    return end != value.c_str() ? parsed : fallback;
}

int EyeSliderPosFromValue(const ConfigFieldDef& def, float value) {
    float minValue = 0.0f;
    float maxValue = 1.0f;
    EyeFieldRange(def, minValue, maxValue);
    float t = Clamp01((value - minValue) / std::max(0.0001f, maxValue - minValue));
    return static_cast<int>(std::round(t * 1000.0f));
}

float EyeValueFromSliderPos(const ConfigFieldDef& def, int pos) {
    float minValue = 0.0f;
    float maxValue = 1.0f;
    EyeFieldRange(def, minValue, maxValue);
    float t = std::clamp(static_cast<float>(pos) / 1000.0f, 0.0f, 1.0f);
    return Lerp(minValue, maxValue, t);
}

void SaveConfigFieldControl(const ConfigFieldUi& field) {
    if (!field.def || !field.control) return;
    std::wstring value;
    if (field.def->kind == ConfigFieldKind::Bool) {
        value = Button_GetCheck(field.control) == BST_CHECKED ? L"1" : L"0";
    } else {
        value = ControlText(field.control);
    }
    WritePrivateProfileStringW(field.def->section, field.def->key, value.c_str(), SettingsPath().c_str());
}

ConfigFieldUi* FindConfigFieldById(ConfigState* state, int id) {
    if (!state) return nullptr;
    for (auto& field : state->fields) {
        if (field.def && field.def->id == id) return &field;
    }
    return nullptr;
}

ConfigFieldUi* FindConfigFieldBySlider(ConfigState* state, HWND slider) {
    if (!state || !slider) return nullptr;
    for (auto& field : state->fields) {
        if (field.slider == slider) return &field;
    }
    return nullptr;
}

void SyncEyeSliderFromEdit(const ConfigFieldUi& field) {
    if (!field.def || !field.slider || !field.control) return;
    float fallback = ParseLooseFloat(field.def->fallback, 0.0f);
    float value = ParseLooseFloat(ControlText(field.control), fallback);
    SendMessageW(field.slider, TBM_SETPOS, TRUE, EyeSliderPosFromValue(*field.def, value));
}

bool ConfigActiveTabIsMonsterPreview(const ConfigState* state) {
    return state && ((state->mode == ConfigDialogMode::Full && state->activeTab == 7) ||
        (state->mode == ConfigDialogMode::Debug && state->activeTab == 3));
}

Settings SettingsFromConfigControls(const ConfigState* state);

void ApplyConfigPreviewOrbit(ConfigState* state) {
    if (!state || !state->previewRenderer || !ConfigActiveTabIsMonsterPreview(state)) return;
    state->previewRenderer->SetMonsterPreviewOrbit(
        state->previewOrbitYaw,
        state->previewOrbitPitch,
        state->previewOrbitDistance);
}

POINT ConfigClientPointToScreen(HWND hwnd, LPARAM lParam) {
    POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    ClientToScreen(hwnd, &p);
    return p;
}

bool ConfigPointOverPreview(ConfigState* state, POINT screenPoint) {
    if (!state || !state->preview || !ConfigActiveTabIsMonsterPreview(state)) return false;
    RECT rc{};
    if (!GetWindowRect(state->preview, &rc)) return false;
    return PtInRect(&rc, screenPoint) != FALSE;
}

void BeginConfigPreviewOrbit(ConfigState* state, HWND captureWindow, POINT screenPoint) {
    if (!state || !ConfigActiveTabIsMonsterPreview(state)) return;
    if (state->previewStatus) ShowWindow(state->previewStatus, SW_HIDE);
    SetFocus(state->preview ? state->preview : captureWindow);
    SetCapture(captureWindow);
    state->previewOrbitDragging = true;
    state->previewManualOrbit = true;
    state->previewLastMouse = screenPoint;
    ApplyConfigPreviewOrbit(state);
}

void EndConfigPreviewOrbit(ConfigState* state, HWND captureWindow) {
    if (!state || !state->previewOrbitDragging) return;
    state->previewOrbitDragging = false;
    if (captureWindow && GetCapture() == captureWindow) ReleaseCapture();
}

void UpdateConfigPreviewOrbit(ConfigState* state, POINT screenPoint) {
    if (!state || !state->previewOrbitDragging || !ConfigActiveTabIsMonsterPreview(state)) return;
    int dx = screenPoint.x - state->previewLastMouse.x;
    int dy = screenPoint.y - state->previewLastMouse.y;
    state->previewLastMouse = screenPoint;
    state->previewOrbitYaw += static_cast<float>(dx) * 0.010f;
    state->previewOrbitPitch = std::clamp(
        state->previewOrbitPitch + static_cast<float>(dy) * 0.0075f,
        -1.15f,
        0.75f);
    state->previewManualOrbit = true;
    ApplyConfigPreviewOrbit(state);
}

void ZoomConfigPreviewOrbit(ConfigState* state, WPARAM wParam) {
    if (!state || !ConfigActiveTabIsMonsterPreview(state) || !state->previewRenderer) return;
    int detents = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
    if (detents == 0) return;
    float factor = detents > 0 ? 0.86f : 1.16f;
    int steps = std::max(1, std::abs(detents));
    for (int i = 0; i < steps; ++i) state->previewOrbitDistance *= factor;
    state->previewOrbitDistance = std::clamp(state->previewOrbitDistance, 1.25f, 8.0f);
    state->previewManualOrbit = true;
    ApplyConfigPreviewOrbit(state);
}

void ApplyConfigPreviewEyeCalibration(ConfigState* state) {
    if (!state || !state->previewRenderer || !ConfigActiveTabIsMonsterPreview(state)) return;
    Settings previewSettings = SettingsFromConfigControls(state);
    if (!previewSettings.monsterAltSkullMesh.empty()) {
        previewSettings.monsterAltSkullChance = 1.0f;
    }
    state->previewRenderer->SetMonsterPreviewEyeCalibration(previewSettings);
    ApplyConfigPreviewOrbit(state);
}

void SetFieldControlValue(HWND control, const ConfigFieldDef& def, const std::wstring& value) {
    if (def.kind == ConfigFieldKind::Bool) {
        Button_SetCheck(control, (value == L"1" || Lower(value) == L"true") ? BST_CHECKED : BST_UNCHECKED);
    } else {
        SetWindowTextW(control, value.c_str());
    }
}

void LoadConfigControls(ConfigState* state, bool defaultsOnly) {
    state->loadingControls = true;
    for (auto& field : state->fields) {
        std::wstring value = defaultsOnly
            ? field.def->fallback
            : IniString(field.def->section, field.def->key, field.def->fallback);
        SetFieldControlValue(field.control, *field.def, value);
        SyncEyeSliderFromEdit(field);
    }
    state->loadingControls = false;
}

void SaveConfigControls(ConfigState* state) {
    for (const auto& field : state->fields) {
        std::wstring value;
        if (field.def->kind == ConfigFieldKind::Bool) {
            value = Button_GetCheck(field.control) == BST_CHECKED ? L"1" : L"0";
        } else {
            value = ControlText(field.control);
        }
        WritePrivateProfileStringW(field.def->section, field.def->key, value.c_str(), SettingsPath().c_str());
    }
}

void ResetVisibleConfigControls(ConfigState* state) {
    if (!state) return;
    state->loadingControls = true;
    for (auto& field : state->fields) {
        SetFieldControlValue(field.control, *field.def, field.def->fallback);
        SyncEyeSliderFromEdit(field);
    }
    state->loadingControls = false;
    SaveConfigControls(state);
}

const ConfigFieldUi* FindConfigField(const ConfigState* state, const wchar_t* section, const wchar_t* key) {
    for (const auto& field : state->fields) {
        if (wcscmp(field.def->section, section) == 0 && wcscmp(field.def->key, key) == 0) return &field;
    }
    return nullptr;
}

std::wstring ConfigControlValue(const ConfigState* state, const wchar_t* section, const wchar_t* key, const wchar_t* fallback) {
    const ConfigFieldUi* field = FindConfigField(state, section, key);
    if (!field) return IniString(section, key, fallback);
    if (field->def->kind == ConfigFieldKind::Bool) {
        return Button_GetCheck(field->control) == BST_CHECKED ? L"1" : L"0";
    }
    return ControlText(field->control);
}

int ParseConfigInt(const ConfigState* state, const wchar_t* section, const wchar_t* key, int fallback) {
    std::wstring value = ConfigControlValue(state, section, key, L"");
    wchar_t* end = nullptr;
    long parsed = std::wcstol(value.c_str(), &end, 10);
    return end != value.c_str() ? static_cast<int>(parsed) : fallback;
}

float ParseConfigFloat(const ConfigState* state, const wchar_t* section, const wchar_t* key, float fallback) {
    std::wstring value = ConfigControlValue(state, section, key, L"");
    wchar_t* end = nullptr;
    float parsed = std::wcstof(value.c_str(), &end);
    return end != value.c_str() ? parsed : fallback;
}

std::wstring ParseConfigString(const ConfigState* state, const wchar_t* section, const wchar_t* key, const wchar_t* fallback) {
    return ConfigControlValue(state, section, key, fallback);
}

Settings SettingsFromConfigControls(const ConfigState* state) {
    Settings s;
    s.allowWarpFallback = ParseConfigInt(state, L"Renderer", L"AllowWarpFallback", s.allowWarpFallback ? 1 : 0) != 0;
    s.gameFullscreen = ParseConfigInt(state, L"GameWindow", L"Fullscreen", s.gameFullscreen ? 1 : 0) != 0;
    s.gameResolutionWidth = std::clamp(ParseConfigInt(state, L"GameWindow", L"ResolutionWidth", s.gameResolutionWidth), 640, 7680);
    s.gameResolutionHeight = std::clamp(ParseConfigInt(state, L"GameWindow", L"ResolutionHeight", s.gameResolutionHeight), 360, 4320);
    s.mazeWidth = std::clamp(ParseConfigInt(state, L"Maze", L"Width", s.mazeWidth) | 1, 15, 151);
    s.mazeHeight = std::clamp(ParseConfigInt(state, L"Maze", L"Height", s.mazeHeight) | 1, 15, 151);
    s.roomCount = std::clamp(ParseConfigInt(state, L"Maze", L"RoomCount", s.roomCount), 0, 80);
    s.roomMinRadius = std::clamp(ParseConfigInt(state, L"Maze", L"RoomMinRadius", s.roomMinRadius), 1, 12);
    s.roomMaxRadius = std::clamp(ParseConfigInt(state, L"Maze", L"RoomMaxRadius", s.roomMaxRadius), s.roomMinRadius, 16);
    s.roomCountRange = std::clamp(ParseConfigInt(state, L"Maze", L"RoomCountRange", s.roomCountRange), 0, 80);
    s.roomMinRadiusRange = std::clamp(ParseConfigInt(state, L"Maze", L"RoomMinRadiusRange", s.roomMinRadiusRange), 0, 12);
    s.roomMaxRadiusRange = std::clamp(ParseConfigInt(state, L"Maze", L"RoomMaxRadiusRange", s.roomMaxRadiusRange), 0, 16);
    s.extraConnectorMinRatio = std::clamp(ParseConfigFloat(state, L"Maze", L"ExtraConnectorMinRatio", s.extraConnectorMinRatio), 0.015f, 0.20f);
    s.extraConnectorMaxRatio = std::clamp(ParseConfigFloat(state, L"Maze", L"ExtraConnectorMaxRatio", s.extraConnectorMaxRatio), s.extraConnectorMinRatio, 0.20f);
    s.wallFeatureFrequency = std::clamp(ParseConfigInt(state, L"Maze", L"WallFeatureFrequency", s.wallFeatureFrequency), 1, 200);
    s.wallFeatureFrequencySpread = std::clamp(ParseConfigFloat(state, L"Maze", L"WallFeatureFrequencySpread", s.wallFeatureFrequencySpread), 0.0f, 3.0f);
    s.saveItemMinPerLayer = std::clamp(ParseConfigInt(state, L"Maze", L"SaveItemMinPerLayer", s.saveItemMinPerLayer), 0, 5);
    s.saveItemMaxPerLayer = std::clamp(ParseConfigInt(state, L"Maze", L"SaveItemMaxPerLayer", s.saveItemMaxPerLayer), s.saveItemMinPerLayer, 5);
    s.saveItemLevelChance = std::clamp(ParseConfigFloat(state, L"Maze", L"SaveItemLevelChance", s.saveItemLevelChance), 0.0f, 1.0f);
    s.mazeSeed = static_cast<uint32_t>(std::clamp(ParseConfigInt(state, L"Maze", L"RandomSeed", static_cast<int>(s.mazeSeed)), 0, std::numeric_limits<int>::max()));
    s.mapOverlay = ParseConfigInt(state, L"Maze", L"MapOverlay", s.mapOverlay ? 1 : 0) != 0;
    s.debugAiMapOverlay = ParseConfigInt(state, L"Debug", L"AiMapOverlay", s.debugAiMapOverlay ? 1 : 0) != 0;
    s.debugInfiniteStamina = ParseConfigInt(state, L"Debug", L"InfiniteStamina", s.debugInfiniteStamina ? 1 : 0) != 0;
    s.debugInvincible = ParseConfigInt(state, L"Debug", L"Invincible", s.debugInvincible ? 1 : 0) != 0;
    s.tileWidthMeters = std::clamp(ParseConfigFloat(state, L"Maze", L"TileWidthMeters", s.tileWidthMeters), 1.2f, 8.0f);
    s.tileLengthMeters = std::clamp(ParseConfigFloat(state, L"Maze", L"TileLengthMeters", s.tileLengthMeters), 1.2f, 8.0f);
    s.wallHeightMeters = std::clamp(ParseConfigFloat(state, L"Maze", L"WallHeightMeters", s.wallHeightMeters), 2.85f, 8.0f);
    s.runVariation = std::clamp(ParseConfigFloat(state, L"Randomization", L"RunVariation", s.runVariation), 0.0f, 1.0f);

    s.assetFolder = ParseConfigString(state, L"Textures", L"AssetFolder", s.assetFolder.c_str());
    s.wallStem = ParseConfigString(state, L"Textures", L"WallStem", s.wallStem.c_str());
    s.floorStem = ParseConfigString(state, L"Textures", L"FloorStem", s.floorStem.c_str());
    s.ceilingStem = ParseConfigString(state, L"Textures", L"CeilingStem", s.ceilingStem.c_str());
    s.fleshStem = ParseConfigString(state, L"Textures", L"FleshStem", s.fleshStem.c_str());
    s.wallTextureMeters = std::max(0.2f, ParseConfigFloat(state, L"Textures", L"WallScaleMeters", s.wallTextureMeters));
    s.floorTextureMeters = std::max(0.2f, ParseConfigFloat(state, L"Textures", L"FloorScaleMeters", s.floorTextureMeters));
    s.ceilingTextureMeters = std::max(0.0f, ParseConfigFloat(state, L"Textures", L"CeilingScaleMeters", s.ceilingTextureMeters));
    s.useExternalNormals = ParseConfigInt(state, L"Textures", L"UseExternalNormals", s.useExternalNormals ? 1 : 0) != 0;
    s.maxNormalMapMB = std::clamp(ParseConfigInt(state, L"Textures", L"MaxNormalMapMB", s.maxNormalMapMB), 0, 1024);

    s.flashlightIntensity = std::clamp(ParseConfigFloat(state, L"Lighting", L"FlashlightIntensity", s.flashlightIntensity), 0.0f, 10.0f);
    s.flashlightAttenuation = std::clamp(ParseConfigFloat(state, L"Lighting", L"FlashlightAttenuation", s.flashlightAttenuation), 0.001f, 2.0f);
    s.flashlightConeDegrees = std::clamp(ParseConfigFloat(state, L"Lighting", L"FlashlightConeDegrees", s.flashlightConeDegrees), 20.0f, 140.0f);
    s.flashlightShadows = ParseConfigInt(state, L"Lighting", L"FlashlightShadows", s.flashlightShadows ? 1 : 0) != 0;
    s.flashlightShadowStrength = std::clamp(ParseConfigFloat(state, L"Lighting", L"FlashlightShadowStrength", s.flashlightShadowStrength), 0.0f, 1.0f);
    s.flashlightShadowDistanceMeters = std::clamp(ParseConfigFloat(state, L"Lighting", L"FlashlightShadowDistanceMeters", s.flashlightShadowDistanceMeters), 2.0f, 45.0f);
    s.flashlightShadowBias = NormalizeFlashlightShadowBias(ParseConfigFloat(state, L"Lighting", L"FlashlightShadowBias", s.flashlightShadowBias));
    s.flashlightShadowMapSize = std::clamp(ParseConfigInt(state, L"Lighting", L"FlashlightShadowMapSize", s.flashlightShadowMapSize), 512, 4096);
    s.ambientLight = std::clamp(ParseConfigFloat(state, L"Lighting", L"AmbientLight", s.ambientLight), 0.0f, 1.0f);
    s.lampIntensity = std::clamp(ParseConfigFloat(state, L"Lighting", L"LampIntensity", s.lampIntensity), 0.0f, 10.0f);
    s.lampSpacing = std::clamp(ParseConfigFloat(state, L"Lighting", L"LampSpacing", s.lampSpacing), 2.0f, 40.0f);
    s.lampOnRatio = std::clamp(ParseConfigFloat(state, L"Lighting", L"LampOnRatio", s.lampOnRatio), 0.0f, 1.0f);
    s.lampFlickerRatio = std::clamp(ParseConfigFloat(state, L"Lighting", L"LampFlickerRatio", s.lampFlickerRatio), 0.0f, 1.0f);
    s.brokenZoneRatio = std::clamp(ParseConfigFloat(state, L"Lighting", L"BrokenZoneRatio", s.brokenZoneRatio), 0.0f, 1.0f);
    s.darkLampVisibleRatio = std::clamp(ParseConfigFloat(state, L"Lighting", L"DarkLampVisibleRatio", s.darkLampVisibleRatio), 0.0f, 1.0f);
    s.fogStartMeters = std::clamp(ParseConfigFloat(state, L"Lighting", L"FogStartMeters", s.fogStartMeters), 0.0f, 200.0f);
    s.fogEndMeters = std::max(s.fogStartMeters + 0.1f, std::clamp(ParseConfigFloat(state, L"Lighting", L"FogEndMeters", s.fogEndMeters), 0.1f, 300.0f));
    s.fogDarkness = std::clamp(ParseConfigFloat(state, L"Lighting", L"FogDarkness", s.fogDarkness), 0.0f, 1.0f);
    s.cornerAOIntensity = std::clamp(ParseConfigFloat(state, L"Lighting", L"CornerAOIntensity", s.cornerAOIntensity), 0.0f, 1.0f);
    s.cornerAORadius = std::clamp(ParseConfigFloat(state, L"Lighting", L"CornerAORadius", s.cornerAORadius), 0.05f, 2.0f);
    s.floorCeilingAOIntensity = std::clamp(ParseConfigFloat(state, L"Lighting", L"FloorCeilingAOIntensity", s.floorCeilingAOIntensity), 0.0f, 1.0f);
    s.exposure = std::clamp(ParseConfigFloat(state, L"Lighting", L"Exposure", s.exposure), 0.1f, 8.0f);
    s.gamma = std::clamp(ParseConfigFloat(state, L"Lighting", L"Gamma", s.gamma), 0.5f, 3.5f);
    s.motionBlurAmount = std::clamp(ParseConfigFloat(state, L"Lighting", L"MotionBlurAmount", s.motionBlurAmount), 0.0f, 2.0f);
    s.bloomAmount = std::clamp(ParseConfigFloat(state, L"Lighting", L"BloomAmount", s.bloomAmount), 0.0f, 2.0f);
    s.lensDirtAmount = std::clamp(ParseConfigFloat(state, L"Lighting", L"LensDirtAmount", s.lensDirtAmount), 0.0f, 2.0f);

    s.walkSpeed = std::clamp(ParseConfigFloat(state, L"CameraAI", L"WalkSpeed", s.walkSpeed), 0.1f, 8.0f);
    s.roomSpeed = std::clamp(ParseConfigFloat(state, L"CameraAI", L"RoomSpeed", s.roomSpeed), 0.1f, 8.0f);
    s.runSpeed = std::clamp(ParseConfigFloat(state, L"CameraAI", L"RunSpeed", s.runSpeed), 0.1f, 12.0f);
    s.turnLookAheadTiles = std::clamp(ParseConfigFloat(state, L"CameraAI", L"TurnLookAheadTiles", s.turnLookAheadTiles), 0.0f, 8.0f);
    s.roomLookAheadTiles = std::clamp(ParseConfigFloat(state, L"CameraAI", L"RoomLookAheadTiles", s.roomLookAheadTiles), 0.0f, 10.0f);
    s.roomPauseChance = std::clamp(ParseConfigFloat(state, L"CameraAI", L"RoomPauseChance", s.roomPauseChance), 0.0f, 1.0f);
    s.junctionScanChance = std::clamp(ParseConfigFloat(state, L"CameraAI", L"JunctionScanChance", s.junctionScanChance), 0.0f, 1.0f);
    s.scanAngleDegrees = std::clamp(ParseConfigFloat(state, L"CameraAI", L"ScanAngleDegrees", s.scanAngleDegrees), 0.0f, 160.0f);
    s.lookBackMinSeconds = std::clamp(ParseConfigFloat(state, L"CameraAI", L"LookBackMinSeconds", s.lookBackMinSeconds), 2.0f, 300.0f);
    s.lookBackMaxSeconds = std::max(s.lookBackMinSeconds, std::clamp(ParseConfigFloat(state, L"CameraAI", L"LookBackMaxSeconds", s.lookBackMaxSeconds), 2.0f, 300.0f));
    s.headBobAmount = std::clamp(ParseConfigFloat(state, L"CameraAI", L"HeadBobAmount", s.headBobAmount), 0.0f, 0.4f);
    s.sideSwayAmount = std::clamp(ParseConfigFloat(state, L"CameraAI", L"SideSwayAmount", s.sideSwayAmount), 0.0f, 0.3f);
    s.junctionScanBaseSeconds = std::clamp(ParseConfigFloat(state, L"CameraAI", L"JunctionScanBaseSeconds", s.junctionScanBaseSeconds), 0.0f, 4.0f);
    s.junctionScanBranchSeconds = std::clamp(ParseConfigFloat(state, L"CameraAI", L"JunctionScanBranchSeconds", s.junctionScanBranchSeconds), 0.0f, 3.0f);

    s.flashlightSwayAmount = std::clamp(ParseConfigFloat(state, L"CameraFX", L"FlashlightSwayAmount", s.flashlightSwayAmount), 0.0f, 4.0f);
    s.flashlightFollowSpeed = std::clamp(ParseConfigFloat(state, L"CameraFX", L"FlashlightFollowSpeed", s.flashlightFollowSpeed), 0.1f, 4.0f);
    s.flashlightPanicDartAmount = std::clamp(ParseConfigFloat(state, L"CameraFX", L"FlashlightPanicDartAmount", s.flashlightPanicDartAmount), 0.0f, 4.0f);
    s.exitDoorOpenSeconds = std::clamp(ParseConfigFloat(state, L"CameraFX", L"ExitDoorOpenSeconds", s.exitDoorOpenSeconds), 0.2f, 8.0f);
    s.exitStepSeconds = std::clamp(ParseConfigFloat(state, L"CameraFX", L"ExitStepSeconds", s.exitStepSeconds), 0.2f, 8.0f);
    s.exitFadeSeconds = std::clamp(ParseConfigFloat(state, L"CameraFX", L"ExitFadeSeconds", s.exitFadeSeconds), 0.2f, 8.0f);
    s.exitStepDistance = std::clamp(ParseConfigFloat(state, L"CameraFX", L"ExitStepDistance", s.exitStepDistance), 0.0f, 8.0f);
    s.fadeInSeconds = std::clamp(ParseConfigFloat(state, L"CameraFX", L"FadeInSeconds", s.fadeInSeconds), 0.0f, 8.0f);
    s.mouseSensitivity = std::clamp(ParseConfigFloat(state, L"Controls", L"MouseSensitivity", s.mouseSensitivity), 0.1f, 5.0f);
    s.invertMouseY = ParseConfigInt(state, L"Controls", L"InvertMouseY", s.invertMouseY ? 1 : 0) != 0;
    s.audioMuted = ParseConfigInt(state, L"Audio", L"Muted", s.audioMuted ? 1 : 0) != 0;
    s.audioMasterVolume = std::clamp(ParseConfigFloat(state, L"Audio", L"MasterVolume", s.audioMasterVolume), 0.0f, 1.0f);
    s.audioEffectsVolume = std::clamp(ParseConfigFloat(state, L"Audio", L"EffectsVolume", s.audioEffectsVolume), 0.0f, 1.0f);
    s.audioAmbienceVolume = std::clamp(ParseConfigFloat(state, L"Audio", L"AmbienceVolume", s.audioAmbienceVolume), 0.0f, 1.0f);
    s.audioMonsterVolume = std::clamp(ParseConfigFloat(state, L"Audio", L"MonsterVolume", s.audioMonsterVolume), 0.0f, 1.0f);

    s.paperDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"PaperDensity", s.paperDensity), 0.0f, 4.0f);
    s.hallwayPaperRunDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"HallwayPaperRunDensity", s.hallwayPaperRunDensity), 0.0f, 4.0f);
    s.chairDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"ChairDensity", s.chairDensity), 0.0f, 4.0f);
    s.waterDamageEnabled = ParseConfigInt(state, L"Atmosphere", L"WaterDamageEnabled", s.waterDamageEnabled ? 1 : 0) != 0;
    s.waterDamageDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"WaterDamageDensity", s.waterDamageDensity), 0.0f, 4.0f);
    s.metalCabinetDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"MetalCabinetDensity", s.metalCabinetDensity), 0.0f, 4.0f);
    s.jumpscareFrequency = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"JumpscareFrequency", s.jumpscareFrequency), 0.0f, 1.0f);
    s.sparkParticles = ParseConfigInt(state, L"Atmosphere", L"SparkParticles", s.sparkParticles ? 1 : 0) != 0;
    s.sparkEmitterRatio = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"SparkEmitterRatio", s.sparkEmitterRatio), 0.0f, 1.0f);
    s.sparkBurstMinSeconds = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"SparkBurstMinSeconds", s.sparkBurstMinSeconds), 0.05f, 60.0f);
    s.sparkBurstMaxSeconds = std::max(s.sparkBurstMinSeconds, std::clamp(ParseConfigFloat(state, L"Atmosphere", L"SparkBurstMaxSeconds", s.sparkBurstMaxSeconds), 0.05f, 60.0f));
    s.sparkMaxParticles = std::clamp(ParseConfigInt(state, L"Atmosphere", L"SparkMaxParticles", s.sparkMaxParticles), 0, 1200);
    s.sparkSize = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"SparkSize", s.sparkSize), 0.1f, 5.0f);
    s.airParticles = ParseConfigInt(state, L"Atmosphere", L"AirParticles", s.airParticles ? 1 : 0) != 0;
    s.airParticleDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"AirParticleDensity", s.airParticleDensity), 0.0f, 4.0f);
    s.airParticleSize = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"AirParticleSize", s.airParticleSize), 0.20f, 4.0f);
    s.airParticleBlur = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"AirParticleBlur", s.airParticleBlur), 0.0f, 3.0f);
    s.bloodSplatterDensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodSplatterDensity", s.bloodSplatterDensity), 0.0f, 4.0f);
    s.bloodBurstCount = std::clamp(ParseConfigInt(state, L"Atmosphere", L"BloodBurstCount", s.bloodBurstCount), 0, 160);
    s.bloodWetness = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWetness", s.bloodWetness), 0.0f, 3.0f);
    s.bloodStreamCount = std::clamp(ParseConfigInt(state, L"Atmosphere", L"BloodStreamCount", s.bloodStreamCount), 4, 32);
    s.bloodStreamThickness = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodStreamThickness", s.bloodStreamThickness), 0.10f, 2.0f);
    s.bloodShaderQuality = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodShaderQuality", s.bloodShaderQuality), 0.25f, 1.0f);
    s.bloodWorldFlicker = ParseConfigInt(state, L"Atmosphere", L"BloodWorldFlicker", s.bloodWorldFlicker ? 1 : 0) != 0;
    s.bloodWorldAlwaysOn = ParseConfigInt(state, L"Atmosphere", L"BloodWorldAlwaysOn", s.bloodWorldAlwaysOn ? 1 : 0) != 0;
    s.bloodWorldCoverage = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldCoverage", s.bloodWorldCoverage), 0.0f, 1.0f);
    s.bloodWorldFlickerMinSeconds = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerMinSeconds", s.bloodWorldFlickerMinSeconds), 60.0f, 7200.0f);
    s.bloodWorldFlickerMaxSeconds = std::max(s.bloodWorldFlickerMinSeconds, std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerMaxSeconds", s.bloodWorldFlickerMaxSeconds), 60.0f, 7200.0f));
    s.bloodWorldFlickerDuration = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerDuration", s.bloodWorldFlickerDuration), 0.15f, 8.0f);
    s.bloodWorldFlickerIntensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerIntensity", s.bloodWorldFlickerIntensity), 0.0f, 2.0f);
    s.bloodStudyView = ParseConfigInt(state, L"Atmosphere", L"BloodStudyView", s.bloodStudyView ? 1 : 0) != 0;
    s.fleshFlicker = ParseConfigInt(state, L"Atmosphere", L"FleshFlicker", s.fleshFlicker ? 1 : 0) != 0;
    s.fleshAlwaysOn = ParseConfigInt(state, L"Atmosphere", L"FleshAlwaysOn", s.fleshAlwaysOn ? 1 : 0) != 0;
    s.fleshWetness = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshWetness", s.fleshWetness), 0.0f, 4.0f);
    s.fleshParallaxScale = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshParallaxScale", s.fleshParallaxScale), 0.0f, 0.50f);
    s.fleshFlickerMinSeconds = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshFlickerMinSeconds", s.fleshFlickerMinSeconds), 60.0f, 7200.0f);
    s.fleshFlickerMaxSeconds = std::max(s.fleshFlickerMinSeconds, std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshFlickerMaxSeconds", s.fleshFlickerMaxSeconds), 60.0f, 7200.0f));
    s.fleshFlickerDuration = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshFlickerDuration", s.fleshFlickerDuration), 0.15f, 8.0f);
    s.fleshFlickerIntensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshFlickerIntensity", s.fleshFlickerIntensity), 0.0f, 2.0f);

    s.effectBloodLoopSeconds = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"BloodLoopSeconds", s.effectBloodLoopSeconds), 1.0f, 180.0f);
    s.effectBloodFullSpreadAge = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"BloodFullSpreadAge", s.effectBloodFullSpreadAge), 0.1f, s.effectBloodLoopSeconds);
    s.effectWaterLoopSeconds = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"WaterLoopSeconds", s.effectWaterLoopSeconds), 0.5f, 60.0f);
    s.effectAirVentLoopSeconds = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"AirVentLoopSeconds", s.effectAirVentLoopSeconds), 0.5f, 60.0f);
    s.effectBrokenLampLoopSeconds = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"BrokenLampLoopSeconds", s.effectBrokenLampLoopSeconds), 0.5f, 60.0f);
    s.effectStaticLoopSeconds = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"StaticLoopSeconds", s.effectStaticLoopSeconds), 0.5f, 60.0f);
    s.effectBrokenLampSparkIntensityMin = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"BrokenLampSparkIntensityMin", s.effectBrokenLampSparkIntensityMin), 0.1f, 12.0f);
    s.effectBrokenLampSparkIntensityMax = std::max(s.effectBrokenLampSparkIntensityMin,
        std::clamp(ParseConfigFloat(state, L"EffectTuning", L"BrokenLampSparkIntensityMax", s.effectBrokenLampSparkIntensityMax), 0.1f, 16.0f));
    s.effectBrokenLampChainIntensityScale = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"BrokenLampChainIntensityScale", s.effectBrokenLampChainIntensityScale), 0.0f, 4.0f);
    s.effectBrokenLampChainBurstsMin = std::clamp(ParseConfigInt(state, L"EffectTuning", L"BrokenLampChainBurstsMin", s.effectBrokenLampChainBurstsMin), 0, 16);
    s.effectBrokenLampChainBurstsMax = std::max(s.effectBrokenLampChainBurstsMin,
        std::clamp(ParseConfigInt(state, L"EffectTuning", L"BrokenLampChainBurstsMax", s.effectBrokenLampChainBurstsMax), 0, 24));
    s.effectAirVentSteamIntensityMin = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"AirVentSteamIntensityMin", s.effectAirVentSteamIntensityMin), 0.1f, 12.0f);
    s.effectAirVentSteamIntensityMax = std::max(s.effectAirVentSteamIntensityMin,
        std::clamp(ParseConfigFloat(state, L"EffectTuning", L"AirVentSteamIntensityMax", s.effectAirVentSteamIntensityMax), 0.1f, 16.0f));
    s.effectAirVentPanelDropEvery = std::clamp(ParseConfigInt(state, L"EffectTuning", L"AirVentPanelDropEvery", s.effectAirVentPanelDropEvery), 1, 32);
    s.effectAirVentPanelDropChance = std::clamp(ParseConfigFloat(state, L"EffectTuning", L"AirVentPanelDropChance", s.effectAirVentPanelDropChance), 0.0f, 1.0f);

    s.dreadEnabled = ParseConfigInt(state, L"Dread", L"Enabled", s.dreadEnabled ? 1 : 0) != 0;
    s.dreadDebugMeter = ParseConfigInt(state, L"Dread", L"DebugMeter", s.dreadDebugMeter ? 1 : 0) != 0;
    s.dreadDecayPerSecond = std::clamp(ParseConfigFloat(state, L"Dread", L"DecayPerSecond", s.dreadDecayPerSecond), 0.0f, 1.0f);
    s.dreadMonsterDistance = std::clamp(ParseConfigFloat(state, L"Dread", L"MonsterDistance", s.dreadMonsterDistance), 1.0f, 60.0f);
    s.dreadMonsterGainPerSecond = std::clamp(ParseConfigFloat(state, L"Dread", L"MonsterGainPerSecond", s.dreadMonsterGainPerSecond), 0.0f, 3.0f);
    s.dreadJumpscareGain = std::clamp(ParseConfigFloat(state, L"Dread", L"JumpscareGain", s.dreadJumpscareGain), 0.0f, 1.0f);
    s.dreadFleshGain = std::clamp(ParseConfigFloat(state, L"Dread", L"FleshGain", s.dreadFleshGain), 0.0f, 1.0f);
    s.dreadWalkSpeedBoost = std::clamp(ParseConfigFloat(state, L"Dread", L"WalkSpeedBoost", s.dreadWalkSpeedBoost), 0.0f, 2.0f);
    s.dreadRunSpeedBoost = std::clamp(ParseConfigFloat(state, L"Dread", L"RunSpeedBoost", s.dreadRunSpeedBoost), 0.0f, 2.0f);
    s.dreadFlashlightFlicker = std::clamp(ParseConfigFloat(state, L"Dread", L"FlashlightFlicker", s.dreadFlashlightFlicker), 0.0f, 3.0f);

    s.monsterScale = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterScale", s.monsterScale), 0.25f, 4.0f);
    s.monsterSpeed = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterSpeed", s.monsterSpeed), 0.1f, 4.0f);
    s.monsterSprintSpeed = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterSprintSpeed", s.monsterSprintSpeed), 0.1f, 4.0f);
    s.monsterIgnorePlayer = ParseConfigInt(state, L"Monster", L"MonsterIgnorePlayer", s.monsterIgnorePlayer ? 1 : 0) != 0;
    s.monsterKillDistance = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterKillDistance", s.monsterKillDistance), 0.2f, 4.0f);
    s.monsterVisibleDistance = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterVisibleDistance", s.monsterVisibleDistance), 1.0f, 60.0f);
    s.monsterSkullMesh = ParseConfigString(state, L"Monster", L"SkullMesh", s.monsterSkullMesh.c_str());
    s.monsterAltSkullMesh = ParseConfigString(state, L"Monster", L"AlternateSkullMesh", s.monsterAltSkullMesh.c_str());
    s.monsterAltSkullChance = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateSkullChance", s.monsterAltSkullChance), 0.0f, 1.0f);
    s.monsterSkullMaxTriangles = std::clamp(ParseConfigInt(state, L"Monster", L"SkullMaxTriangles", s.monsterSkullMaxTriangles), 0, 90000);
    s.monsterSkullYawDegrees = std::clamp(ParseConfigFloat(state, L"Monster", L"SkullYawDegrees", s.monsterSkullYawDegrees), -180.0f, 180.0f);
    s.monsterSkullPitchDegrees = std::clamp(ParseConfigFloat(state, L"Monster", L"SkullPitchDegrees", s.monsterSkullPitchDegrees), -180.0f, 180.0f);
    s.monsterSkullRollDegrees = std::clamp(ParseConfigFloat(state, L"Monster", L"SkullRollDegrees", s.monsterSkullRollDegrees), -180.0f, 180.0f);
    s.monsterAltSkullYawDegrees = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateSkullYawDegrees", s.monsterAltSkullYawDegrees), -180.0f, 180.0f);
    s.monsterAltSkullPitchDegrees = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateSkullPitchDegrees", s.monsterAltSkullPitchDegrees), -180.0f, 180.0f);
    s.monsterAltSkullRollDegrees = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateSkullRollDegrees", s.monsterAltSkullRollDegrees), -180.0f, 180.0f);
    s.monsterRightEyeX = std::clamp(ParseConfigFloat(state, L"Monster", L"RightEyeX", s.monsterRightEyeX), -0.45f, 0.45f);
    s.monsterRightEyeY = std::clamp(ParseConfigFloat(state, L"Monster", L"RightEyeY", s.monsterRightEyeY), -0.45f, 0.20f);
    s.monsterRightEyeZ = std::clamp(ParseConfigFloat(state, L"Monster", L"RightEyeZ", s.monsterRightEyeZ), -0.35f, 0.20f);
    s.monsterLeftEyeX = std::clamp(ParseConfigFloat(state, L"Monster", L"LeftEyeX", s.monsterLeftEyeX), -0.45f, 0.45f);
    s.monsterLeftEyeY = std::clamp(ParseConfigFloat(state, L"Monster", L"LeftEyeY", s.monsterLeftEyeY), -0.45f, 0.20f);
    s.monsterLeftEyeZ = std::clamp(ParseConfigFloat(state, L"Monster", L"LeftEyeZ", s.monsterLeftEyeZ), -0.35f, 0.20f);
    s.monsterAltRightEyeX = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateRightEyeX", s.monsterAltRightEyeX), -0.45f, 0.45f);
    s.monsterAltRightEyeY = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateRightEyeY", s.monsterAltRightEyeY), -0.45f, 0.20f);
    s.monsterAltRightEyeZ = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateRightEyeZ", s.monsterAltRightEyeZ), -0.35f, 0.20f);
    s.monsterAltLeftEyeX = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateLeftEyeX", s.monsterAltLeftEyeX), -0.45f, 0.45f);
    s.monsterAltLeftEyeY = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateLeftEyeY", s.monsterAltLeftEyeY), -0.45f, 0.20f);
    s.monsterAltLeftEyeZ = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateLeftEyeZ", s.monsterAltLeftEyeZ), -0.35f, 0.20f);
    return s;
}

void SetConfigPreviewStatus(ConfigState* state, const wchar_t* text) {
    if (!state || !state->previewStatus) return;
    if (text && text[0] != L'\0') {
        SetWindowTextW(state->previewStatus, text);
        ShowWindow(state->previewStatus, SW_SHOW);
    } else {
        ShowWindow(state->previewStatus, SW_HIDE);
    }
}

void UpdateConfigPreviewHint(ConfigState* state) {
    if (!state || !state->previewHint) return;
    const wchar_t* hint = ConfigActiveTabIsMonsterPreview(state)
        ? L"Click Update preview after changing monster settings. Drag the preview to rotate the skull; use the wheel to zoom."
        : L"Edit several values, then click Update preview to rebuild the embedded screensaver once.";
    SetWindowTextW(state->previewHint, hint);
}

void MarkConfigPreviewDirty(ConfigState* state) {
    if (!state || state->loadingControls) return;
    state->previewPending = false;
    SetConfigPreviewStatus(state, state->previewRenderer
        ? L"Preview settings changed. Click Update preview."
        : L"Click Update preview to render current settings.");
}

void RestartConfigPreview(ConfigState* state) {
    if (!state || !state->preview) return;
    SetConfigPreviewStatus(state, L"Updating preview...");
    if (state->previewStatus) UpdateWindow(state->previewStatus);
    state->previewRenderer.reset();
    state->previewPending = false;
    Settings previewSettings = SettingsFromConfigControls(state);
    state->previewRenderer = std::make_unique<Renderer>();
    bool monsterPreview = ConfigActiveTabIsMonsterPreview(state);
    if (monsterPreview && !previewSettings.monsterAltSkullMesh.empty()) {
        previewSettings.monsterAltSkullChance = 1.0f;
    }
    MonsterPreviewView previewView = monsterPreview ? MonsterPreviewView::Front : MonsterPreviewView::Orbit;
    if (state->previewRenderer->Initialize(state->preview, &previewSettings, monsterPreview, previewView)) {
        if (monsterPreview) ApplyConfigPreviewOrbit(state);
        SetConfigPreviewStatus(state, L"");
    } else {
        state->previewRenderer.reset();
        SetConfigPreviewStatus(state, L"Direct3D preview unavailable.");
    }
}

void ScheduleConfigPreview(ConfigState* state, ULONGLONG delayMs = 450) {
    (void)delayMs;
    MarkConfigPreviewDirty(state);
}

int ConfigVisibleHeight() {
    return kConfigContentBottom - kConfigContentTop;
}

int ConfigMaxScroll(const ConfigState* state, int tab) {
    if (!state || tab < 0 || tab >= state->tabCount) return 0;
    return std::max(0, state->contentHeight[static_cast<size_t>(tab)] - ConfigVisibleHeight());
}

void MoveConfigChildY(HWND child, int y) {
    if (!child) return;
    RECT rc{};
    GetWindowRect(child, &rc);
    POINT pt{rc.left, rc.top};
    ScreenToClient(GetParent(child), &pt);
    SetWindowPos(child, nullptr, pt.x, y, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void SetConfigScrollOffset(ConfigState* state, int value) {
    if (!state) return;
    int tab = state->activeTab;
    int maxScroll = ConfigMaxScroll(state, tab);
    state->scrollOffset[static_cast<size_t>(tab)] = std::clamp(value, 0, maxScroll);
}

void ApplyConfigScroll(ConfigState* state) {
    if (!state) return;
    int tab = state->activeTab;
    int maxScroll = ConfigMaxScroll(state, tab);
    int offset = std::clamp(state->scrollOffset[static_cast<size_t>(tab)], 0, maxScroll);
    state->scrollOffset[static_cast<size_t>(tab)] = offset;

    if (state->scrollBar) {
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = maxScroll + ConfigVisibleHeight() - 1;
        si.nPage = ConfigVisibleHeight();
        si.nPos = offset;
        SetScrollInfo(state->scrollBar, SB_CTL, &si, TRUE);
        ShowWindow(state->scrollBar, maxScroll > 0 ? SW_SHOW : SW_HIDE);
    }

    auto fullyVisibleAt = [](int y, int h) {
        return y >= 0 && y + h <= ConfigVisibleHeight();
    };
    for (const auto& header : state->headers) {
        int headerY = header.baseY - offset;
        bool show = header.tab == tab && fullyVisibleAt(headerY, 24);
        MoveConfigChildY(header.control, headerY);
        ShowWindow(header.control, show ? SW_SHOW : SW_HIDE);
    }
    for (const auto& field : state->fields) {
        bool active = field.def->tab == tab;
        int labelY = field.labelBaseY - offset;
        int controlY = field.controlBaseY - offset;
        int sliderY = field.sliderBaseY - offset;
        bool show = active && fullyVisibleAt(controlY, 28);
        MoveConfigChildY(field.label, labelY);
        MoveConfigChildY(field.control, controlY);
        ShowWindow(field.label, show ? SW_SHOW : SW_HIDE);
        ShowWindow(field.control, show ? SW_SHOW : SW_HIDE);
        if (field.slider) {
            MoveConfigChildY(field.slider, sliderY);
            ShowWindow(field.slider, show ? SW_SHOW : SW_HIDE);
        }
    }

    if (state->hwnd) {
        HWND target = state->scrollPane ? state->scrollPane : state->hwnd;
        RedrawWindow(target, nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
    }
}

void ShowConfigTab(ConfigState* state, int tab) {
    if (!state) return;
    tab = std::clamp(tab, 0, std::max(0, state->tabCount - 1));
    state->activeTab = tab;
    if (!ConfigActiveTabIsMonsterPreview(state) && state->previewOrbitDragging) {
        EndConfigPreviewOrbit(state, GetCapture());
    }
    if (state->note && tab < static_cast<int>(state->tabNotes.size())) SetWindowTextW(state->note, state->tabNotes[static_cast<size_t>(tab)].c_str());
    ApplyConfigScroll(state);
    state->previewPending = false;
    UpdateConfigPreviewHint(state);
    if (!state->previewRenderer) {
        SetConfigPreviewStatus(state, L"Click Update preview to render current settings.");
    } else {
        SetConfigPreviewStatus(state, L"");
    }
}

LRESULT CALLBACK ConfigPreviewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }

    ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_SIZE:
        if (state && state->previewRenderer) {
            state->previewRenderer->Resize(LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (state && ConfigActiveTabIsMonsterPreview(state)) {
            BeginConfigPreviewOrbit(state, hwnd, ConfigClientPointToScreen(hwnd, lParam));
            return 0;
        }
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (state && state->previewOrbitDragging && GetCapture() == hwnd) {
            EndConfigPreviewOrbit(state, hwnd);
            return 0;
        }
        break;
    case WM_CAPTURECHANGED:
        if (state && reinterpret_cast<HWND>(lParam) != hwnd) {
            state->previewOrbitDragging = false;
        }
        break;
    case WM_MOUSEMOVE:
        if (state && state->previewOrbitDragging && ConfigActiveTabIsMonsterPreview(state)) {
            UpdateConfigPreviewOrbit(state, ConfigClientPointToScreen(hwnd, lParam));
            return 0;
        }
        break;
    case WM_MOUSEWHEEL:
        if (state && ConfigActiveTabIsMonsterPreview(state) && state->previewRenderer) {
            ZoomConfigPreviewOrbit(state, wParam);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ConfigScrollPaneWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_ERASEBKGND: {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        FillRect(reinterpret_cast<HDC>(wParam), &rc, GetSysColorBrush(COLOR_WINDOW));
        return 1;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ConfigWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_CREATE: {
        HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        state = new ConfigState();
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto* params = cs ? reinterpret_cast<ConfigCreateParams*>(cs->lpCreateParams) : nullptr;
        if (params) {
            state->mode = params->mode;
            state->embedded = params->embedded;
        }
        state->hwnd = hwnd;
        BuildConfigModel(state);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));

        state->tab = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            12, 12, 720, 628, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigTabId)), nullptr, nullptr);
        SendMessageW(state->tab, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        for (int i = 0; i < state->tabCount; ++i) {
            TCITEMW item{};
            item.mask = TCIF_TEXT;
            item.pszText = const_cast<wchar_t*>(state->tabLabels[static_cast<size_t>(i)].c_str());
            TabCtrl_InsertItem(state->tab, i, &item);
        }

        state->note = CreateWindowW(L"STATIC", state->tabNotes.empty() ? L"" : state->tabNotes[0].c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
            30, 50, 660, 34, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(state->note, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->scrollPane = CreateWindowExW(0, L"BackroomsMazeConfigScrollPane", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            0, kConfigContentTop, 714, ConfigVisibleHeight(), hwnd, nullptr, nullptr, nullptr);

        constexpr int kColumnCount = 2;
        constexpr int kColumnX[kColumnCount] = {26, 376};
        constexpr int kLabelW = 160;
        constexpr int kControlOffset = 170;
        std::array<std::array<int, kColumnCount>, kConfigMaxTabCount> y{};
        std::array<std::array<const wchar_t*, kColumnCount>, kConfigMaxTabCount> group{};
        for (auto& tabY : y) {
            tabY.fill(0);
        }

        for (const auto& def : state->fieldDefs) {
            size_t tabIndex = static_cast<size_t>(def.tab);
            size_t colIndex = static_cast<size_t>(def.column);
            if (tabIndex >= static_cast<size_t>(state->tabCount) || colIndex >= static_cast<size_t>(kColumnCount)) continue;
            if (group[tabIndex][colIndex] != def.group) {
                if (y[tabIndex][colIndex] > 0) y[tabIndex][colIndex] += 10;
                HWND header = CreateWindowW(L"STATIC", def.group, WS_CHILD | SS_LEFT,
                    kColumnX[colIndex], y[tabIndex][colIndex], 320, 22, state->scrollPane, nullptr, nullptr, nullptr);
                SendMessageW(header, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
                state->headers.push_back({def.tab, header, y[tabIndex][colIndex]});
                group[tabIndex][colIndex] = def.group;
                y[tabIndex][colIndex] += 26;
            }

            int fieldY = y[tabIndex][colIndex];
            y[tabIndex][colIndex] += 30;
            HWND label = CreateWindowW(L"STATIC", def.label, WS_CHILD | SS_RIGHT | SS_NOPREFIX | SS_ENDELLIPSIS,
                kColumnX[colIndex], fieldY + 4, kLabelW, 22, state->scrollPane, nullptr, nullptr, nullptr);
            HWND control = nullptr;
            if (def.kind == ConfigFieldKind::Bool) {
                control = CreateWindowW(L"BUTTON", L"", WS_CHILD | BS_AUTOCHECKBOX,
                    kColumnX[colIndex] + kControlOffset, fieldY + 2, 24, 22, state->scrollPane, reinterpret_cast<HMENU>(static_cast<INT_PTR>(def.id)), nullptr, nullptr);
            } else {
                control = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL,
                    kColumnX[colIndex] + kControlOffset, fieldY, def.width, 24, state->scrollPane, reinterpret_cast<HMENU>(static_cast<INT_PTR>(def.id)), nullptr, nullptr);
            }
            HWND slider = nullptr;
            if (IsEyeConfigField(def)) {
                int editX = kColumnX[colIndex] + kControlOffset;
                slider = CreateWindowExW(0, TRACKBAR_CLASSW, L"",
                    WS_CHILD | TBS_HORZ | TBS_NOTICKS,
                    editX + def.width + 8, fieldY - 1, 116, 26,
                    state->scrollPane, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigEyeSliderBaseId + def.id - kConfigFieldBaseId)), nullptr, nullptr);
                SendMessageW(slider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 1000));
                SendMessageW(slider, TBM_SETPAGESIZE, 0, 20);
                SendMessageW(slider, TBM_SETLINESIZE, 0, 4);
            }
            SendMessageW(label, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            state->fields.push_back({&def, label, control, slider, fieldY + 4, fieldY, fieldY - 1});
        }
        for (int tab = 0; tab < state->tabCount; ++tab) {
            int bottom = std::max(y[static_cast<size_t>(tab)][0], y[static_cast<size_t>(tab)][1]) + 8;
            state->contentHeight[static_cast<size_t>(tab)] = std::max(ConfigVisibleHeight(), bottom);
        }
        state->scrollBar = CreateWindowExW(0, L"SCROLLBAR", L"",
            WS_CHILD | SBS_VERT, 714, kConfigContentTop, 18, ConfigVisibleHeight(),
            hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigScrollId)), nullptr, nullptr);

        LoadConfigControls(state, false);
        ShowConfigTab(state, 0);

        HWND previewTitle = CreateWindowW(L"STATIC", L"Live preview", WS_CHILD | WS_VISIBLE | SS_LEFT,
            754, 16, 430, 22, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(previewTitle, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->preview = CreateWindowExW(WS_EX_CLIENTEDGE, L"BackroomsMazeConfigPreview", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 754, 44, 430, 300,
            hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigPreviewId)), nullptr, state);
        state->previewStatus = CreateWindowW(L"STATIC", L"", WS_CHILD | SS_CENTER,
            774, 174, 390, 24, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(state->previewStatus, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->previewHint = CreateWindowW(L"STATIC", L"Edit several values, then click Update preview to rebuild the embedded screensaver once.",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 754, 354, 430, 42, hwnd, nullptr, nullptr, nullptr);
        SendMessageW(state->previewHint, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        state->previewUpdateButton = CreateWindowW(L"BUTTON", L"Update preview", WS_CHILD | WS_VISIBLE,
            754, 404, 128, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigPreviewUpdateId)), nullptr, nullptr);
        SendMessageW(state->previewUpdateButton, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SetTimer(hwnd, kConfigPreviewTimerId, 16, nullptr);
        UpdateConfigPreviewHint(state);
        SetConfigPreviewStatus(state, L"Click Update preview to render current settings.");

        HWND save = CreateWindowW(L"BUTTON", L"Save", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
            12, 660, 96, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigSaveId)), nullptr, nullptr);
        HWND reset = CreateWindowW(L"BUTTON", L"Reset defaults", WS_CHILD | WS_VISIBLE,
            116, 660, 126, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigResetId)), nullptr, nullptr);
        HWND open = CreateWindowW(L"BUTTON", L"Open INI", WS_CHILD | WS_VISIBLE,
            250, 660, 104, 30, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConfigOpenId)), nullptr, nullptr);
        HWND close = CreateWindowW(L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
            628, 660, 104, 30, hwnd, reinterpret_cast<HMENU>(IDCANCEL), nullptr, nullptr);
        SendMessageW(save, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(reset, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(open, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        SendMessageW(close, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        return 0;
    }
    case WM_NOTIFY:
        if (state && reinterpret_cast<NMHDR*>(lParam)->idFrom == kConfigTabId &&
            reinterpret_cast<NMHDR*>(lParam)->code == TCN_SELCHANGE) {
            ShowConfigTab(state, TabCtrl_GetCurSel(state->tab));
        }
        return 0;
    case WM_TIMER:
        if (state && wParam == kConfigPreviewTimerId) {
            if (state->previewPending && GetTickCount64() >= state->previewApplyAt) {
                RestartConfigPreview(state);
            }
            if (state->previewRenderer) state->previewRenderer->Tick();
            return 0;
        }
        break;
    case WM_VSCROLL:
        if (state && reinterpret_cast<HWND>(lParam) == state->scrollBar) {
            int offset = state->scrollOffset[static_cast<size_t>(state->activeTab)];
            int maxScroll = ConfigMaxScroll(state, state->activeTab);
            switch (LOWORD(wParam)) {
            case SB_LINEUP: offset -= kConfigScrollStep; break;
            case SB_LINEDOWN: offset += kConfigScrollStep; break;
            case SB_PAGEUP: offset -= ConfigVisibleHeight() - kConfigScrollStep; break;
            case SB_PAGEDOWN: offset += ConfigVisibleHeight() - kConfigScrollStep; break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION: {
                SCROLLINFO si{};
                si.cbSize = sizeof(si);
                si.fMask = SIF_TRACKPOS;
                GetScrollInfo(state->scrollBar, SB_CTL, &si);
                offset = si.nTrackPos;
                break;
            }
            case SB_TOP: offset = 0; break;
            case SB_BOTTOM: offset = maxScroll; break;
            default: break;
            }
            SetConfigScrollOffset(state, offset);
            ApplyConfigScroll(state);
            return 0;
        }
        break;
    case WM_MOUSEWHEEL:
        if (state && ConfigActiveTabIsMonsterPreview(state) && state->previewRenderer) {
            POINT screenPoint{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (ConfigPointOverPreview(state, screenPoint)) {
                ZoomConfigPreviewOrbit(state, wParam);
                return 0;
            }
        }
        if (state && ConfigMaxScroll(state, state->activeTab) > 0) {
            int detents = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            int offset = state->scrollOffset[static_cast<size_t>(state->activeTab)] - detents * kConfigScrollStep;
            SetConfigScrollOffset(state, offset);
            ApplyConfigScroll(state);
            return 0;
        }
        break;
    case WM_HSCROLL:
        if (state && reinterpret_cast<HWND>(lParam)) {
            ConfigFieldUi* field = FindConfigFieldBySlider(state, reinterpret_cast<HWND>(lParam));
            if (field && field->def && field->control) {
                int pos = static_cast<int>(SendMessageW(field->slider, TBM_GETPOS, 0, 0));
                std::wstring value = FormatConfigFloat(EyeValueFromSliderPos(*field->def, pos));
                state->loadingControls = true;
                SetWindowTextW(field->control, value.c_str());
                state->loadingControls = false;
                SaveConfigFieldControl(*field);
                ApplyConfigPreviewEyeCalibration(state);
                return 0;
            }
        }
        break;
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
        if (state && ConfigActiveTabIsMonsterPreview(state)) {
            POINT screenPoint = ConfigClientPointToScreen(hwnd, lParam);
            if (ConfigPointOverPreview(state, screenPoint)) {
                BeginConfigPreviewOrbit(state, hwnd, screenPoint);
                return 0;
            }
        }
        break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
        if (state && state->previewOrbitDragging && GetCapture() == hwnd) {
            EndConfigPreviewOrbit(state, hwnd);
            return 0;
        }
        break;
    case WM_CAPTURECHANGED:
        if (state && state->previewOrbitDragging && reinterpret_cast<HWND>(lParam) != hwnd && GetCapture() != state->preview) {
            state->previewOrbitDragging = false;
        }
        break;
    case WM_MOUSEMOVE:
        if (state && state->previewOrbitDragging && ConfigActiveTabIsMonsterPreview(state) && GetCapture() == hwnd) {
            UpdateConfigPreviewOrbit(state, ConfigClientPointToScreen(hwnd, lParam));
            return 0;
        }
        break;
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == kConfigSaveId && state) {
            SaveConfigControls(state);
            if (state->mode == ConfigDialogMode::Debug && gApp && gApp->rendererInitialized) {
                gApp->renderer.ApplyGameSettings(LoadSettings());
            }
            const wchar_t* message = state->mode == ConfigDialogMode::Game
                ? L"Game settings saved. Display settings apply next launch; start a new run to reload level-generation settings."
                : (state->mode == ConfigDialogMode::Debug
                    ? L"Debug settings saved. Re-enter Debug or update the preview to reload scene-generation settings."
                    : L"Settings saved. Restart the screensaver to use changed values.");
            MessageBoxW(hwnd, message, L"Backrooms Maze", MB_OK | MB_ICONINFORMATION);
            return 0;
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
            return 0;
        }
        if (id == kConfigOpenId) {
            std::wstring arg = L"\"" + SettingsPath().wstring() + L"\"";
            ShellExecuteW(hwnd, L"open", L"notepad.exe", arg.c_str(), nullptr, SW_SHOWNORMAL);
            return 0;
        }
        if (id == kConfigPreviewUpdateId && state) {
            RestartConfigPreview(state);
            return 0;
        }
        if (id == IDCANCEL) {
            DestroyWindow(hwnd);
            return 0;
        }
        if (state) {
            ConfigFieldUi* field = FindConfigFieldById(state, id);
            WORD code = HIWORD(wParam);
            if (field && (code == EN_CHANGE || code == BN_CLICKED)) {
                if (IsEyeConfigField(*field->def) && code == EN_CHANGE) {
                    SyncEyeSliderFromEdit(*field);
                    if (!state->loadingControls) {
                        SaveConfigFieldControl(*field);
                        ApplyConfigPreviewEyeCalibration(state);
                    }
                } else {
                    ScheduleConfigPreview(state);
                }
                return 0;
            }
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (state) {
            KillTimer(hwnd, kConfigPreviewTimerId);
            state->previewRenderer.reset();
        }
        if (state && state->embedded) {
            if (gApp && gApp->gameConfig == hwnd) {
                gApp->gameConfig = nullptr;
                PostMessageW(gApp->hwnd, kGameConfigClosedMessage, 0, 0);
            }
        } else {
            PostQuitMessage(0);
        }
        return 0;
    case WM_NCDESTROY:
        delete state;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void ShowConfig(HWND owner, ConfigDialogMode mode) {
    EnsureSettingsFile();
    INITCOMMONCONTROLSEX commonControls{sizeof(commonControls), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&commonControls);

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"BackroomsMazeConfigWindow";
    const wchar_t* previewCls = L"BackroomsMazeConfigPreview";
    const wchar_t* scrollPaneCls = L"BackroomsMazeConfigScrollPane";

    WNDCLASSW previewWc{};
    previewWc.lpfnWndProc = ConfigPreviewWndProc;
    previewWc.hInstance = hInstance;
    previewWc.lpszClassName = previewCls;
    previewWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    previewWc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&previewWc);

    WNDCLASSW scrollPaneWc{};
    scrollPaneWc.lpfnWndProc = ConfigScrollPaneWndProc;
    scrollPaneWc.hInstance = hInstance;
    scrollPaneWc.lpszClassName = scrollPaneCls;
    scrollPaneWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    scrollPaneWc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&scrollPaneWc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = ConfigWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    const wchar_t* title = mode == ConfigDialogMode::Game
        ? L"Backrooms Maze Game Settings"
        : (mode == ConfigDialogMode::Debug ? L"Backrooms Maze Debug Settings" : L"Backrooms Maze Configuration");
    ConfigCreateParams params{mode, false};
    HWND hwnd = CreateWindowExW(WS_EX_DLGMODALFRAME, cls, title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1220, 735,
        owner, nullptr, hInstance, &params);
    if (!hwnd) return;
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        ConfigState* state = reinterpret_cast<ConfigState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (state && msg.message == WM_MOUSEWHEEL) {
            POINT screenPoint{GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam)};
            if (ConfigPointOverPreview(state, screenPoint)) {
                ZoomConfigPreviewOrbit(state, msg.wParam);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

#if defined(BACKROOMS_GAME_EXE)
#include "../game/game_settings_panel.inl"
#endif

HWND CreateEmbeddedConfig(HWND parent, ConfigDialogMode mode) {
    EnsureSettingsFile();
    INITCOMMONCONTROLSEX commonControls{sizeof(commonControls), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
    InitCommonControlsEx(&commonControls);

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t* cls = L"BackroomsMazeConfigWindow";
    const wchar_t* previewCls = L"BackroomsMazeConfigPreview";
    const wchar_t* scrollPaneCls = L"BackroomsMazeConfigScrollPane";

    WNDCLASSW previewWc{};
    previewWc.lpfnWndProc = ConfigPreviewWndProc;
    previewWc.hInstance = hInstance;
    previewWc.lpszClassName = previewCls;
    previewWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    previewWc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&previewWc);

    WNDCLASSW scrollPaneWc{};
    scrollPaneWc.lpfnWndProc = ConfigScrollPaneWndProc;
    scrollPaneWc.hInstance = hInstance;
    scrollPaneWc.lpszClassName = scrollPaneCls;
    scrollPaneWc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    scrollPaneWc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&scrollPaneWc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = ConfigWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    ConfigCreateParams params{mode, true};
    return CreateWindowExW(0, cls, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, 10, 10, parent, nullptr, hInstance, &params);
}
