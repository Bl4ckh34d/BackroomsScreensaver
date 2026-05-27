constexpr int kConfigTabId = 2001;
constexpr int kConfigSaveId = 2002;
constexpr int kConfigResetId = 2003;
constexpr int kConfigOpenId = 2004;
constexpr int kConfigPreviewId = 2005;
constexpr int kConfigPreviewTimerId = 2006;
constexpr int kConfigScrollId = 2007;
constexpr int kConfigPreviewUpdateId = 2008;
constexpr int kConfigFieldBaseId = 3000;
constexpr int kConfigEyeSliderBaseId = 4200;
constexpr int kConfigMaxTabCount = 8;
constexpr int kConfigContentTop = 92;
constexpr int kConfigContentBottom = 646;
constexpr int kConfigScrollStep = 42;
constexpr int kConfigMouseSensitivityId = kConfigFieldBaseId + 166;
constexpr int kConfigInvertMouseYId = kConfigFieldBaseId + 167;
constexpr int kConfigAudioMutedId = kConfigFieldBaseId + 168;
constexpr int kConfigAudioMasterVolumeId = kConfigFieldBaseId + 169;
constexpr int kConfigAudioEffectsVolumeId = kConfigFieldBaseId + 170;
constexpr int kConfigAudioAmbienceVolumeId = kConfigFieldBaseId + 171;
constexpr int kConfigAudioMonsterVolumeId = kConfigFieldBaseId + 172;
constexpr int kConfigGameFullscreenId = kConfigFieldBaseId + 173;
constexpr int kConfigGameResolutionWidthId = kConfigFieldBaseId + 174;
constexpr int kConfigGameResolutionHeightId = kConfigFieldBaseId + 175;

enum class ConfigFieldKind {
    Text,
    Bool
};

struct ConfigFieldDef {
    int tab;
    int column;
    int id;
    const wchar_t* group;
    const wchar_t* section;
    const wchar_t* key;
    const wchar_t* label;
    const wchar_t* fallback;
    ConfigFieldKind kind;
    int width;
};

struct ConfigFieldUi {
    const ConfigFieldDef* def = nullptr;
    HWND label = nullptr;
    HWND control = nullptr;
    HWND slider = nullptr;
    int labelBaseY = 0;
    int controlBaseY = 0;
    int sliderBaseY = 0;
};

struct ConfigHeaderUi {
    int tab = 0;
    HWND control = nullptr;
    int baseY = 0;
};

struct ConfigState {
    HWND hwnd = nullptr;
    HWND tab = nullptr;
    HWND note = nullptr;
    HWND scrollPane = nullptr;
    HWND scrollBar = nullptr;
    HWND preview = nullptr;
    HWND previewStatus = nullptr;
    HWND previewHint = nullptr;
    HWND previewUpdateButton = nullptr;
    std::vector<ConfigHeaderUi> headers;
    std::vector<ConfigFieldUi> fields;
    std::vector<ConfigFieldDef> fieldDefs;
    std::vector<std::wstring> tabLabels;
    std::vector<std::wstring> tabNotes;
    std::array<int, kConfigMaxTabCount> scrollOffset{};
    std::array<int, kConfigMaxTabCount> contentHeight{};
    std::unique_ptr<Renderer> previewRenderer;
    ConfigDialogMode mode = ConfigDialogMode::Full;
    int tabCount = 0;
    int activeTab = 0;
    bool loadingControls = false;
    bool previewPending = false;
    ULONGLONG previewApplyAt = 0;
    bool previewOrbitDragging = false;
    bool embedded = false;
    POINT previewLastMouse{};
    bool previewManualOrbit = false;
    float previewOrbitYaw = 0.0f;
    float previewOrbitPitch = -0.18f;
    float previewOrbitDistance = 3.15f;
};

const wchar_t* kConfigTabs[kConfigMaxTabCount] = {
    L"Renderer",
    L"Maze",
    L"Textures",
    L"Lighting",
    L"Camera AI",
    L"Camera FX",
    L"Atmosphere",
    L"Monster"
};

