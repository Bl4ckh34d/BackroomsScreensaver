// Config dialog mode-specific model builders.

struct ConfigCreateParams {
    ConfigDialogMode mode = ConfigDialogMode::Full;
    bool embedded = false;
};

const ConfigFieldDef* FindBaseConfigField(const wchar_t* section, const wchar_t* key) {
    for (const auto& def : kConfigFields) {
        if (std::wcscmp(def.section, section) == 0 && std::wcscmp(def.key, key) == 0) return &def;
    }
    return nullptr;
}

void AddConfigFieldCopy(std::vector<ConfigFieldDef>& fields, const wchar_t* section, const wchar_t* key,
    int tab, int column, const wchar_t* group, const wchar_t* labelOverride = nullptr) {
    const ConfigFieldDef* base = FindBaseConfigField(section, key);
    if (!base) return;
    ConfigFieldDef copy = *base;
    copy.tab = tab;
    copy.column = column;
    copy.group = group;
    if (labelOverride) copy.label = labelOverride;
    fields.push_back(copy);
}

void AddCustomConfigField(std::vector<ConfigFieldDef>& fields, int tab, int column, int id,
    const wchar_t* group, const wchar_t* section, const wchar_t* key, const wchar_t* label,
    const wchar_t* fallback, ConfigFieldKind kind, int width) {
    fields.push_back({tab, column, id, group, section, key, label, fallback, kind, width});
}

void BuildFullConfigModel(ConfigState* state) {
    state->tabLabels.assign(std::begin(kConfigTabs), std::end(kConfigTabs));
    state->tabNotes.assign(std::begin(kConfigNotes), std::end(kConfigNotes));
    state->fieldDefs.assign(std::begin(kConfigFields), std::end(kConfigFields));
}

void BuildGameConfigModel(ConfigState* state) {
    state->tabLabels = {L"System", L"Graphics", L"Game", L"Controls", L"Audio"};
    state->tabNotes = {
        L"Game runtime and launch policy. These settings apply to the playable executable and shared INI.",
        L"Rendering, textures, lighting, post processing, particles, and visual atmosphere.",
        L"Maze generation, scare density, monster pressure, dread, and exit pacing.",
        L"Manual player control tuning and persisted key bindings.",
        L"Audio settings control the in-game XAudio2 mix buses."
    };

    auto& f = state->fieldDefs;
    AddCustomConfigField(f, 0, 0, kConfigGameFullscreenId, L"Display", L"GameWindow", L"Fullscreen", L"Fullscreen", L"1", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 0, 0, kConfigGameResolutionWidthId, L"Display", L"GameWindow", L"ResolutionWidth", L"Resolution width", L"1920", ConfigFieldKind::Text, 90);
    AddCustomConfigField(f, 0, 0, kConfigGameResolutionHeightId, L"Display", L"GameWindow", L"ResolutionHeight", L"Resolution height", L"1080", ConfigFieldKind::Text, 90);
    AddConfigFieldCopy(f, L"Renderer", L"AllowWarpFallback", 0, 1, L"System");
    AddConfigFieldCopy(f, L"Randomization", L"RunVariation", 0, 0, L"Runtime");
    AddConfigFieldCopy(f, L"Maze", L"RandomSeed", 0, 1, L"Runtime");

    const wchar_t* textureKeys[] = {L"AssetFolder", L"WallStem", L"FloorStem", L"CeilingStem", L"FleshStem",
        L"WallScaleMeters", L"FloorScaleMeters", L"CeilingScaleMeters", L"UseExternalNormals", L"MaxNormalMapMB"};
    for (const wchar_t* key : textureKeys) AddConfigFieldCopy(f, L"Textures", key, 1, key == textureKeys[0] || key == textureKeys[1] || key == textureKeys[2] || key == textureKeys[3] || key == textureKeys[4] ? 0 : 1, L"Textures");
    const wchar_t* lightingKeys[] = {L"FlashlightIntensity", L"FlashlightAttenuation", L"FlashlightConeDegrees", L"AmbientLight",
        L"FlashlightShadows", L"FlashlightShadowStrength", L"FlashlightShadowDistanceMeters", L"FlashlightShadowBias", L"FlashlightShadowMapSize",
        L"LampIntensity", L"LampSpacing", L"LampOnRatio", L"LampFlickerRatio", L"BrokenZoneRatio", L"DarkLampVisibleRatio",
        L"FogStartMeters", L"FogEndMeters", L"FogDarkness", L"CornerAOIntensity", L"CornerAORadius", L"FloorCeilingAOIntensity",
        L"Exposure", L"Gamma", L"MotionBlurAmount", L"BloomAmount", L"LensDirtAmount"};
    for (const wchar_t* key : lightingKeys) AddConfigFieldCopy(f, L"Lighting", key, 1,
        (std::wcsstr(key, L"Lamp") || std::wcsstr(key, L"AO")) ? 1 : 0,
        std::wcsstr(key, L"Exposure") || std::wcsstr(key, L"Gamma") || std::wcsstr(key, L"Blur") || std::wcsstr(key, L"Bloom") || std::wcsstr(key, L"Lens")
            ? L"Post Processing" : L"Lighting");
    const wchar_t* visualAtmosphereKeys[] = {L"SparkParticles", L"SparkEmitterRatio", L"SparkMaxParticles", L"SparkSize",
        L"AirParticles", L"AirParticleDensity", L"AirParticleSize", L"AirParticleBlur",
        L"BloodWetness", L"BloodShaderQuality", L"FleshWetness", L"FleshParallaxScale"};
    for (const wchar_t* key : visualAtmosphereKeys) AddConfigFieldCopy(f, L"Atmosphere", key, 1, std::wcsstr(key, L"Air") ? 1 : 0, L"Visual Effects");

    const wchar_t* mazeKeys[] = {L"Width", L"Height", L"TileWidthMeters", L"TileLengthMeters", L"WallHeightMeters",
        L"RoomCount", L"RoomMinRadius", L"RoomMaxRadius", L"RoomCountRange", L"RoomMinRadiusRange", L"RoomMaxRadiusRange"};
    for (const wchar_t* key : mazeKeys) AddConfigFieldCopy(f, L"Maze", key, 2, std::wcsstr(key, L"Room") ? 1 : 0, std::wcsstr(key, L"Room") ? L"Rooms" : L"Maze");
    const wchar_t* gameplayAtmosphereKeys[] = {L"PaperDensity", L"HallwayPaperRunDensity", L"ChairDensity", L"WaterDamageEnabled", L"WaterDamageDensity",
        L"MetalCabinetDensity", L"JumpscareFrequency", L"BloodSplatterDensity", L"BloodBurstCount",
        L"BloodStreamCount", L"BloodStreamThickness", L"BloodWorldFlicker", L"BloodWorldAlwaysOn", L"FleshFlicker", L"FleshAlwaysOn"};
    for (const wchar_t* key : gameplayAtmosphereKeys) AddConfigFieldCopy(f, L"Atmosphere", key, 2, std::wcsstr(key, L"Blood") || std::wcsstr(key, L"Flesh") ? 1 : 0, L"Scares / Clutter");
    const wchar_t* monsterGameplayKeys[] = {L"MonsterScale", L"MonsterSpeed", L"MonsterSprintSpeed", L"MonsterIgnorePlayer", L"MonsterKillDistance", L"MonsterVisibleDistance"};
    for (const wchar_t* key : monsterGameplayKeys) AddConfigFieldCopy(f, L"Monster", key, 2, 0, L"Monster");
    const wchar_t* dreadKeys[] = {L"Enabled", L"DecayPerSecond", L"MonsterDistance", L"MonsterGainPerSecond",
        L"JumpscareGain", L"FleshGain", L"WalkSpeedBoost", L"RunSpeedBoost", L"FlashlightFlicker"};
    for (const wchar_t* key : dreadKeys) AddConfigFieldCopy(f, L"Dread", key, 2, 1, L"Dread");
    const wchar_t* exitKeys[] = {L"ExitDoorOpenSeconds", L"ExitStepSeconds", L"ExitFadeSeconds", L"ExitStepDistance", L"FadeInSeconds"};
    for (const wchar_t* key : exitKeys) AddConfigFieldCopy(f, L"CameraFX", key, 2, 0, L"Exit");

    AddConfigFieldCopy(f, L"CameraAI", L"WalkSpeed", 3, 0, L"Movement");
    AddConfigFieldCopy(f, L"CameraAI", L"RunSpeed", 3, 0, L"Movement");
    AddConfigFieldCopy(f, L"CameraAI", L"HeadBobAmount", 3, 0, L"Movement");
    AddConfigFieldCopy(f, L"CameraAI", L"SideSwayAmount", 3, 0, L"Movement");
    AddCustomConfigField(f, 3, 0, kConfigMouseSensitivityId, L"Mouse", L"Controls", L"MouseSensitivity", L"Mouse sensitivity", L"1", ConfigFieldKind::Text, 90);
    AddCustomConfigField(f, 3, 0, kConfigInvertMouseYId, L"Mouse", L"Controls", L"InvertMouseY", L"Invert Y axis", L"0", ConfigFieldKind::Bool, 0);
    AddConfigFieldCopy(f, L"CameraFX", L"FlashlightSwayAmount", 3, 1, L"Flashlight");
    AddConfigFieldCopy(f, L"CameraFX", L"FlashlightFollowSpeed", 3, 1, L"Flashlight");
    AddConfigFieldCopy(f, L"CameraFX", L"FlashlightPanicDartAmount", 3, 1, L"Flashlight");

    AddCustomConfigField(f, 4, 0, kConfigAudioMutedId, L"Master", L"Audio", L"Muted", L"Mute audio", L"0", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 4, 0, kConfigAudioMasterVolumeId, L"Master", L"Audio", L"MasterVolume", L"Master volume", L"1", ConfigFieldKind::Text, 90);
    AddCustomConfigField(f, 4, 0, kConfigAudioEffectsVolumeId, L"Mix", L"Audio", L"EffectsVolume", L"Effects volume", L"1", ConfigFieldKind::Text, 90);
    AddCustomConfigField(f, 4, 0, kConfigAudioAmbienceVolumeId, L"Mix", L"Audio", L"AmbienceVolume", L"Ambience volume", L"1", ConfigFieldKind::Text, 90);
    AddCustomConfigField(f, 4, 0, kConfigAudioMonsterVolumeId, L"Mix", L"Audio", L"MonsterVolume", L"Monster volume", L"1", ConfigFieldKind::Text, 90);
}