const wchar_t* kConfigNotes[kConfigMaxTabCount] = {
    L"Renderer device policy. Keep WARP disabled to require hardware GPU rendering.",
    L"Maze dimensions are forced odd. Per-run variation jitters Camera AI, Flashlight Motion, and non-blood/flesh Atmosphere values by up to 15%; room +/- fields are explicit integer ranges.",
    L"PBR stems refer to files like <stem>_color_4k.jpg. Empty floor/ceiling stems use built-in textures.",
    L"Flashlight shadows, lamp population, black haze, and procedural corner ambient occlusion.",
    L"Camera pathing, head movement, room scanning, and look-back behavior.",
    L"Independent flashlight motion, entrance fades, and the exit door transition.",
    L"Paper, props, water damage, hanging broken lamps, and spark particles.",
    L"Monster scale, chase logic, dread response, and eye calibration. On this tab, right-drag the preview to orbit and use the wheel to zoom."
};

const ConfigFieldDef kConfigFields[] = {
    {0, 0, kConfigFieldBaseId + 0, L"GPU Device", L"Renderer", L"AllowWarpFallback", L"WARP fallback", L"0", ConfigFieldKind::Bool, 0},

    {1, 0, kConfigFieldBaseId + 1, L"Maze Size", L"Maze", L"Width", L"Width", L"25", ConfigFieldKind::Text, 90},
    {1, 0, kConfigFieldBaseId + 2, L"Maze Size", L"Maze", L"Height", L"Height", L"25", ConfigFieldKind::Text, 90},
    {1, 0, kConfigFieldBaseId + 104, L"Maze Size", L"Maze", L"TileWidthMeters", L"Tile width meters", L"1.6", ConfigFieldKind::Text, 90},
    {1, 0, kConfigFieldBaseId + 105, L"Maze Size", L"Maze", L"TileLengthMeters", L"Tile length meters", L"1.6", ConfigFieldKind::Text, 90},
    {1, 0, kConfigFieldBaseId + 106, L"Maze Size", L"Maze", L"WallHeightMeters", L"Wall/ceiling height", L"2.4", ConfigFieldKind::Text, 90},
    {1, 1, kConfigFieldBaseId + 3, L"Room Generation", L"Maze", L"RoomCount", L"Room count", L"3", ConfigFieldKind::Text, 90},
    {1, 1, kConfigFieldBaseId + 4, L"Room Generation", L"Maze", L"RoomMinRadius", L"Room min radius", L"1.5", ConfigFieldKind::Text, 90},
    {1, 1, kConfigFieldBaseId + 5, L"Room Generation", L"Maze", L"RoomMaxRadius", L"Room max radius", L"3", ConfigFieldKind::Text, 90},
    {1, 1, kConfigFieldBaseId + 113, L"Room Generation", L"Maze", L"RoomCountRange", L"Room count +/-", L"1", ConfigFieldKind::Text, 90},
    {1, 1, kConfigFieldBaseId + 114, L"Room Generation", L"Maze", L"RoomMinRadiusRange", L"Min radius +/-", L"1", ConfigFieldKind::Text, 90},
    {1, 1, kConfigFieldBaseId + 115, L"Room Generation", L"Maze", L"RoomMaxRadiusRange", L"Max radius +/-", L"1", ConfigFieldKind::Text, 90},
    {1, 0, kConfigFieldBaseId + 49, L"Maze Size", L"Maze", L"RandomSeed", L"Random seed (0=random)", L"0", ConfigFieldKind::Text, 90},
    {1, 0, kConfigFieldBaseId + 128, L"Maze Debug", L"Maze", L"MapOverlay", L"Show map overlay", L"1", ConfigFieldKind::Bool, 0},
    {1, 1, kConfigFieldBaseId + 100, L"Randomization", L"Randomization", L"RunVariation", L"Per-run variation", L"0.1", ConfigFieldKind::Text, 90},

    {2, 0, kConfigFieldBaseId + 6, L"Texture Files", L"Textures", L"AssetFolder", L"Asset folder", L"assets\\PBRs", ConfigFieldKind::Text, 170},
    {2, 0, kConfigFieldBaseId + 7, L"Texture Files", L"Textures", L"WallStem", L"Wall PBR stem", L"backrooms_wall", ConfigFieldKind::Text, 150},
    {2, 0, kConfigFieldBaseId + 8, L"Texture Files", L"Textures", L"FloorStem", L"Floor PBR stem", L"backrooms_carpet", ConfigFieldKind::Text, 150},
    {2, 0, kConfigFieldBaseId + 9, L"Texture Files", L"Textures", L"CeilingStem", L"Ceiling PBR stem", L"backrooms_ceiling", ConfigFieldKind::Text, 150},
    {2, 0, kConfigFieldBaseId + 78, L"Texture Files", L"Textures", L"FleshStem", L"Flesh PBR stem", L"downloads\\Others001_4k\\others_0001", ConfigFieldKind::Text, 170},
    {2, 1, kConfigFieldBaseId + 10, L"Texture Scale", L"Textures", L"WallScaleMeters", L"Wall scale meters", L"1.8", ConfigFieldKind::Text, 90},
    {2, 1, kConfigFieldBaseId + 11, L"Texture Scale", L"Textures", L"FloorScaleMeters", L"Floor scale meters", L"1.8", ConfigFieldKind::Text, 90},
    {2, 1, kConfigFieldBaseId + 12, L"Texture Scale", L"Textures", L"CeilingScaleMeters", L"Ceiling scale meters (0=auto 2x2)", L"0", ConfigFieldKind::Text, 90},
    {2, 1, kConfigFieldBaseId + 13, L"Normal Maps", L"Textures", L"UseExternalNormals", L"Use external normal maps", L"1", ConfigFieldKind::Bool, 0},
    {2, 1, kConfigFieldBaseId + 14, L"Normal Maps", L"Textures", L"MaxNormalMapMB", L"Max normal map MB", L"512", ConfigFieldKind::Text, 90},

    {3, 0, kConfigFieldBaseId + 15, L"Flashlight", L"Lighting", L"FlashlightIntensity", L"Flashlight intensity", L"1", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 16, L"Flashlight", L"Lighting", L"FlashlightAttenuation", L"Flashlight attenuation", L"0.115", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 48, L"Flashlight", L"Lighting", L"FlashlightConeDegrees", L"Cone width degrees", L"80", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 17, L"Flashlight", L"Lighting", L"AmbientLight", L"Ambient light", L"0", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 43, L"Flashlight", L"Lighting", L"FlashlightShadows", L"Realtime shadows", L"1", ConfigFieldKind::Bool, 0},
    {3, 0, kConfigFieldBaseId + 44, L"Flashlight", L"Lighting", L"FlashlightShadowStrength", L"Shadow strength", L"0.72", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 45, L"Flashlight", L"Lighting", L"FlashlightShadowDistanceMeters", L"Shadow distance meters", L"18", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 46, L"Flashlight", L"Lighting", L"FlashlightShadowBias", L"Shadow bias", L"0.00075", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 47, L"Flashlight", L"Lighting", L"FlashlightShadowMapSize", L"Shadow map size", L"4096", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 18, L"Ceiling Lamps", L"Lighting", L"LampIntensity", L"Lamp intensity", L"1.45", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 19, L"Ceiling Lamps", L"Lighting", L"LampSpacing", L"Lamp spacing", L"3.2", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 20, L"Ceiling Lamps", L"Lighting", L"LampOnRatio", L"Lamp on ratio", L"0.18", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 21, L"Ceiling Lamps", L"Lighting", L"LampFlickerRatio", L"Lamp flicker ratio", L"0.38", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 22, L"Ceiling Lamps", L"Lighting", L"BrokenZoneRatio", L"Broken zone ratio", L"0.26", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 50, L"Ceiling Lamps", L"Lighting", L"DarkLampVisibleRatio", L"Visible dark fixtures", L"1.0", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 23, L"Fog / Haze", L"Lighting", L"FogStartMeters", L"Fog start meters", L"0", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 24, L"Fog / Haze", L"Lighting", L"FogEndMeters", L"Fog end meters", L"12", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 25, L"Fog / Haze", L"Lighting", L"FogDarkness", L"Fog darkness", L"1", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 26, L"Ambient Occlusion", L"Lighting", L"CornerAOIntensity", L"Corner AO intensity", L"0.45", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 27, L"Ambient Occlusion", L"Lighting", L"CornerAORadius", L"Corner AO radius", L"0.5", ConfigFieldKind::Text, 90},
    {3, 1, kConfigFieldBaseId + 28, L"Ambient Occlusion", L"Lighting", L"FloorCeilingAOIntensity", L"Floor/ceiling AO", L"0.3", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 41, L"Post Processing", L"Lighting", L"Exposure", L"Exposure", L"1", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 42, L"Post Processing", L"Lighting", L"Gamma", L"Gamma", L"1", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 148, L"Post Processing", L"Lighting", L"MotionBlurAmount", L"Motion blur", L"0.18", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 149, L"Post Processing", L"Lighting", L"BloomAmount", L"Bloom", L"0.22", ConfigFieldKind::Text, 90},
    {3, 0, kConfigFieldBaseId + 150, L"Post Processing", L"Lighting", L"LensDirtAmount", L"Lens dirt", L"0.18", ConfigFieldKind::Text, 90},

    {4, 0, kConfigFieldBaseId + 29, L"Movement Speeds", L"CameraAI", L"WalkSpeed", L"Walk speed", L"1.88", ConfigFieldKind::Text, 90},
    {4, 0, kConfigFieldBaseId + 30, L"Movement Speeds", L"CameraAI", L"RoomSpeed", L"Room speed", L"1.45", ConfigFieldKind::Text, 90},
    {4, 0, kConfigFieldBaseId + 31, L"Movement Speeds", L"CameraAI", L"RunSpeed", L"Run speed", L"3.05", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 32, L"Look Ahead", L"CameraAI", L"TurnLookAheadTiles", L"Turn look-ahead tiles", L"2.1", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 33, L"Look Ahead", L"CameraAI", L"RoomLookAheadTiles", L"Room look-ahead tiles", L"2.4", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 34, L"Look Ahead", L"CameraAI", L"RoomPauseChance", L"Room pause chance", L"0.62", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 35, L"Look Ahead", L"CameraAI", L"JunctionScanChance", L"Junction scan chance", L"0.88", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 36, L"Look Ahead", L"CameraAI", L"ScanAngleDegrees", L"Scan angle degrees", L"55", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 54, L"Look Ahead", L"CameraAI", L"JunctionScanBaseSeconds", L"Junction base seconds", L"0.85", ConfigFieldKind::Text, 90},
    {4, 1, kConfigFieldBaseId + 55, L"Look Ahead", L"CameraAI", L"JunctionScanBranchSeconds", L"Junction branch seconds", L"1.08", ConfigFieldKind::Text, 90},
    {4, 0, kConfigFieldBaseId + 37, L"Look Back / Head Motion", L"CameraAI", L"LookBackMinSeconds", L"Look-back min seconds", L"5", ConfigFieldKind::Text, 90},
    {4, 0, kConfigFieldBaseId + 38, L"Look Back / Head Motion", L"CameraAI", L"LookBackMaxSeconds", L"Look-back max seconds", L"90", ConfigFieldKind::Text, 90},
    {4, 0, kConfigFieldBaseId + 39, L"Look Back / Head Motion", L"CameraAI", L"HeadBobAmount", L"Head bob amount", L"0.075", ConfigFieldKind::Text, 90},
    {4, 0, kConfigFieldBaseId + 40, L"Look Back / Head Motion", L"CameraAI", L"SideSwayAmount", L"Side sway amount", L"0.025", ConfigFieldKind::Text, 90},

    {5, 0, kConfigFieldBaseId + 51, L"Flashlight Motion", L"CameraFX", L"FlashlightSwayAmount", L"Independent sway", L"3", ConfigFieldKind::Text, 90},
    {5, 0, kConfigFieldBaseId + 52, L"Flashlight Motion", L"CameraFX", L"FlashlightFollowSpeed", L"Follow speed", L"2", ConfigFieldKind::Text, 90},
    {5, 0, kConfigFieldBaseId + 53, L"Flashlight Motion", L"CameraFX", L"FlashlightPanicDartAmount", L"Panic dart amount", L"3", ConfigFieldKind::Text, 90},
    {5, 1, kConfigFieldBaseId + 56, L"Exit Door Transition", L"CameraFX", L"ExitDoorOpenSeconds", L"Door open seconds", L"1.75", ConfigFieldKind::Text, 90},
    {5, 1, kConfigFieldBaseId + 57, L"Exit Door Transition", L"CameraFX", L"ExitStepSeconds", L"Step-through seconds", L"1.85", ConfigFieldKind::Text, 90},
    {5, 1, kConfigFieldBaseId + 58, L"Exit Door Transition", L"CameraFX", L"ExitFadeSeconds", L"Fade-out seconds", L"1.1", ConfigFieldKind::Text, 90},
    {5, 1, kConfigFieldBaseId + 59, L"Exit Door Transition", L"CameraFX", L"ExitStepDistance", L"Step distance", L"2.35", ConfigFieldKind::Text, 90},
    {5, 1, kConfigFieldBaseId + 60, L"Exit Door Transition", L"CameraFX", L"FadeInSeconds", L"New maze fade-in", L"1.25", ConfigFieldKind::Text, 90},

    {6, 0, kConfigFieldBaseId + 61, L"Clutter Density", L"Atmosphere", L"PaperDensity", L"Loose paper density", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 62, L"Clutter Density", L"Atmosphere", L"HallwayPaperRunDensity", L"Paper hallway density", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 63, L"Clutter Density", L"Atmosphere", L"ChairDensity", L"Chair density", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 66, L"Clutter Density", L"Atmosphere", L"WaterDamageDensity", L"Water damage density", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 117, L"Clutter Density", L"Atmosphere", L"MetalCabinetDensity", L"Metal cabinet density", L"0.85", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 101, L"Scare Timing", L"Atmosphere", L"JumpscareFrequency", L"Jumpscare frequency", L"0.3", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 67, L"Spark Particles", L"Atmosphere", L"SparkParticles", L"Enable sparks", L"1", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 68, L"Spark Particles", L"Atmosphere", L"SparkEmitterRatio", L"Broken lamp spark ratio", L"0.11", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 69, L"Spark Particles", L"Atmosphere", L"SparkBurstMinSeconds", L"Burst min seconds", L"2.8", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 70, L"Spark Particles", L"Atmosphere", L"SparkBurstMaxSeconds", L"Burst max seconds", L"8.8", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 71, L"Spark Particles", L"Atmosphere", L"SparkMaxParticles", L"Max spark particles", L"160", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 72, L"Spark Particles", L"Atmosphere", L"SparkSize", L"Spark size", L"1", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 144, L"Air Particles", L"Atmosphere", L"AirParticles", L"Enable air motes", L"1", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 145, L"Air Particles", L"Atmosphere", L"AirParticleDensity", L"Mote density", L"1", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 146, L"Air Particles", L"Atmosphere", L"AirParticleSize", L"Mote size", L"1", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 147, L"Air Particles", L"Atmosphere", L"AirParticleBlur", L"Focus blur", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 79, L"Blood / Flesh Scares", L"Atmosphere", L"BloodSplatterDensity", L"Blood density", L"0.05", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 80, L"Blood / Flesh Scares", L"Atmosphere", L"BloodBurstCount", L"Paint burst count", L"20", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 81, L"Blood / Flesh Scares", L"Atmosphere", L"BloodWetness", L"Wet reflectivity", L"0.995", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 118, L"Blood / Flesh Scares", L"Atmosphere", L"BloodStreamCount", L"Wall stream count", L"30", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 119, L"Blood / Flesh Scares", L"Atmosphere", L"BloodStreamThickness", L"Stream thickness", L"0.88", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 127, L"Blood / Flesh Scares", L"Atmosphere", L"BloodShaderQuality", L"Blood shader quality", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 120, L"Blood / Flesh Scares", L"Atmosphere", L"BloodWorldCoverage", L"Blood world coverage", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 121, L"Blood / Flesh Scares", L"Atmosphere", L"BloodWorldFlickerIntensity", L"Blood world intensity", L"1", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 122, L"Blood World", L"Atmosphere", L"BloodWorldFlicker", L"Enable blood world", L"0", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 123, L"Blood World", L"Atmosphere", L"BloodWorldAlwaysOn", L"Blood world always on", L"0", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 124, L"Blood World", L"Atmosphere", L"BloodWorldFlickerMinSeconds", L"Flicker min seconds", L"42", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 125, L"Blood World", L"Atmosphere", L"BloodWorldFlickerMaxSeconds", L"Flicker max seconds", L"110", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 126, L"Blood World", L"Atmosphere", L"BloodWorldFlickerDuration", L"Flicker duration", L"1.15", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 129, L"Blood World", L"Atmosphere", L"BloodStudyView", L"Fixed blood study view", L"0", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 82, L"Blood / Flesh Scares", L"Atmosphere", L"FleshFlicker", L"Enable flesh flicker", L"1", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 87, L"Blood / Flesh Scares", L"Atmosphere", L"FleshAlwaysOn", L"Flesh always on", L"0", ConfigFieldKind::Bool, 0},
    {6, 1, kConfigFieldBaseId + 88, L"Blood / Flesh Scares", L"Atmosphere", L"FleshWetness", L"Flesh wetness", L"0.995", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 89, L"Blood / Flesh Scares", L"Atmosphere", L"FleshParallaxScale", L"Flesh relief scale", L"0.14", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 83, L"Blood / Flesh Scares", L"Atmosphere", L"FleshFlickerMinSeconds", L"Flicker min seconds", L"34", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 84, L"Blood / Flesh Scares", L"Atmosphere", L"FleshFlickerMaxSeconds", L"Flicker max seconds", L"92", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 85, L"Blood / Flesh Scares", L"Atmosphere", L"FleshFlickerDuration", L"Flicker duration", L"0.75", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 86, L"Blood / Flesh Scares", L"Atmosphere", L"FleshFlickerIntensity", L"Flicker intensity", L"1", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 151, L"Effect Tuning", L"EffectTuning", L"BloodLoopSeconds", L"Blood loop seconds", L"56", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 152, L"Effect Tuning", L"EffectTuning", L"BloodFullSpreadAge", L"Blood final age", L"48", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 153, L"Effect Tuning", L"EffectTuning", L"WaterLoopSeconds", L"Water loop seconds", L"7.5", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 154, L"Effect Tuning", L"EffectTuning", L"AirVentLoopSeconds", L"Vent loop seconds", L"6.2", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 155, L"Effect Tuning", L"EffectTuning", L"BrokenLampLoopSeconds", L"Lamp loop seconds", L"5.2", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 156, L"Effect Tuning", L"EffectTuning", L"StaticLoopSeconds", L"Static loop seconds", L"8", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 157, L"Effect Tuning", L"EffectTuning", L"BrokenLampSparkIntensityMin", L"Spark min", L"2.2", ConfigFieldKind::Text, 90},
    {6, 0, kConfigFieldBaseId + 158, L"Effect Tuning", L"EffectTuning", L"BrokenLampSparkIntensityMax", L"Spark max", L"4.1", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 159, L"Effect Tuning", L"EffectTuning", L"BrokenLampChainIntensityScale", L"Chain intensity scale", L"0.70", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 160, L"Effect Tuning", L"EffectTuning", L"BrokenLampChainBurstsMin", L"Chain bursts min", L"1", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 161, L"Effect Tuning", L"EffectTuning", L"BrokenLampChainBurstsMax", L"Chain bursts max", L"3", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 162, L"Effect Tuning", L"EffectTuning", L"AirVentSteamIntensityMin", L"Vent steam min", L"1.9", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 163, L"Effect Tuning", L"EffectTuning", L"AirVentSteamIntensityMax", L"Vent steam max", L"3.2", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 164, L"Effect Tuning", L"EffectTuning", L"AirVentPanelDropEvery", L"Debug vent drop every", L"3", ConfigFieldKind::Text, 90},
    {6, 1, kConfigFieldBaseId + 165, L"Effect Tuning", L"EffectTuning", L"AirVentPanelDropChance", L"Runtime vent drop chance", L"0.333333", ConfigFieldKind::Text, 90},

    {7, 0, kConfigFieldBaseId + 73, L"Creature", L"Monster", L"MonsterScale", L"Scale", L"1", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 74, L"Creature", L"Monster", L"MonsterSpeed", L"Base speed multiplier", L"0.68", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 75, L"Creature", L"Monster", L"MonsterSprintSpeed", L"Sprint speed multiplier", L"0.88", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 102, L"Skull Meshes", L"Monster", L"SkullMesh", L"Deer skull mesh", L"assets\\White-Tailed Deer Skull.obj", ConfigFieldKind::Text, 170},
    {7, 0, kConfigFieldBaseId + 130, L"Skull Meshes", L"Monster", L"AlternateSkullMesh", L"Ram skull mesh", L"assets\\models\\Ram_Skull\\Ram_Skull_Scan.OBJ", ConfigFieldKind::Text, 170},
    {7, 0, kConfigFieldBaseId + 131, L"Skull Meshes", L"Monster", L"AlternateSkullChance", L"Ram skull chance", L"0.35", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 103, L"Skull Meshes", L"Monster", L"SkullMaxTriangles", L"Skull mesh triangles", L"18000", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 132, L"Deer Orientation", L"Monster", L"SkullYawDegrees", L"Deer yaw degrees", L"0", ConfigFieldKind::Text, 58},
    {7, 0, kConfigFieldBaseId + 133, L"Deer Orientation", L"Monster", L"SkullPitchDegrees", L"Deer pitch degrees", L"0", ConfigFieldKind::Text, 58},
    {7, 0, kConfigFieldBaseId + 134, L"Deer Orientation", L"Monster", L"SkullRollDegrees", L"Deer roll degrees", L"0", ConfigFieldKind::Text, 58},
    {7, 0, kConfigFieldBaseId + 135, L"Ram Orientation", L"Monster", L"AlternateSkullYawDegrees", L"Ram yaw degrees", L"-90", ConfigFieldKind::Text, 58},
    {7, 0, kConfigFieldBaseId + 136, L"Ram Orientation", L"Monster", L"AlternateSkullPitchDegrees", L"Ram pitch degrees", L"0", ConfigFieldKind::Text, 58},
    {7, 0, kConfigFieldBaseId + 137, L"Ram Orientation", L"Monster", L"AlternateSkullRollDegrees", L"Ram roll degrees", L"90", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 107, L"Deer Eye Calibration", L"Monster", L"RightEyeX", L"Right eye X", L"-0.086", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 108, L"Deer Eye Calibration", L"Monster", L"RightEyeY", L"Right eye Y", L"-0.304", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 109, L"Deer Eye Calibration", L"Monster", L"RightEyeZ", L"Right eye Z", L"0.095", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 110, L"Deer Eye Calibration", L"Monster", L"LeftEyeX", L"Left eye X", L"0.106", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 111, L"Deer Eye Calibration", L"Monster", L"LeftEyeY", L"Left eye Y", L"-0.304", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 112, L"Deer Eye Calibration", L"Monster", L"LeftEyeZ", L"Left eye Z", L"0.095", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 138, L"Ram Eye Calibration", L"Monster", L"AlternateRightEyeX", L"Right eye X", L"-0.086", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 139, L"Ram Eye Calibration", L"Monster", L"AlternateRightEyeY", L"Right eye Y", L"-0.145", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 140, L"Ram Eye Calibration", L"Monster", L"AlternateRightEyeZ", L"Right eye Z", L"0.095", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 141, L"Ram Eye Calibration", L"Monster", L"AlternateLeftEyeX", L"Left eye X", L"0.106", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 142, L"Ram Eye Calibration", L"Monster", L"AlternateLeftEyeY", L"Left eye Y", L"-0.145", ConfigFieldKind::Text, 58},
    {7, 1, kConfigFieldBaseId + 143, L"Ram Eye Calibration", L"Monster", L"AlternateLeftEyeZ", L"Left eye Z", L"0.095", ConfigFieldKind::Text, 58},
    {7, 0, kConfigFieldBaseId + 76, L"Threat Logic", L"Monster", L"MonsterKillDistance", L"Kill distance", L"1.18", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 77, L"Threat Logic", L"Monster", L"MonsterVisibleDistance", L"Visible threat meters", L"12", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 90, L"Dread Meter", L"Dread", L"Enabled", L"Enable dread", L"1", ConfigFieldKind::Bool, 0},
    {7, 0, kConfigFieldBaseId + 91, L"Dread Meter", L"Dread", L"DebugMeter", L"Show debug meter", L"0", ConfigFieldKind::Bool, 0},
    {7, 0, kConfigFieldBaseId + 176, L"Debug Overlay", L"Debug", L"AiMapOverlay", L"AI minimap overlay", L"0", ConfigFieldKind::Bool, 0},
    {7, 0, kConfigFieldBaseId + 92, L"Dread Meter", L"Dread", L"DecayPerSecond", L"Decay per second", L"0.03", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 93, L"Dread Meter", L"Dread", L"MonsterDistance", L"Monster pressure meters", L"13", ConfigFieldKind::Text, 90},
    {7, 0, kConfigFieldBaseId + 94, L"Dread Meter", L"Dread", L"MonsterGainPerSecond", L"Monster gain per sec", L"0.42", ConfigFieldKind::Text, 90},
    {7, 1, kConfigFieldBaseId + 95, L"Dread Response", L"Dread", L"JumpscareGain", L"Jumpscare gain", L"0.1", ConfigFieldKind::Text, 90},
    {7, 1, kConfigFieldBaseId + 96, L"Dread Response", L"Dread", L"FleshGain", L"Flesh flicker gain", L"0.2", ConfigFieldKind::Text, 90},
    {7, 1, kConfigFieldBaseId + 97, L"Dread Response", L"Dread", L"WalkSpeedBoost", L"Walk speed boost", L"0.05", ConfigFieldKind::Text, 90},
    {7, 1, kConfigFieldBaseId + 98, L"Dread Response", L"Dread", L"RunSpeedBoost", L"Run speed boost", L"0.075", ConfigFieldKind::Text, 90},
    {7, 1, kConfigFieldBaseId + 99, L"Dread Response", L"Dread", L"FlashlightFlicker", L"Flashlight flicker", L"1", ConfigFieldKind::Text, 90},
};

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
        L"Audio settings are persisted now; the audio engine will consume them in a later milestone."
    };

    auto& f = state->fieldDefs;
    AddCustomConfigField(f, 0, 0, kConfigGameFullscreenId, L"Display", L"GameWindow", L"Fullscreen", L"Fullscreen", L"0", ConfigFieldKind::Bool, 0);
    AddCustomConfigField(f, 0, 0, kConfigGameResolutionWidthId, L"Display", L"GameWindow", L"ResolutionWidth", L"Resolution width", L"1280", ConfigFieldKind::Text, 90);
    AddCustomConfigField(f, 0, 0, kConfigGameResolutionHeightId, L"Display", L"GameWindow", L"ResolutionHeight", L"Resolution height", L"760", ConfigFieldKind::Text, 90);
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
    const wchar_t* gameplayAtmosphereKeys[] = {L"PaperDensity", L"HallwayPaperRunDensity", L"ChairDensity", L"WaterDamageDensity",
        L"MetalCabinetDensity", L"JumpscareFrequency", L"BloodSplatterDensity", L"BloodBurstCount",
        L"BloodStreamCount", L"BloodStreamThickness", L"BloodWorldFlicker", L"BloodWorldAlwaysOn", L"FleshFlicker", L"FleshAlwaysOn"};
    for (const wchar_t* key : gameplayAtmosphereKeys) AddConfigFieldCopy(f, L"Atmosphere", key, 2, std::wcsstr(key, L"Blood") || std::wcsstr(key, L"Flesh") ? 1 : 0, L"Scares / Clutter");
    const wchar_t* monsterGameplayKeys[] = {L"MonsterScale", L"MonsterSpeed", L"MonsterSprintSpeed", L"MonsterKillDistance", L"MonsterVisibleDistance"};
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
        L"Monster mesh, orientation, and eye calibration. Right-drag the preview to orbit and use the wheel to zoom."
    };

    auto& f = state->fieldDefs;
    AddConfigFieldCopy(f, L"Maze", L"MapOverlay", 0, 0, L"Overlays");
    AddConfigFieldCopy(f, L"Dread", L"DebugMeter", 0, 0, L"Overlays");
    AddCustomConfigField(f, 0, 0, kConfigFieldBaseId + 176, L"Overlays", L"Debug", L"AiMapOverlay", L"AI minimap overlay", L"0", ConfigFieldKind::Bool, 0);
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
        L"AlternateSkullYawDegrees", L"AlternateSkullPitchDegrees", L"AlternateSkullRollDegrees",
        L"RightEyeX", L"RightEyeY", L"RightEyeZ", L"LeftEyeX", L"LeftEyeY", L"LeftEyeZ",
        L"AlternateRightEyeX", L"AlternateRightEyeY", L"AlternateRightEyeZ", L"AlternateLeftEyeX", L"AlternateLeftEyeY", L"AlternateLeftEyeZ"};
    for (const wchar_t* key : monsterKeys) AddConfigFieldCopy(f, L"Monster", key, 3,
        std::wcsstr(key, L"Eye") ? 1 : 0,
        std::wcsstr(key, L"Eye") ? (std::wcsstr(key, L"Alternate") ? L"Ram Eye Calibration" : L"Deer Eye Calibration") : L"Meshes / Orientation");
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