void BuildDebugConfigModel(ConfigState* state) {
    state->tabLabels = {L"Debug View", L"Effects", L"Autopilot", L"Monster"};
    state->tabNotes = {
        L"Debug-only overlays and forced study views.",
        L"Effect loop and stress-test tuning used by the debug scene.",
        L"Screensaver/autopilot camera behavior, useful for reusing movement logic in monster AI.",
        L"Monster mesh and orientation. Right-drag the preview to orbit and use the wheel to zoom."
    };

    auto& f = state->fieldDefs;
    AddConfigFieldCopy(f, L"Maze", L"MapOverlay", 0, 0, L"Overlays");
    AddConfigFieldCopy(f, L"Dread", L"DebugMeter", 0, 0, L"Overlays");
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 176, L"Overlays", L"Debug", L"AiMapOverlay", L"AI minimap overlay", L"0", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 177, L"Cheats", L"Debug", L"InfiniteStamina", L"Infinite stamina", L"0", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 178, L"Cheats", L"Debug", L"Invincible", L"Invincible", L"0", ConfigFieldKind::Bool, 0);
    AddConfigFieldCopy(f, L"Atmosphere", L"BloodStudyView", 0, 1, L"Forced Views");

    const wchar_t* effectKeys[] = {L"BloodLoopSeconds", L"BloodFullSpreadAge", L"WaterLoopSeconds", L"AirVentLoopSeconds",
        L"BrokenLampLoopSeconds", L"StaticLoopSeconds", L"BrokenLampSparkIntensityMin", L"BrokenLampSparkIntensityMax",
        L"BrokenLampChainIntensityScale", L"BrokenLampChainBurstsMin", L"BrokenLampChainBurstsMax",
        L"AirVentSteamIntensityMin", L"AirVentSteamIntensityMax", L"AirVentPanelDropEvery", L"AirVentPanelDropChance"};
    for (const wchar_t* key : effectKeys) AddConfigFieldCopy(f, L"EffectTuning", key, 1,
        std::wcsstr(key, L"AirVent") ? 1 : 0, L"Effect Tuning");

    const wchar_t* autopilotKeys[] = {L"RoomSpeed", L"TurnLookAheadTiles", L"RoomLookAheadTiles", L"RoomPauseChance",
        L"JunctionScanChance", L"ScanAngleDegrees", L"LookBackMinSeconds", L"LookBackMaxSeconds",
        L"JunctionScanBaseSeconds", L"JunctionScanBranchSeconds"};
    for (const wchar_t* key : autopilotKeys) AddConfigFieldCopy(f, L"CameraAI", key, 2,
        std::wcsstr(key, L"Look") || std::wcsstr(key, L"Scan") || std::wcsstr(key, L"Junction") ? 1 : 0, L"Autopilot");

    const wchar_t* monsterKeys[] = {L"SkullMesh", L"AlternateSkullMesh", L"AlternateSkullChance", L"SkullMaxTriangles",
        L"SkullYawDegrees", L"SkullPitchDegrees", L"SkullRollDegrees",
        L"AlternateSkullYawDegrees", L"AlternateSkullPitchDegrees", L"AlternateSkullRollDegrees"};
    for (const wchar_t* key : monsterKeys) {
        AddConfigFieldCopy(f, L"Monster", key, 3, 0, L"Meshes / Orientation");
    }
}

void BuildConfigModel(ConfigState* state) {
    if (!state) return;
    state->fieldDefs.clear();
    state->tabLabels.clear();
    state->tabNotes.clear();
    if (state->mode == ConfigDialogMode::Game) {
        BuildGameConfigModel(state);
    } else if (state->mode == ConfigDialogMode::Debug) {
        BuildDebugConfigModel(state);
    } else {
        BuildFullConfigModel(state);
    }
    state->tabCount = static_cast<int>(std::min<size_t>(state->tabLabels.size(), kConfigMaxTabCount));
}
