// Settings schema, INI defaults/loading, asset lookup, WIC image loading, and runtime variation.
// Included from main.cpp before maze/renderer code.

struct ImageRGBA {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels;

    bool Valid() const {
        return width > 0 && height > 0 && pixels.size() == static_cast<size_t>(width * height * 4);
    }
};

class ScopedCom {
public:
    ScopedCom() {
        hr_ = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }
    ~ScopedCom() {
        if (SUCCEEDED(hr_)) CoUninitialize();
    }
    bool Ok() const {
        return SUCCEEDED(hr_) || hr_ == RPC_E_CHANGED_MODE;
    }
private:
    HRESULT hr_ = E_FAIL;
};

std::filesystem::path ModuleDirectory() {
    wchar_t buffer[MAX_PATH]{};
    DWORD len = GetModuleFileNameW(nullptr, buffer, ARRAYSIZE(buffer));
    if (len == 0) return std::filesystem::current_path();
    return std::filesystem::path(buffer).parent_path();
}

std::vector<std::filesystem::path> CandidateAssetRoots() {
    std::vector<std::filesystem::path> roots;
    auto add = [&](std::filesystem::path p) {
        p = p.lexically_normal();
        if (std::find(roots.begin(), roots.end(), p) == roots.end()) roots.push_back(std::move(p));
    };

    std::filesystem::path module = ModuleDirectory();
    add(module / L"assets" / L"PBRs");
    add(module.parent_path() / L"assets" / L"PBRs");
    add(module.parent_path().parent_path() / L"assets" / L"PBRs");
    add(std::filesystem::current_path() / L"assets" / L"PBRs");
    add(std::filesystem::current_path());
    return roots;
}

std::filesystem::path FindAssetFile(const wchar_t* filename) {
    for (const auto& root : CandidateAssetRoots()) {
        std::filesystem::path p = root / filename;
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) return p;
    }
    return {};
}

bool LoadImageWic(const std::filesystem::path& path, int targetW, int targetH, ImageRGBA& out) {
    out = {};
    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapDecoder> decoder;
    hr = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);
    if (FAILED(hr)) return false;

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame);
    if (FAILED(hr)) return false;

    UINT sourceW = 0;
    UINT sourceH = 0;
    hr = frame->GetSize(&sourceW, &sourceH);
    if (FAILED(hr) || sourceW == 0 || sourceH == 0) return false;
    if (targetW <= 0) targetW = static_cast<int>(sourceW);
    if (targetH <= 0) targetH = static_cast<int>(sourceH);
    if (targetW <= 0 || targetH <= 0) return false;

    ComPtr<IWICBitmapSource> source = frame;
    ComPtr<IWICBitmapScaler> scaler;
    if (targetW != static_cast<int>(sourceW) || targetH != static_cast<int>(sourceH)) {
        hr = factory->CreateBitmapScaler(&scaler);
        if (SUCCEEDED(hr)) {
            hr = scaler->Initialize(frame.Get(), targetW, targetH, WICBitmapInterpolationModeFant);
            if (SUCCEEDED(hr)) source = scaler;
        }
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return false;
    hr = converter->Initialize(source.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) return false;

    out.width = targetW;
    out.height = targetH;
    out.pixels.resize(static_cast<size_t>(targetW * targetH * 4));
    hr = converter->CopyPixels(nullptr, targetW * 4, static_cast<UINT>(out.pixels.size()), out.pixels.data());
    return SUCCEEDED(hr);
}

enum class GameInputAction {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    Sprint,
    Crouch,
    Interact,
    Flashlight,
    Pause,
    Count
};

constexpr int kGameInputActionCount = static_cast<int>(GameInputAction::Count);

struct GameInputBindingDef {
    GameInputAction action;
    const wchar_t* label;
    const wchar_t* iniKey;
    int defaultVk;
};

const GameInputBindingDef kGameInputBindings[] = {
    {GameInputAction::MoveForward, L"Move forward", L"KeyMoveForward", 'W'},
    {GameInputAction::MoveBackward, L"Move backward", L"KeyMoveBackward", 'S'},
    {GameInputAction::MoveLeft, L"Move left", L"KeyMoveLeft", 'A'},
    {GameInputAction::MoveRight, L"Move right", L"KeyMoveRight", 'D'},
    {GameInputAction::Sprint, L"Sprint", L"KeySprint", VK_SHIFT},
    {GameInputAction::Crouch, L"Crouch", L"KeyCrouch", VK_CONTROL},
    {GameInputAction::Interact, L"Interact", L"KeyInteract", 'E'},
    {GameInputAction::Flashlight, L"Flashlight", L"KeyFlashlight", 'F'},
    {GameInputAction::Pause, L"Pause / menu", L"KeyPause", VK_ESCAPE}
};

std::array<int, kGameInputActionCount> DefaultGameKeyBindings() {
    std::array<int, kGameInputActionCount> keys{};
    for (const GameInputBindingDef& binding : kGameInputBindings) {
        keys[static_cast<size_t>(binding.action)] = binding.defaultVk;
    }
    return keys;
}

const GameInputBindingDef& GameInputBinding(GameInputAction action) {
    return kGameInputBindings[static_cast<size_t>(action)];
}

struct Settings;
int GameActionKey(const Settings& settings, GameInputAction action);
void SetGameActionKey(Settings& settings, GameInputAction action, int vk);

struct Settings {
    bool allowWarpFallback = false;
    bool gameFullscreen = true;
    int gameResolutionWidth = 1920;
    int gameResolutionHeight = 1080;

    int mazeWidth = kMazeW;
    int mazeHeight = kMazeH;
    int roomCount = 3;
    int roomMinRadius = 1;
    int roomMaxRadius = 3;
    int roomCountRange = 1;
    int roomMinRadiusRange = 1;
    int roomMaxRadiusRange = 1;
    float extraConnectorMinRatio = 0.015f;
    float extraConnectorMaxRatio = 0.050f;
    int wallFeatureFrequency = 20;
    float wallFeatureFrequencySpread = 1.0f;
    int saveItemMinPerLayer = 1;
    int saveItemMaxPerLayer = 3;
    float saveItemLevelChance = 0.15f;
    uint32_t mazeSeed = 0;
    bool mapOverlay = true;
    bool debugAiMapOverlay = false;
    bool debugInfiniteStamina = false;
    bool debugInvincible = false;
    float runVariation = 0.1f;
    float tileWidthMeters = kTile;
    float tileLengthMeters = kTile;
    float wallHeightMeters = kWallH;

    float floorTextureMeters = kFloorTextureMeters;
    float wallTextureMeters = kWallTextureMeters;
    float ceilingTextureMeters = kCeilingTextureMeters;
    std::wstring assetFolder = L"assets\\PBRs";
    std::wstring wallStem = L"backrooms_wall";
    std::wstring floorStem = L"downloads\\Fabric029_4K-JPG\\Fabric029_4K-JPG";
    std::wstring ceilingStem = L"backup_backrooms_pack_20260511_182856\\backrooms_ceiling";
    std::wstring fleshStem = L"downloads\\Others001_4k\\others_0001";
    bool useExternalNormals = true;
    int maxNormalMapMB = 512;

    float flashlightIntensity = 1.0f;
    float flashlightAttenuation = 0.115f;
    float flashlightConeDegrees = 80.0f;
    bool flashlightShadows = true;
    float flashlightShadowStrength = 0.72f;
    float flashlightShadowDistanceMeters = 18.0f;
    float flashlightShadowBias = 0.00075f;
    int flashlightShadowMapSize = 2048;
    float ambientLight = 0.0f;
    float lampIntensity = 1.45f;
    float lampSpacing = 3.2f;
    float lampOnRatio = 0.95f;
    float lampFlickerRatio = 0.10f;
    float brokenZoneRatio = 0.05f;
    float darkLampVisibleRatio = 1.0f;
    float fogStartMeters = 0.0f;
    float fogEndMeters = 28.0f;
    float fogDarkness = 1.0f;
    float cornerAOIntensity = 0.45f;
    float cornerAORadius = 0.5f;
    float floorCeilingAOIntensity = 0.3f;
    float exposure = 1.0f;
    float gamma = 1.0f;
    float motionBlurAmount = 0.18f;
    float bloomAmount = 0.22f;
    float lensDirtAmount = 0.18f;

    float walkSpeed = 1.692f;
    float roomSpeed = 1.45f;
    float runSpeed = 3.05f;
    float turnLookAheadTiles = 2.1f;
    float roomLookAheadTiles = 2.4f;
    float roomPauseChance = 0.62f;
    float junctionScanChance = 0.88f;
    float scanAngleDegrees = 55.0f;
    float lookBackMinSeconds = 5.0f;
    float lookBackMaxSeconds = 90.0f;
    float headBobAmount = 0.0825f;
    float sideSwayAmount = 0.025f;
    float junctionScanBaseSeconds = 0.85f;
    float junctionScanBranchSeconds = 1.08f;

    float flashlightSwayAmount = 3.0f;
    float flashlightFollowSpeed = 2.0f;
    float flashlightPanicDartAmount = 3.0f;
    float exitDoorOpenSeconds = 1.75f;
    float exitStepSeconds = 1.85f;
    float exitFadeSeconds = 1.10f;
    float exitStepDistance = 2.35f;
    float fadeInSeconds = 1.25f;
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    std::array<int, kGameInputActionCount> gameKeyBindings = DefaultGameKeyBindings();

    bool audioMuted = false;
    float audioMasterVolume = 1.0f;
    float audioEffectsVolume = 1.0f;
    float audioAmbienceVolume = 1.0f;
    float audioMonsterVolume = 1.0f;

    float paperDensity = 1.0f;
    float hallwayPaperRunDensity = 1.0f;
    float chairDensity = 1.0f;
    bool waterDamageEnabled = false;
    float waterDamageDensity = 0.0f;
    float metalCabinetDensity = 0.85f;
    float jumpscareFrequency = 0.15f;
    bool sparkParticles = true;
    float sparkEmitterRatio = 0.15f;
    float sparkBurstMinSeconds = 2.8f;
    float sparkBurstMaxSeconds = 8.8f;
    int sparkMaxParticles = 160;
    float sparkSize = 1.0f;
    bool airParticles = true;
    float airParticleDensity = 1.0f;
    float airParticleSize = 1.0f;
    float airParticleBlur = 1.0f;
    float bloodSplatterDensity = 0.0f;
    int bloodBurstCount = 25;
    float bloodWetness = 0.995f;
    int bloodStreamCount = static_cast<int>(kEffectBloodStreamCount);
    float bloodStreamThickness = kEffectBloodStreamThickness;
    float bloodShaderQuality = kEffectBloodShaderQuality;
    bool bloodWorldFlicker = true;
    bool bloodWorldAlwaysOn = false;
    float bloodWorldCoverage = 1.0f;
    float bloodWorldFlickerMinSeconds = 1500.0f;
    float bloodWorldFlickerMaxSeconds = 4800.0f;
    float bloodWorldFlickerDuration = 0.35f;
    float bloodWorldFlickerIntensity = 1.0f;
    bool bloodStudyView = false;
    bool fleshFlicker = true;
    bool fleshAlwaysOn = false;
    float fleshWetness = 0.995f;
    float fleshParallaxScale = 0.22f;
    float fleshFlickerMinSeconds = 1500.0f;
    float fleshFlickerMaxSeconds = 4800.0f;
    float fleshFlickerDuration = 0.35f;
    float fleshFlickerIntensity = 1.0f;

    float effectBloodLoopSeconds = kEffectBloodLoopSeconds;
    float effectBloodFullSpreadAge = kEffectBloodFullSpreadAge;
    float effectWaterLoopSeconds = kEffectWaterLoopSeconds;
    float effectAirVentLoopSeconds = kEffectAirVentLoopSeconds;
    float effectBrokenLampLoopSeconds = kEffectBrokenLampLoopSeconds;
    float effectStaticLoopSeconds = kEffectStaticLoopSeconds;
    float effectBrokenLampSparkIntensityMin = kEffectBrokenLampSparkIntensity.min;
    float effectBrokenLampSparkIntensityMax = kEffectBrokenLampSparkIntensity.max;
    float effectBrokenLampChainIntensityScale = kEffectBrokenLampChainIntensityScale;
    int effectBrokenLampChainBurstsMin = kEffectBrokenLampChainBursts.min;
    int effectBrokenLampChainBurstsMax = kEffectBrokenLampChainBursts.max;
    float effectAirVentSteamIntensityMin = kEffectAirVentSteamIntensity.min;
    float effectAirVentSteamIntensityMax = kEffectAirVentSteamIntensity.max;
    int effectAirVentPanelDropEvery = kEffectAirVentPanelDropEvery;
    float effectAirVentPanelDropChance = 0.10f;

    bool dreadEnabled = true;
    bool dreadDebugMeter = false;
    float dreadDecayPerSecond = 0.030f;
    float dreadMonsterDistance = 13.0f;
    float dreadMonsterGainPerSecond = 0.42f;
    float dreadJumpscareGain = 0.1f;
    float dreadFleshGain = 0.2f;
    float dreadWalkSpeedBoost = 0.05f;
    float dreadRunSpeedBoost = 0.075f;
    float dreadFlashlightFlicker = 1.0f;

    float monsterScale = 1.0f;
    float monsterSpeed = 0.68f;
    float monsterSprintSpeed = 0.88f;
    bool monsterIgnorePlayer = false;
    float monsterKillDistance = 1.18f;
    float monsterVisibleDistance = 12.0f;
    std::wstring monsterSkullMesh = L"assets\\models\\monster_face_mask\\horror_mask.obj";
    std::wstring monsterAltSkullMesh = L"";
    float monsterAltSkullChance = 0.0f;
    int monsterSkullMaxTriangles = 16000;
    float monsterSkullYawDegrees = 0.0f;
    float monsterSkullPitchDegrees = 0.0f;
    float monsterSkullRollDegrees = 0.0f;
    float monsterAltSkullYawDegrees = -90.0f;
    float monsterAltSkullPitchDegrees = 0.0f;
    float monsterAltSkullRollDegrees = 90.0f;
    float monsterRightEyeX = -0.086f;
    float monsterRightEyeY = -0.304f;
    float monsterRightEyeZ = 0.095f;
    float monsterLeftEyeX = 0.106f;
    float monsterLeftEyeY = -0.304f;
    float monsterLeftEyeZ = 0.095f;
    float monsterAltRightEyeX = -0.086f;
    float monsterAltRightEyeY = -0.145f;
    float monsterAltRightEyeZ = 0.095f;
    float monsterAltLeftEyeX = 0.106f;
    float monsterAltLeftEyeY = -0.145f;
    float monsterAltLeftEyeZ = 0.095f;
};

int GameActionKey(const Settings& settings, GameInputAction action) {
    size_t index = static_cast<size_t>(action);
    if (index >= settings.gameKeyBindings.size()) return 0;
    return settings.gameKeyBindings[index];
}

void SetGameActionKey(Settings& settings, GameInputAction action, int vk) {
    size_t index = static_cast<size_t>(action);
    if (index >= settings.gameKeyBindings.size()) return;
    settings.gameKeyBindings[index] = std::clamp(vk, 1, 255);
}

void AssignGameActionKey(Settings& settings, GameInputAction action, int vk) {
    vk = std::clamp(vk, 1, 255);
    int actionIndex = static_cast<int>(action);
    int previous = GameActionKey(settings, action);
    for (int i = 0; i < kGameInputActionCount; ++i) {
        if (i != actionIndex && settings.gameKeyBindings[static_cast<size_t>(i)] == vk) {
            settings.gameKeyBindings[static_cast<size_t>(i)] = previous;
            break;
        }
    }
    SetGameActionKey(settings, action, vk);
}

std::wstring KeyDisplayName(int vk) {
    vk = std::clamp(vk, 1, 255);
    if (vk >= 'A' && vk <= 'Z') return std::wstring(1, static_cast<wchar_t>(vk));
    if (vk >= '0' && vk <= '9') return std::wstring(1, static_cast<wchar_t>(vk));
    switch (vk) {
    case VK_ESCAPE: return L"Esc";
    case VK_SPACE: return L"Space";
    case VK_SHIFT: return L"Shift";
    case VK_CONTROL: return L"Ctrl";
    case VK_MENU: return L"Alt";
    case VK_TAB: return L"Tab";
    case VK_RETURN: return L"Enter";
    case VK_BACK: return L"Backspace";
    case VK_LEFT: return L"Left";
    case VK_RIGHT: return L"Right";
    case VK_UP: return L"Up";
    case VK_DOWN: return L"Down";
    case VK_PRIOR: return L"Page Up";
    case VK_NEXT: return L"Page Down";
    case VK_HOME: return L"Home";
    case VK_END: return L"End";
    case VK_INSERT: return L"Insert";
    case VK_DELETE: return L"Delete";
    case VK_LSHIFT: return L"Left Shift";
    case VK_RSHIFT: return L"Right Shift";
    case VK_LCONTROL: return L"Left Ctrl";
    case VK_RCONTROL: return L"Right Ctrl";
    case VK_LMENU: return L"Left Alt";
    case VK_RMENU: return L"Right Alt";
    default: break;
    }

    UINT scan = MapVirtualKeyW(static_cast<UINT>(vk), MAPVK_VK_TO_VSC);
    wchar_t name[96]{};
    LONG lparam = static_cast<LONG>(scan << 16);
    if (GetKeyNameTextW(lparam, name, ARRAYSIZE(name)) > 0) return name;
    wchar_t fallback[32]{};
    swprintf_s(fallback, L"VK %d", vk);
    return fallback;
}

std::filesystem::path SettingsPath() {
    wchar_t localAppData[MAX_PATH]{};
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, ARRAYSIZE(localAppData));
    std::filesystem::path base = (len > 0 && len < ARRAYSIZE(localAppData))
        ? std::filesystem::path(localAppData)
        : ModuleDirectory();
    std::filesystem::path dir = base / L"BackroomsMazeScreensaver";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir / L"BackroomsMaze.ini";
}

std::filesystem::path PackagedSettingsPath() {
    return ModuleDirectory() / L"BackroomsMaze.ini";
}

std::filesystem::path GameSavePath() {
    return SettingsPath().parent_path() / L"BackroomsMaze.save";
}

std::filesystem::path CacheDirectory() {
    wchar_t localAppData[MAX_PATH]{};
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, ARRAYSIZE(localAppData));
    std::filesystem::path base = (len > 0 && len < ARRAYSIZE(localAppData))
        ? std::filesystem::path(localAppData)
        : ModuleDirectory();
    std::filesystem::path dir = base / L"BackroomsMazeScreensaver" / L"Cache";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return dir;
}

bool StartupProfileEnabled() {
    static const bool enabled = []() {
        wchar_t value[8]{};
        return GetEnvironmentVariableW(L"BACKROOMS_PROFILE_STARTUP", value, ARRAYSIZE(value)) > 0;
    }();
    return enabled;
}

void StartupProfileLine(const std::wstring& line) {
    if (!StartupProfileEnabled()) return;
    std::wofstream out(ModuleDirectory() / L"BackroomsMaze.profile.log", std::ios::app);
    if (out) out << line << L"\r\n";
}

class StartupProfile {
public:
    explicit StartupProfile(const wchar_t* name) : name_(name), enabled_(StartupProfileEnabled()) {
        if (!enabled_) return;
        start_ = NowMs();
        last_ = start_;
        StartupProfileLine(L"[" + name_ + L"]");
    }

    void Mark(const wchar_t* label) {
        if (!enabled_) return;
        double now = NowMs();
        std::wostringstream line;
        line << std::fixed << std::setprecision(3);
        line << name_ << L" " << label
             << L": +" << (now - last_) << L" ms"
             << L", total " << (now - start_) << L" ms";
        StartupProfileLine(line.str());
        last_ = now;
    }

private:
    static double NowMs() {
        LARGE_INTEGER frequency{};
        LARGE_INTEGER counter{};
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
        return static_cast<double>(counter.QuadPart) * 1000.0 / static_cast<double>(std::max<LONGLONG>(1, frequency.QuadPart));
    }

    std::wstring name_;
    bool enabled_ = false;
    double start_ = 0.0;
    double last_ = 0.0;
};

std::wstring DefaultConfigText() {
    std::wostringstream s;
    s << L"; Backrooms Maze Screensaver settings\r\n"
      << L"; Paths can be absolute, or relative to this SCR folder/current project folder.\r\n\r\n"
      << L"[Renderer]\r\n"
      << L"AllowWarpFallback=0\r\n\r\n"
      << L"[GameWindow]\r\n"
      << L"Fullscreen=1\r\n"
      << L"ResolutionWidth=1920\r\n"
      << L"ResolutionHeight=1080\r\n\r\n"
      << L"[Maze]\r\n"
      << L"Width=25\r\n"
      << L"Height=25\r\n"
      << L"RoomCount=3\r\n"
      << L"RoomMinRadius=1.5\r\n"
      << L"RoomMaxRadius=3\r\n"
      << L"RoomCountRange=1\r\n"
      << L"RoomMinRadiusRange=1\r\n"
      << L"RoomMaxRadiusRange=1\r\n"
      << L"; Opens this random fraction of eligible interior wall tiles after maze generation.\r\n"
      << L"ExtraConnectorMinRatio=0.015\r\n"
      << L"ExtraConnectorMaxRatio=0.05\r\n"
      << L"; Base eligible wall interval for windows/crawl tunnels. Spread is multiplicative; 1 means roughly half to double per maze.\r\n"
      << L"WallFeatureFrequency=20\r\n"
      << L"WallFeatureFrequencySpread=1\r\n"
      << L"; Save item/typewriter placement for playable layers.\r\n"
      << L"SaveItemMinPerLayer=1\r\n"
      << L"SaveItemMaxPerLayer=3\r\n"
      << L"SaveItemLevelChance=0.15\r\n"
      << L"RandomSeed=0\r\n\r\n"
      << L"MapOverlay=1\r\n\r\n"
      << L"TileWidthMeters=1.6\r\n"
      << L"TileLengthMeters=1.6\r\n"
      << L"WallHeightMeters=2.4\r\n\r\n"
      << L"[Randomization]\r\n"
      << L"; 0 keeps configured values fixed. 1 enables bounded per-run jitter.\r\n"
      << L"; Camera AI, Flashlight Motion, and non-blood/flesh Atmosphere values use +/-15%.\r\n"
      << L"; Maze room +/- fields are explicit integer ranges, also scaled by this value.\r\n"
      << L"RunVariation=0.1\r\n\r\n"
      << L"[Textures]\r\n"
      << L"AssetFolder=assets\\PBRs\r\n"
      << L"WallStem=backrooms_wall\r\n"
      << L"; Leave a stem empty to use the built-in procedural material.\r\n"
      << L"FloorStem=downloads\\Fabric029_4K-JPG\\Fabric029_4K-JPG\r\n"
      << L"CeilingStem=backup_backrooms_pack_20260511_182856\\backrooms_ceiling\r\n"
      << L"FleshStem=downloads\\Others001_4k\\others_0001\r\n"
      << L"WallScaleMeters=1.8\r\n"
      << L"FloorScaleMeters=1.8\r\n"
      << L"; 0 auto-aligns the 6x6 ceiling sheet as a 3x3 panel grid per maze tile.\r\n"
      << L"CeilingScaleMeters=0\r\n"
      << L"UseExternalNormals=1\r\n"
      << L"MaxNormalMapMB=512\r\n\r\n"
      << L"[Lighting]\r\n"
      << L"FlashlightIntensity=1\r\n"
      << L"FlashlightAttenuation=0.115\r\n"
      << L"FlashlightConeDegrees=80\r\n"
      << L"FlashlightShadows=1\r\n"
      << L"FlashlightShadowStrength=0.72\r\n"
      << L"FlashlightShadowDistanceMeters=18\r\n"
      << L"FlashlightShadowBias=0.00075\r\n"
      << L"FlashlightShadowMapSize=2048\r\n"
      << L"AmbientLight=0\r\n"
      << L"LampIntensity=1.45\r\n"
      << L"LampSpacing=3.2\r\n"
      << L"LampOnRatio=0.95\r\n"
      << L"LampFlickerRatio=0.1\r\n"
      << L"BrokenZoneRatio=0.05\r\n"
      << L"DarkLampVisibleRatio=1.0\r\n"
      << L"FogStartMeters=0\r\n"
      << L"FogEndMeters=28\r\n"
      << L"FogDarkness=1\r\n"
      << L"CornerAOIntensity=0.45\r\n"
      << L"CornerAORadius=0.5\r\n"
      << L"FloorCeilingAOIntensity=0.3\r\n"
      << L"Exposure=1\r\n"
      << L"Gamma=1\r\n"
      << L"MotionBlurAmount=0.18\r\n"
      << L"BloomAmount=0.22\r\n"
      << L"LensDirtAmount=0.18\r\n\r\n"
      << L"[CameraAI]\r\n"
      << L"WalkSpeed=1.692\r\n"
      << L"RoomSpeed=1.45\r\n"
      << L"RunSpeed=3.05\r\n"
      << L"TurnLookAheadTiles=2.1\r\n"
      << L"RoomLookAheadTiles=2.4\r\n"
      << L"RoomPauseChance=0.62\r\n"
      << L"JunctionScanChance=0.88\r\n"
      << L"ScanAngleDegrees=55\r\n"
      << L"LookBackMinSeconds=5\r\n"
      << L"LookBackMaxSeconds=90\r\n"
      << L"HeadBobAmount=0.0825\r\n"
      << L"SideSwayAmount=0.025\r\n"
      << L"JunctionScanBaseSeconds=0.85\r\n"
      << L"JunctionScanBranchSeconds=1.08\r\n\r\n"
      << L"[CameraFX]\r\n"
      << L"FlashlightSwayAmount=3\r\n"
      << L"FlashlightFollowSpeed=2\r\n"
      << L"FlashlightPanicDartAmount=3\r\n"
      << L"ExitDoorOpenSeconds=1.75\r\n"
      << L"ExitStepSeconds=1.85\r\n"
      << L"ExitFadeSeconds=1.1\r\n"
      << L"ExitStepDistance=2.35\r\n"
      << L"FadeInSeconds=1.25\r\n\r\n"
      << L"[Controls]\r\n"
      << L"MouseSensitivity=1\r\n"
      << L"InvertMouseY=0\r\n";
    for (const GameInputBindingDef& binding : kGameInputBindings) {
        s << binding.iniKey << L"=" << binding.defaultVk << L"\r\n";
    }
    s << L"\r\n"
      << L"[Audio]\r\n"
      << L"; Reserved for the game audio engine. Values are saved now so the settings UI is ready.\r\n"
      << L"Muted=0\r\n"
      << L"MasterVolume=1\r\n"
      << L"EffectsVolume=1\r\n"
      << L"AmbienceVolume=1\r\n"
      << L"MonsterVolume=1\r\n\r\n"
      << L"[Atmosphere]\r\n"
      << L"PaperDensity=1\r\n"
      << L"HallwayPaperRunDensity=1\r\n"
      << L"ChairDensity=1\r\n"
      << L"WaterDamageEnabled=0\r\n"
      << L"WaterDamageDensity=0\r\n"
      << L"MetalCabinetDensity=0.85\r\n"
      << L"JumpscareFrequency=0.15\r\n"
      << L"SparkParticles=1\r\n"
      << L"SparkEmitterRatio=0.15\r\n"
      << L"SparkBurstMinSeconds=2.8\r\n"
      << L"SparkBurstMaxSeconds=8.8\r\n"
      << L"SparkMaxParticles=160\r\n"
      << L"SparkSize=1\r\n"
      << L"AirParticles=1\r\n"
      << L"AirParticleDensity=1\r\n"
      << L"AirParticleSize=1\r\n"
      << L"AirParticleBlur=1\r\n"
        << L"BloodSplatterDensity=0\r\n"
      << L"BloodBurstCount=25\r\n"
      << L"BloodWetness=0.995\r\n"
      << L"BloodStreamCount=30\r\n"
      << L"BloodStreamThickness=0.88\r\n"
      << L"BloodShaderQuality=1\r\n"
      << L"BloodWorldFlicker=1\r\n"
      << L"BloodWorldAlwaysOn=0\r\n"
      << L"BloodWorldCoverage=1\r\n"
      << L"BloodWorldFlickerMinSeconds=1500\r\n"
      << L"BloodWorldFlickerMaxSeconds=4800\r\n"
      << L"BloodWorldFlickerDuration=0.35\r\n"
      << L"BloodWorldFlickerIntensity=1\r\n"
      << L"BloodStudyView=0\r\n"
      << L"FleshFlicker=1\r\n"
      << L"FleshAlwaysOn=0\r\n"
      << L"FleshWetness=0.995\r\n"
      << L"FleshParallaxScale=0.22\r\n"
      << L"FleshFlickerMinSeconds=1500\r\n"
      << L"FleshFlickerMaxSeconds=4800\r\n"
      << L"FleshFlickerDuration=0.35\r\n"
      << L"FleshFlickerIntensity=1\r\n\r\n"
      << L"[EffectTuning]\r\n"
      << L"; Shared source of truth for debug viewer and screensaver effect playback.\r\n"
      << L"BloodLoopSeconds=56\r\n"
      << L"BloodFullSpreadAge=48\r\n"
      << L"WaterLoopSeconds=7.5\r\n"
      << L"AirVentLoopSeconds=6.2\r\n"
      << L"BrokenLampLoopSeconds=5.2\r\n"
      << L"StaticLoopSeconds=8\r\n"
      << L"BrokenLampSparkIntensityMin=2.2\r\n"
      << L"BrokenLampSparkIntensityMax=4.1\r\n"
      << L"BrokenLampChainIntensityScale=0.70\r\n"
      << L"BrokenLampChainBurstsMin=1\r\n"
      << L"BrokenLampChainBurstsMax=3\r\n"
      << L"AirVentSteamIntensityMin=1.9\r\n"
      << L"AirVentSteamIntensityMax=3.2\r\n"
      << L"AirVentPanelDropEvery=3\r\n"
      << L"AirVentPanelDropChance=0.1\r\n\r\n"
      << L"[Dread]\r\n"
      << L"Enabled=1\r\n"
      << L"DebugMeter=0\r\n"
      << L"DecayPerSecond=0.03\r\n"
      << L"MonsterDistance=13\r\n"
      << L"MonsterGainPerSecond=0.42\r\n"
      << L"JumpscareGain=0.1\r\n"
      << L"FleshGain=0.2\r\n"
      << L"WalkSpeedBoost=0.05\r\n"
      << L"RunSpeedBoost=0.075\r\n"
      << L"FlashlightFlicker=1\r\n\r\n"
      << L"[Debug]\r\n"
      << L"AiMapOverlay=0\r\n"
      << L"InfiniteStamina=0\r\n"
      << L"Invincible=0\r\n\r\n"
      << L"[Monster]\r\n"
      << L"MonsterScale=1\r\n"
      << L"MonsterSpeed=0.68\r\n"
      << L"MonsterSprintSpeed=0.88\r\n"
      << L"MonsterIgnorePlayer=0\r\n"
      << L"MonsterKillDistance=1.18\r\n"
      << L"MonsterVisibleDistance=12\r\n"
      << L"SkullMesh=assets\\models\\monster_face_mask\\horror_mask.obj\r\n"
      << L"AlternateSkullMesh=\r\n"
      << L"AlternateSkullChance=0\r\n"
      << L"SkullMaxTriangles=16000\r\n"
      << L"SkullYawDegrees=0\r\n"
      << L"SkullPitchDegrees=0\r\n"
      << L"SkullRollDegrees=0\r\n"
      << L"AlternateSkullYawDegrees=-90\r\n"
      << L"AlternateSkullPitchDegrees=0\r\n"
      << L"AlternateSkullRollDegrees=90\r\n"
      << L"RightEyeX=-0.086\r\n"
      << L"RightEyeY=-0.304\r\n"
      << L"RightEyeZ=0.095\r\n"
      << L"LeftEyeX=0.106\r\n"
      << L"LeftEyeY=-0.304\r\n"
      << L"LeftEyeZ=0.095\r\n"
      << L"AlternateRightEyeX=-0.086\r\n"
      << L"AlternateRightEyeY=-0.145\r\n"
      << L"AlternateRightEyeZ=0.095\r\n"
      << L"AlternateLeftEyeX=0.106\r\n"
      << L"AlternateLeftEyeY=-0.145\r\n"
      << L"AlternateLeftEyeZ=0.095\r\n";
    return s.str();
}

bool WriteTextFile(const std::filesystem::path& path, const std::wstring& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    std::string utf8;
    int needed = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (needed > 0) {
        utf8.resize(static_cast<size_t>(needed));
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), utf8.data(), needed, nullptr, nullptr);
    }
    out.write(utf8.data(), static_cast<std::streamsize>(utf8.size()));
    return out.good();
}

std::wstring ReadTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    if (bytes.empty()) return {};
    int needed = MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    if (needed <= 0) return {};
    std::wstring text(static_cast<size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, bytes.data(), static_cast<int>(bytes.size()), text.data(), needed);
    return text;
}

void EnsureSettingsFile() {
    std::error_code ec;
    auto path = SettingsPath();
    if (!std::filesystem::exists(path, ec)) {
        auto packaged = PackagedSettingsPath();
        if (std::filesystem::exists(packaged, ec)) {
            std::filesystem::copy_file(packaged, path, std::filesystem::copy_options::overwrite_existing, ec);
        }
        if (!std::filesystem::exists(path, ec)) {
            WriteTextFile(path, DefaultConfigText());
        }
    }
}

std::wstring IniString(const wchar_t* section, const wchar_t* key, const wchar_t* fallback) {
    EnsureSettingsFile();
    wchar_t buffer[1024]{};
    GetPrivateProfileStringW(section, key, fallback, buffer, ARRAYSIZE(buffer), SettingsPath().c_str());
    return buffer;
}

float IniFloat(const wchar_t* section, const wchar_t* key, float fallback) {
    wchar_t fb[64]{};
    swprintf_s(fb, L"%.6g", fallback);
    std::wstring value = IniString(section, key, fb);
    wchar_t* end = nullptr;
    float parsed = std::wcstof(value.c_str(), &end);
    return end != value.c_str() ? parsed : fallback;
}

float NormalizeFlashlightShadowBias(float value) {
    if (!std::isfinite(value)) {
        return 0.00075f;
    }
    if (value > 0.02f) {
        value *= 0.001f;
    }
    return std::clamp(value, 0.00005f, 0.006f);
}

int IniInt(const wchar_t* section, const wchar_t* key, int fallback) {
    return GetPrivateProfileIntW(section, key, fallback, SettingsPath().c_str());
}

Settings LoadSettings() {
    EnsureSettingsFile();
    Settings s;
    s.allowWarpFallback = IniInt(L"Renderer", L"AllowWarpFallback", s.allowWarpFallback ? 1 : 0) != 0;
    s.gameFullscreen = IniInt(L"GameWindow", L"Fullscreen", s.gameFullscreen ? 1 : 0) != 0;
    s.gameResolutionWidth = std::clamp(IniInt(L"GameWindow", L"ResolutionWidth", s.gameResolutionWidth), 640, 7680);
    s.gameResolutionHeight = std::clamp(IniInt(L"GameWindow", L"ResolutionHeight", s.gameResolutionHeight), 360, 4320);
    s.mazeWidth = std::clamp(IniInt(L"Maze", L"Width", s.mazeWidth) | 1, 15, 151);
    s.mazeHeight = std::clamp(IniInt(L"Maze", L"Height", s.mazeHeight) | 1, 15, 151);
    s.roomCount = std::clamp(IniInt(L"Maze", L"RoomCount", s.roomCount), 0, 80);
    s.roomMinRadius = std::clamp(IniInt(L"Maze", L"RoomMinRadius", s.roomMinRadius), 1, 12);
    s.roomMaxRadius = std::clamp(IniInt(L"Maze", L"RoomMaxRadius", s.roomMaxRadius), s.roomMinRadius, 16);
    s.roomCountRange = std::clamp(IniInt(L"Maze", L"RoomCountRange", s.roomCountRange), 0, 80);
    s.roomMinRadiusRange = std::clamp(IniInt(L"Maze", L"RoomMinRadiusRange", s.roomMinRadiusRange), 0, 12);
    s.roomMaxRadiusRange = std::clamp(IniInt(L"Maze", L"RoomMaxRadiusRange", s.roomMaxRadiusRange), 0, 16);
    s.extraConnectorMinRatio = std::clamp(IniFloat(L"Maze", L"ExtraConnectorMinRatio", s.extraConnectorMinRatio), 0.015f, 0.20f);
    s.extraConnectorMaxRatio = std::clamp(IniFloat(L"Maze", L"ExtraConnectorMaxRatio", s.extraConnectorMaxRatio), s.extraConnectorMinRatio, 0.20f);
    s.wallFeatureFrequency = std::clamp(IniInt(L"Maze", L"WallFeatureFrequency", s.wallFeatureFrequency), 1, 200);
    s.wallFeatureFrequencySpread = std::clamp(IniFloat(L"Maze", L"WallFeatureFrequencySpread", s.wallFeatureFrequencySpread), 0.0f, 3.0f);
    s.saveItemMinPerLayer = std::clamp(IniInt(L"Maze", L"SaveItemMinPerLayer", s.saveItemMinPerLayer), 0, 5);
    s.saveItemMaxPerLayer = std::clamp(IniInt(L"Maze", L"SaveItemMaxPerLayer", s.saveItemMaxPerLayer), s.saveItemMinPerLayer, 5);
    s.saveItemLevelChance = std::clamp(IniFloat(L"Maze", L"SaveItemLevelChance", s.saveItemLevelChance), 0.0f, 1.0f);
    s.mazeSeed = static_cast<uint32_t>(std::clamp(IniInt(L"Maze", L"RandomSeed", static_cast<int>(s.mazeSeed)), 0, std::numeric_limits<int>::max()));
    s.mapOverlay = IniInt(L"Maze", L"MapOverlay", s.mapOverlay ? 1 : 0) != 0;
    s.debugAiMapOverlay = IniInt(L"Debug", L"AiMapOverlay", s.debugAiMapOverlay ? 1 : 0) != 0;
    s.debugInfiniteStamina = IniInt(L"Debug", L"InfiniteStamina", s.debugInfiniteStamina ? 1 : 0) != 0;
    s.debugInvincible = IniInt(L"Debug", L"Invincible", s.debugInvincible ? 1 : 0) != 0;
    s.tileWidthMeters = std::clamp(IniFloat(L"Maze", L"TileWidthMeters", s.tileWidthMeters), 1.2f, 8.0f);
    s.tileLengthMeters = std::clamp(IniFloat(L"Maze", L"TileLengthMeters", s.tileLengthMeters), 1.2f, 8.0f);
    s.wallHeightMeters = std::clamp(IniFloat(L"Maze", L"WallHeightMeters", s.wallHeightMeters), 1.8f, 8.0f);
    s.runVariation = std::clamp(IniFloat(L"Randomization", L"RunVariation", s.runVariation), 0.0f, 1.0f);

    s.assetFolder = IniString(L"Textures", L"AssetFolder", s.assetFolder.c_str());
    s.wallStem = IniString(L"Textures", L"WallStem", s.wallStem.c_str());
    s.floorStem = IniString(L"Textures", L"FloorStem", s.floorStem.c_str());
    s.ceilingStem = IniString(L"Textures", L"CeilingStem", s.ceilingStem.c_str());
    s.fleshStem = IniString(L"Textures", L"FleshStem", s.fleshStem.c_str());
    s.wallTextureMeters = std::max(0.2f, IniFloat(L"Textures", L"WallScaleMeters", s.wallTextureMeters));
    s.floorTextureMeters = std::max(0.2f, IniFloat(L"Textures", L"FloorScaleMeters", s.floorTextureMeters));
    s.ceilingTextureMeters = std::max(0.0f, IniFloat(L"Textures", L"CeilingScaleMeters", s.ceilingTextureMeters));
    s.useExternalNormals = IniInt(L"Textures", L"UseExternalNormals", s.useExternalNormals ? 1 : 0) != 0;
    s.maxNormalMapMB = std::clamp(IniInt(L"Textures", L"MaxNormalMapMB", s.maxNormalMapMB), 0, 1024);

    s.flashlightIntensity = std::clamp(IniFloat(L"Lighting", L"FlashlightIntensity", s.flashlightIntensity), 0.0f, 10.0f);
    s.flashlightAttenuation = std::clamp(IniFloat(L"Lighting", L"FlashlightAttenuation", s.flashlightAttenuation), 0.001f, 2.0f);
    s.flashlightConeDegrees = std::clamp(IniFloat(L"Lighting", L"FlashlightConeDegrees", s.flashlightConeDegrees), 20.0f, 140.0f);
    s.flashlightShadows = IniInt(L"Lighting", L"FlashlightShadows", s.flashlightShadows ? 1 : 0) != 0;
    s.flashlightShadowStrength = std::clamp(IniFloat(L"Lighting", L"FlashlightShadowStrength", s.flashlightShadowStrength), 0.0f, 1.0f);
    s.flashlightShadowDistanceMeters = std::clamp(IniFloat(L"Lighting", L"FlashlightShadowDistanceMeters", s.flashlightShadowDistanceMeters), 2.0f, 45.0f);
    s.flashlightShadowBias = NormalizeFlashlightShadowBias(IniFloat(L"Lighting", L"FlashlightShadowBias", s.flashlightShadowBias));
    s.flashlightShadowMapSize = std::clamp(IniInt(L"Lighting", L"FlashlightShadowMapSize", s.flashlightShadowMapSize), 512, 4096);
    s.ambientLight = std::clamp(IniFloat(L"Lighting", L"AmbientLight", s.ambientLight), 0.0f, 1.0f);
    s.lampIntensity = std::clamp(IniFloat(L"Lighting", L"LampIntensity", s.lampIntensity), 0.0f, 10.0f);
    s.lampSpacing = std::clamp(IniFloat(L"Lighting", L"LampSpacing", s.lampSpacing), 2.0f, 40.0f);
    s.lampOnRatio = std::clamp(IniFloat(L"Lighting", L"LampOnRatio", s.lampOnRatio), 0.0f, 1.0f);
    s.lampFlickerRatio = std::clamp(IniFloat(L"Lighting", L"LampFlickerRatio", s.lampFlickerRatio), 0.0f, 1.0f);
    s.brokenZoneRatio = std::clamp(IniFloat(L"Lighting", L"BrokenZoneRatio", s.brokenZoneRatio), 0.0f, 1.0f);
    s.darkLampVisibleRatio = std::clamp(IniFloat(L"Lighting", L"DarkLampVisibleRatio", s.darkLampVisibleRatio), 0.0f, 1.0f);
    s.fogStartMeters = std::clamp(IniFloat(L"Lighting", L"FogStartMeters", s.fogStartMeters), 0.0f, 200.0f);
    s.fogEndMeters = std::max(s.fogStartMeters + 0.1f, std::clamp(IniFloat(L"Lighting", L"FogEndMeters", s.fogEndMeters), 0.1f, 300.0f));
    s.fogDarkness = std::clamp(IniFloat(L"Lighting", L"FogDarkness", s.fogDarkness), 0.0f, 1.0f);
    s.cornerAOIntensity = std::clamp(IniFloat(L"Lighting", L"CornerAOIntensity", s.cornerAOIntensity), 0.0f, 1.0f);
    s.cornerAORadius = std::clamp(IniFloat(L"Lighting", L"CornerAORadius", s.cornerAORadius), 0.05f, 2.0f);
    s.floorCeilingAOIntensity = std::clamp(IniFloat(L"Lighting", L"FloorCeilingAOIntensity", s.floorCeilingAOIntensity), 0.0f, 1.0f);
    s.exposure = std::clamp(IniFloat(L"Lighting", L"Exposure", s.exposure), 0.1f, 8.0f);
    s.gamma = std::clamp(IniFloat(L"Lighting", L"Gamma", s.gamma), 0.5f, 3.5f);
    s.motionBlurAmount = std::clamp(IniFloat(L"Lighting", L"MotionBlurAmount", s.motionBlurAmount), 0.0f, 2.0f);
    s.bloomAmount = std::clamp(IniFloat(L"Lighting", L"BloomAmount", s.bloomAmount), 0.0f, 2.0f);
    s.lensDirtAmount = std::clamp(IniFloat(L"Lighting", L"LensDirtAmount", s.lensDirtAmount), 0.0f, 2.0f);

    s.walkSpeed = std::clamp(IniFloat(L"CameraAI", L"WalkSpeed", s.walkSpeed), 0.1f, 8.0f);
    s.roomSpeed = std::clamp(IniFloat(L"CameraAI", L"RoomSpeed", s.roomSpeed), 0.1f, 8.0f);
    s.runSpeed = std::clamp(IniFloat(L"CameraAI", L"RunSpeed", s.runSpeed), 0.1f, 12.0f);
    s.turnLookAheadTiles = std::clamp(IniFloat(L"CameraAI", L"TurnLookAheadTiles", s.turnLookAheadTiles), 0.0f, 8.0f);
    s.roomLookAheadTiles = std::clamp(IniFloat(L"CameraAI", L"RoomLookAheadTiles", s.roomLookAheadTiles), 0.0f, 10.0f);
    s.roomPauseChance = std::clamp(IniFloat(L"CameraAI", L"RoomPauseChance", s.roomPauseChance), 0.0f, 1.0f);
    s.junctionScanChance = std::clamp(IniFloat(L"CameraAI", L"JunctionScanChance", s.junctionScanChance), 0.0f, 1.0f);
    s.scanAngleDegrees = std::clamp(IniFloat(L"CameraAI", L"ScanAngleDegrees", s.scanAngleDegrees), 0.0f, 160.0f);
    s.lookBackMinSeconds = std::clamp(IniFloat(L"CameraAI", L"LookBackMinSeconds", s.lookBackMinSeconds), 2.0f, 300.0f);
    s.lookBackMaxSeconds = std::max(s.lookBackMinSeconds, std::clamp(IniFloat(L"CameraAI", L"LookBackMaxSeconds", s.lookBackMaxSeconds), 2.0f, 300.0f));
    s.headBobAmount = std::clamp(IniFloat(L"CameraAI", L"HeadBobAmount", s.headBobAmount), 0.0f, 0.4f);
    s.sideSwayAmount = std::clamp(IniFloat(L"CameraAI", L"SideSwayAmount", s.sideSwayAmount), 0.0f, 0.3f);
    s.junctionScanBaseSeconds = std::clamp(IniFloat(L"CameraAI", L"JunctionScanBaseSeconds", s.junctionScanBaseSeconds), 0.0f, 4.0f);
    s.junctionScanBranchSeconds = std::clamp(IniFloat(L"CameraAI", L"JunctionScanBranchSeconds", s.junctionScanBranchSeconds), 0.0f, 3.0f);

    s.flashlightSwayAmount = std::clamp(IniFloat(L"CameraFX", L"FlashlightSwayAmount", s.flashlightSwayAmount), 0.0f, 4.0f);
    s.flashlightFollowSpeed = std::clamp(IniFloat(L"CameraFX", L"FlashlightFollowSpeed", s.flashlightFollowSpeed), 0.1f, 4.0f);
    s.flashlightPanicDartAmount = std::clamp(IniFloat(L"CameraFX", L"FlashlightPanicDartAmount", s.flashlightPanicDartAmount), 0.0f, 4.0f);
    s.exitDoorOpenSeconds = std::clamp(IniFloat(L"CameraFX", L"ExitDoorOpenSeconds", s.exitDoorOpenSeconds), 0.2f, 8.0f);
    s.exitStepSeconds = std::clamp(IniFloat(L"CameraFX", L"ExitStepSeconds", s.exitStepSeconds), 0.2f, 8.0f);
    s.exitFadeSeconds = std::clamp(IniFloat(L"CameraFX", L"ExitFadeSeconds", s.exitFadeSeconds), 0.2f, 8.0f);
    s.exitStepDistance = std::clamp(IniFloat(L"CameraFX", L"ExitStepDistance", s.exitStepDistance), 0.0f, 8.0f);
    s.fadeInSeconds = std::clamp(IniFloat(L"CameraFX", L"FadeInSeconds", s.fadeInSeconds), 0.0f, 8.0f);
    s.mouseSensitivity = std::clamp(IniFloat(L"Controls", L"MouseSensitivity", s.mouseSensitivity), 0.1f, 5.0f);
    s.invertMouseY = IniInt(L"Controls", L"InvertMouseY", s.invertMouseY ? 1 : 0) != 0;
    for (const GameInputBindingDef& binding : kGameInputBindings) {
        int fallback = GameActionKey(s, binding.action);
        SetGameActionKey(s, binding.action, std::clamp(IniInt(L"Controls", binding.iniKey, fallback), 1, 255));
    }

    s.audioMuted = IniInt(L"Audio", L"Muted", s.audioMuted ? 1 : 0) != 0;
    s.audioMasterVolume = std::clamp(IniFloat(L"Audio", L"MasterVolume", s.audioMasterVolume), 0.0f, 1.0f);
    s.audioEffectsVolume = std::clamp(IniFloat(L"Audio", L"EffectsVolume", s.audioEffectsVolume), 0.0f, 1.0f);
    s.audioAmbienceVolume = std::clamp(IniFloat(L"Audio", L"AmbienceVolume", s.audioAmbienceVolume), 0.0f, 1.0f);
    s.audioMonsterVolume = std::clamp(IniFloat(L"Audio", L"MonsterVolume", s.audioMonsterVolume), 0.0f, 1.0f);

    s.paperDensity = std::clamp(IniFloat(L"Atmosphere", L"PaperDensity", s.paperDensity), 0.0f, 4.0f);
    s.hallwayPaperRunDensity = std::clamp(IniFloat(L"Atmosphere", L"HallwayPaperRunDensity", s.hallwayPaperRunDensity), 0.0f, 4.0f);
    s.chairDensity = std::clamp(IniFloat(L"Atmosphere", L"ChairDensity", s.chairDensity), 0.0f, 4.0f);
    s.waterDamageEnabled = IniInt(L"Atmosphere", L"WaterDamageEnabled", s.waterDamageEnabled ? 1 : 0) != 0;
    s.waterDamageDensity = std::clamp(IniFloat(L"Atmosphere", L"WaterDamageDensity", s.waterDamageDensity), 0.0f, 4.0f);
    s.metalCabinetDensity = std::clamp(IniFloat(L"Atmosphere", L"MetalCabinetDensity", s.metalCabinetDensity), 0.0f, 4.0f);
    s.jumpscareFrequency = std::clamp(IniFloat(L"Atmosphere", L"JumpscareFrequency", s.jumpscareFrequency), 0.0f, 1.0f);
    s.sparkParticles = IniInt(L"Atmosphere", L"SparkParticles", s.sparkParticles ? 1 : 0) != 0;
    s.sparkEmitterRatio = std::clamp(IniFloat(L"Atmosphere", L"SparkEmitterRatio", s.sparkEmitterRatio), 0.0f, 1.0f);
    s.sparkBurstMinSeconds = std::clamp(IniFloat(L"Atmosphere", L"SparkBurstMinSeconds", s.sparkBurstMinSeconds), 0.05f, 60.0f);
    s.sparkBurstMaxSeconds = std::max(s.sparkBurstMinSeconds, std::clamp(IniFloat(L"Atmosphere", L"SparkBurstMaxSeconds", s.sparkBurstMaxSeconds), 0.05f, 60.0f));
    s.sparkMaxParticles = std::clamp(IniInt(L"Atmosphere", L"SparkMaxParticles", s.sparkMaxParticles), 0, 1200);
    s.sparkSize = std::clamp(IniFloat(L"Atmosphere", L"SparkSize", s.sparkSize), 0.1f, 5.0f);
    s.airParticles = IniInt(L"Atmosphere", L"AirParticles", s.airParticles ? 1 : 0) != 0;
    s.airParticleDensity = std::clamp(IniFloat(L"Atmosphere", L"AirParticleDensity", s.airParticleDensity), 0.0f, 4.0f);
    s.airParticleSize = std::clamp(IniFloat(L"Atmosphere", L"AirParticleSize", s.airParticleSize), 0.20f, 4.0f);
    s.airParticleBlur = std::clamp(IniFloat(L"Atmosphere", L"AirParticleBlur", s.airParticleBlur), 0.0f, 3.0f);
    s.bloodSplatterDensity = std::clamp(IniFloat(L"Atmosphere", L"BloodSplatterDensity", s.bloodSplatterDensity), 0.0f, 4.0f);
    s.waterDamageEnabled = false;
    s.waterDamageDensity = 0.0f;
    s.bloodSplatterDensity = 0.0f;
    s.bloodBurstCount = std::clamp(IniInt(L"Atmosphere", L"BloodBurstCount", s.bloodBurstCount), 0, 160);
    s.bloodWetness = std::clamp(IniFloat(L"Atmosphere", L"BloodWetness", s.bloodWetness), 0.0f, 3.0f);
    s.bloodStreamCount = std::clamp(IniInt(L"Atmosphere", L"BloodStreamCount", s.bloodStreamCount), 4, 32);
    s.bloodStreamThickness = std::clamp(IniFloat(L"Atmosphere", L"BloodStreamThickness", s.bloodStreamThickness), 0.10f, 2.0f);
    s.bloodShaderQuality = std::clamp(IniFloat(L"Atmosphere", L"BloodShaderQuality", s.bloodShaderQuality), 0.25f, 1.0f);
    s.bloodWorldFlicker = IniInt(L"Atmosphere", L"BloodWorldFlicker", s.bloodWorldFlicker ? 1 : 0) != 0;
    s.bloodWorldAlwaysOn = IniInt(L"Atmosphere", L"BloodWorldAlwaysOn", s.bloodWorldAlwaysOn ? 1 : 0) != 0;
    s.bloodWorldCoverage = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldCoverage", s.bloodWorldCoverage), 0.0f, 1.0f);
    s.bloodWorldFlickerMinSeconds = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerMinSeconds", s.bloodWorldFlickerMinSeconds), 60.0f, 7200.0f);
    s.bloodWorldFlickerMaxSeconds = std::max(s.bloodWorldFlickerMinSeconds, std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerMaxSeconds", s.bloodWorldFlickerMaxSeconds), 60.0f, 7200.0f));
    s.bloodWorldFlickerDuration = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerDuration", s.bloodWorldFlickerDuration), 0.15f, 8.0f);
    s.bloodWorldFlickerIntensity = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerIntensity", s.bloodWorldFlickerIntensity), 0.0f, 2.0f);
    s.bloodStudyView = IniInt(L"Atmosphere", L"BloodStudyView", s.bloodStudyView ? 1 : 0) != 0;
    s.fleshFlicker = IniInt(L"Atmosphere", L"FleshFlicker", s.fleshFlicker ? 1 : 0) != 0;
    s.fleshAlwaysOn = IniInt(L"Atmosphere", L"FleshAlwaysOn", s.fleshAlwaysOn ? 1 : 0) != 0;
    s.fleshWetness = std::clamp(IniFloat(L"Atmosphere", L"FleshWetness", s.fleshWetness), 0.0f, 4.0f);
    s.fleshParallaxScale = std::clamp(IniFloat(L"Atmosphere", L"FleshParallaxScale", s.fleshParallaxScale), 0.0f, 0.50f);
    s.fleshFlickerMinSeconds = std::clamp(IniFloat(L"Atmosphere", L"FleshFlickerMinSeconds", s.fleshFlickerMinSeconds), 60.0f, 7200.0f);
    s.fleshFlickerMaxSeconds = std::max(s.fleshFlickerMinSeconds, std::clamp(IniFloat(L"Atmosphere", L"FleshFlickerMaxSeconds", s.fleshFlickerMaxSeconds), 60.0f, 7200.0f));
    s.fleshFlickerDuration = std::clamp(IniFloat(L"Atmosphere", L"FleshFlickerDuration", s.fleshFlickerDuration), 0.15f, 8.0f);
    s.fleshFlickerIntensity = std::clamp(IniFloat(L"Atmosphere", L"FleshFlickerIntensity", s.fleshFlickerIntensity), 0.0f, 2.0f);

    s.effectBloodLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"BloodLoopSeconds", s.effectBloodLoopSeconds), 1.0f, 180.0f);
    s.effectBloodFullSpreadAge = std::clamp(IniFloat(L"EffectTuning", L"BloodFullSpreadAge", s.effectBloodFullSpreadAge), 0.1f, s.effectBloodLoopSeconds);
    s.effectWaterLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"WaterLoopSeconds", s.effectWaterLoopSeconds), 0.5f, 60.0f);
    s.effectAirVentLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"AirVentLoopSeconds", s.effectAirVentLoopSeconds), 0.5f, 60.0f);
    s.effectBrokenLampLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"BrokenLampLoopSeconds", s.effectBrokenLampLoopSeconds), 0.5f, 60.0f);
    s.effectStaticLoopSeconds = std::clamp(IniFloat(L"EffectTuning", L"StaticLoopSeconds", s.effectStaticLoopSeconds), 0.5f, 60.0f);
    s.effectBrokenLampSparkIntensityMin = std::clamp(IniFloat(L"EffectTuning", L"BrokenLampSparkIntensityMin", s.effectBrokenLampSparkIntensityMin), 0.1f, 12.0f);
    s.effectBrokenLampSparkIntensityMax = std::max(s.effectBrokenLampSparkIntensityMin,
        std::clamp(IniFloat(L"EffectTuning", L"BrokenLampSparkIntensityMax", s.effectBrokenLampSparkIntensityMax), 0.1f, 16.0f));
    s.effectBrokenLampChainIntensityScale = std::clamp(IniFloat(L"EffectTuning", L"BrokenLampChainIntensityScale", s.effectBrokenLampChainIntensityScale), 0.0f, 4.0f);
    s.effectBrokenLampChainBurstsMin = std::clamp(IniInt(L"EffectTuning", L"BrokenLampChainBurstsMin", s.effectBrokenLampChainBurstsMin), 0, 16);
    s.effectBrokenLampChainBurstsMax = std::max(s.effectBrokenLampChainBurstsMin,
        std::clamp(IniInt(L"EffectTuning", L"BrokenLampChainBurstsMax", s.effectBrokenLampChainBurstsMax), 0, 24));
    s.effectAirVentSteamIntensityMin = std::clamp(IniFloat(L"EffectTuning", L"AirVentSteamIntensityMin", s.effectAirVentSteamIntensityMin), 0.1f, 12.0f);
    s.effectAirVentSteamIntensityMax = std::max(s.effectAirVentSteamIntensityMin,
        std::clamp(IniFloat(L"EffectTuning", L"AirVentSteamIntensityMax", s.effectAirVentSteamIntensityMax), 0.1f, 16.0f));
    s.effectAirVentPanelDropEvery = std::clamp(IniInt(L"EffectTuning", L"AirVentPanelDropEvery", s.effectAirVentPanelDropEvery), 1, 32);
    s.effectAirVentPanelDropChance = std::clamp(IniFloat(L"EffectTuning", L"AirVentPanelDropChance", s.effectAirVentPanelDropChance), 0.0f, 1.0f);

    s.dreadEnabled = IniInt(L"Dread", L"Enabled", s.dreadEnabled ? 1 : 0) != 0;
    s.dreadDebugMeter = IniInt(L"Dread", L"DebugMeter", s.dreadDebugMeter ? 1 : 0) != 0;
    s.dreadDecayPerSecond = std::clamp(IniFloat(L"Dread", L"DecayPerSecond", s.dreadDecayPerSecond), 0.0f, 1.0f);
    s.dreadMonsterDistance = std::clamp(IniFloat(L"Dread", L"MonsterDistance", s.dreadMonsterDistance), 1.0f, 60.0f);
    s.dreadMonsterGainPerSecond = std::clamp(IniFloat(L"Dread", L"MonsterGainPerSecond", s.dreadMonsterGainPerSecond), 0.0f, 3.0f);
    s.dreadJumpscareGain = std::clamp(IniFloat(L"Dread", L"JumpscareGain", s.dreadJumpscareGain), 0.0f, 1.0f);
    s.dreadFleshGain = std::clamp(IniFloat(L"Dread", L"FleshGain", s.dreadFleshGain), 0.0f, 1.0f);
    s.dreadWalkSpeedBoost = std::clamp(IniFloat(L"Dread", L"WalkSpeedBoost", s.dreadWalkSpeedBoost), 0.0f, 2.0f);
    s.dreadRunSpeedBoost = std::clamp(IniFloat(L"Dread", L"RunSpeedBoost", s.dreadRunSpeedBoost), 0.0f, 2.0f);
    s.dreadFlashlightFlicker = std::clamp(IniFloat(L"Dread", L"FlashlightFlicker", s.dreadFlashlightFlicker), 0.0f, 3.0f);

    s.monsterScale = std::clamp(IniFloat(L"Monster", L"MonsterScale", s.monsterScale), 0.25f, 4.0f);
    s.monsterSpeed = std::clamp(IniFloat(L"Monster", L"MonsterSpeed", s.monsterSpeed), 0.1f, 4.0f);
    s.monsterSprintSpeed = std::clamp(IniFloat(L"Monster", L"MonsterSprintSpeed", s.monsterSprintSpeed), 0.1f, 4.0f);
    s.monsterIgnorePlayer = IniInt(L"Monster", L"MonsterIgnorePlayer", s.monsterIgnorePlayer ? 1 : 0) != 0;
    s.monsterKillDistance = std::clamp(IniFloat(L"Monster", L"MonsterKillDistance", s.monsterKillDistance), 0.2f, 4.0f);
    s.monsterVisibleDistance = std::clamp(IniFloat(L"Monster", L"MonsterVisibleDistance", s.monsterVisibleDistance), 1.0f, 60.0f);
    s.monsterSkullMesh = IniString(L"Monster", L"SkullMesh", s.monsterSkullMesh.c_str());
    s.monsterAltSkullMesh = IniString(L"Monster", L"AlternateSkullMesh", s.monsterAltSkullMesh.c_str());
    s.monsterAltSkullChance = std::clamp(IniFloat(L"Monster", L"AlternateSkullChance", s.monsterAltSkullChance), 0.0f, 1.0f);
    s.monsterSkullMaxTriangles = std::clamp(IniInt(L"Monster", L"SkullMaxTriangles", s.monsterSkullMaxTriangles), 0, 90000);
    auto normalizeLegacyMonsterMesh = [](std::wstring& meshPath) {
        std::wstring lowered = meshPath;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(towlower(c));
        });
        if (lowered.find(L"white-tailed deer skull") != std::wstring::npos ||
            lowered.find(L"ram_skull") != std::wstring::npos) {
            meshPath = L"assets\\models\\monster_face_mask\\horror_mask.obj";
        }
    };
    normalizeLegacyMonsterMesh(s.monsterSkullMesh);
    normalizeLegacyMonsterMesh(s.monsterAltSkullMesh);
    if (s.monsterSkullMesh.empty()) {
        s.monsterSkullMesh = L"assets\\models\\monster_face_mask\\horror_mask.obj";
    }
    {
        std::wstring loweredSkull = s.monsterSkullMesh;
        std::transform(loweredSkull.begin(), loweredSkull.end(), loweredSkull.begin(), [](wchar_t c) {
            return static_cast<wchar_t>(towlower(c));
        });
        if (loweredSkull.find(L"horror_mask") != std::wstring::npos ||
            loweredSkull.find(L"monster_face_mask") != std::wstring::npos) {
            s.monsterSkullMaxTriangles = std::max(s.monsterSkullMaxTriangles, 16000);
        }
    }
    if (s.monsterAltSkullMesh.empty()) {
        s.monsterAltSkullChance = 0.0f;
    }
    s.monsterSkullYawDegrees = std::clamp(IniFloat(L"Monster", L"SkullYawDegrees", s.monsterSkullYawDegrees), -180.0f, 180.0f);
    s.monsterSkullPitchDegrees = std::clamp(IniFloat(L"Monster", L"SkullPitchDegrees", s.monsterSkullPitchDegrees), -180.0f, 180.0f);
    s.monsterSkullRollDegrees = std::clamp(IniFloat(L"Monster", L"SkullRollDegrees", s.monsterSkullRollDegrees), -180.0f, 180.0f);
    s.monsterAltSkullYawDegrees = std::clamp(IniFloat(L"Monster", L"AlternateSkullYawDegrees", s.monsterAltSkullYawDegrees), -180.0f, 180.0f);
    s.monsterAltSkullPitchDegrees = std::clamp(IniFloat(L"Monster", L"AlternateSkullPitchDegrees", s.monsterAltSkullPitchDegrees), -180.0f, 180.0f);
    s.monsterAltSkullRollDegrees = std::clamp(IniFloat(L"Monster", L"AlternateSkullRollDegrees", s.monsterAltSkullRollDegrees), -180.0f, 180.0f);
    s.monsterRightEyeX = std::clamp(IniFloat(L"Monster", L"RightEyeX", s.monsterRightEyeX), -0.45f, 0.45f);
    s.monsterRightEyeY = std::clamp(IniFloat(L"Monster", L"RightEyeY", s.monsterRightEyeY), -0.45f, 0.20f);
    s.monsterRightEyeZ = std::clamp(IniFloat(L"Monster", L"RightEyeZ", s.monsterRightEyeZ), -0.35f, 0.20f);
    s.monsterLeftEyeX = std::clamp(IniFloat(L"Monster", L"LeftEyeX", s.monsterLeftEyeX), -0.45f, 0.45f);
    s.monsterLeftEyeY = std::clamp(IniFloat(L"Monster", L"LeftEyeY", s.monsterLeftEyeY), -0.45f, 0.20f);
    s.monsterLeftEyeZ = std::clamp(IniFloat(L"Monster", L"LeftEyeZ", s.monsterLeftEyeZ), -0.35f, 0.20f);
    s.monsterAltRightEyeX = std::clamp(IniFloat(L"Monster", L"AlternateRightEyeX", s.monsterAltRightEyeX), -0.45f, 0.45f);
    s.monsterAltRightEyeY = std::clamp(IniFloat(L"Monster", L"AlternateRightEyeY", s.monsterAltRightEyeY), -0.45f, 0.20f);
    s.monsterAltRightEyeZ = std::clamp(IniFloat(L"Monster", L"AlternateRightEyeZ", s.monsterAltRightEyeZ), -0.35f, 0.20f);
    s.monsterAltLeftEyeX = std::clamp(IniFloat(L"Monster", L"AlternateLeftEyeX", s.monsterAltLeftEyeX), -0.45f, 0.45f);
    s.monsterAltLeftEyeY = std::clamp(IniFloat(L"Monster", L"AlternateLeftEyeY", s.monsterAltLeftEyeY), -0.45f, 0.20f);
    s.monsterAltLeftEyeZ = std::clamp(IniFloat(L"Monster", L"AlternateLeftEyeZ", s.monsterAltLeftEyeZ), -0.35f, 0.20f);
    return s;
}

std::filesystem::path ResolveAsset(const Settings& settings, const std::wstring& filename) {
    std::vector<std::filesystem::path> roots;
    auto add = [&](std::filesystem::path p) {
        p = p.lexically_normal();
        if (std::find(roots.begin(), roots.end(), p) == roots.end()) roots.push_back(std::move(p));
    };
    std::filesystem::path folder(settings.assetFolder);
    if (!folder.empty()) {
        if (folder.is_absolute()) add(folder);
        else {
            add(ModuleDirectory() / folder);
            add(ModuleDirectory().parent_path() / folder);
            add(ModuleDirectory().parent_path().parent_path() / folder);
            add(std::filesystem::current_path() / folder);
        }
    }
    for (const auto& root : CandidateAssetRoots()) add(root);
    for (const auto& root : roots) {
        std::filesystem::path p = root / filename;
        std::error_code ec;
        if (std::filesystem::exists(p, ec)) return p;
    }
    return {};
}

uint32_t ResolveRuntimeSeed(uint32_t configuredSeed) {
    if (configuredSeed != 0) return configuredSeed;

    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    uint64_t mix = static_cast<uint64_t>(counter.QuadPart);
    mix ^= GetTickCount64() + 0x9e3779b97f4a7c15ull;
    mix ^= static_cast<uint64_t>(GetCurrentProcessId()) << 32;
    mix ^= static_cast<uint64_t>(GetCurrentThreadId()) << 16;
    mix ^= static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    mix ^= static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&mix));

    uint64_t osRandom[2]{};
    if (SystemFunction036(osRandom, sizeof(osRandom))) {
        mix ^= osRandom[0];
        mix ^= osRandom[1] + 0x9e3779b97f4a7c15ull + (mix << 6) + (mix >> 2);
    }

    std::random_device rd;
    mix ^= static_cast<uint64_t>(rd()) << 1;
    mix ^= static_cast<uint64_t>(rd()) << 33;

    mix ^= mix >> 33;
    mix *= 0xff51afd7ed558ccdull;
    mix ^= mix >> 33;
    mix *= 0xc4ceb9fe1a85ec53ull;
    mix ^= mix >> 33;
    uint32_t seed = static_cast<uint32_t>(mix ^ (mix >> 32));
    return seed != 0 ? seed : 1u;
}

float RuntimeSigned(uint32_t seed, int salt) {
    return Rand01(salt, salt * 37 + 19, seed ^ 0x6d2b79f5u) * 2.0f - 1.0f;
}

float JitterScaled(uint32_t seed, int salt, float value, float amount, float maxFraction, float minValue, float maxValue) {
    if (amount <= 0.0001f || std::abs(value) <= 0.0001f) return std::clamp(value, minValue, maxValue);
    float scale = 1.0f + RuntimeSigned(seed, salt) * amount * maxFraction;
    return std::clamp(value * scale, minValue, maxValue);
}

int JitterInt(uint32_t seed, int salt, int value, float amount, float maxFraction, int minValue, int maxValue) {
    if (value == 0) return std::clamp(value, minValue, maxValue);
    float jittered = JitterScaled(seed, salt, static_cast<float>(value), amount, maxFraction,
        static_cast<float>(minValue), static_cast<float>(maxValue));
    return std::clamp(static_cast<int>(std::round(jittered)), minValue, maxValue);
}

int JitterIntRange(uint32_t seed, int salt, int value, int range, float amount, int minValue, int maxValue) {
    if (amount <= 0.0001f || range <= 0) return std::clamp(value, minValue, maxValue);
    float delta = RuntimeSigned(seed, salt) * amount * static_cast<float>(range);
    return std::clamp(value + static_cast<int>(std::round(delta)), minValue, maxValue);
}

void ApplyRuntimeVariation(Settings& s, uint32_t seed) {
    float v = Clamp01(s.runVariation);
    if (v <= 0.0001f) return;
    constexpr float kUserFraction = 0.15f;

    s.roomCount = JitterIntRange(seed, 101, s.roomCount, s.roomCountRange, v, 0, 80);
    s.roomMinRadius = JitterIntRange(seed, 102, s.roomMinRadius, s.roomMinRadiusRange, v, 1, 12);
    s.roomMaxRadius = JitterIntRange(seed, 103, s.roomMaxRadius, s.roomMaxRadiusRange, v, s.roomMinRadius, 16);

    s.flashlightIntensity = JitterScaled(seed, 201, s.flashlightIntensity, v, 0.18f, 0.0f, 10.0f);
    s.flashlightAttenuation = JitterScaled(seed, 202, s.flashlightAttenuation, v, 0.22f, 0.001f, 2.0f);
    s.flashlightConeDegrees = JitterScaled(seed, 203, s.flashlightConeDegrees, v, 0.10f, 20.0f, 140.0f);
    s.ambientLight = JitterScaled(seed, 204, s.ambientLight, v, 0.45f, 0.0f, 1.0f);
    s.lampIntensity = JitterScaled(seed, 205, s.lampIntensity, v, 0.38f, 0.0f, 10.0f);
    s.lampOnRatio = JitterScaled(seed, 207, s.lampOnRatio, v, 0.65f, 0.0f, 1.0f);
    s.lampFlickerRatio = JitterScaled(seed, 208, s.lampFlickerRatio, v, 0.55f, 0.0f, 1.0f);
    s.brokenZoneRatio = JitterScaled(seed, 209, s.brokenZoneRatio, v, 0.55f, 0.0f, 1.0f);
    s.fogStartMeters = JitterScaled(seed, 211, s.fogStartMeters, v, 0.22f, 0.0f, 200.0f);
    s.fogEndMeters = std::max(s.fogStartMeters + 0.1f, JitterScaled(seed, 212, s.fogEndMeters, v, 0.22f, 0.1f, 300.0f));
    s.fogDarkness = JitterScaled(seed, 213, s.fogDarkness, v, 0.28f, 0.0f, 1.0f);
    s.cornerAOIntensity = JitterScaled(seed, 214, s.cornerAOIntensity, v, 0.24f, 0.0f, 1.0f);
    s.exposure = JitterScaled(seed, 215, s.exposure, v, 0.12f, 0.1f, 8.0f);
    s.motionBlurAmount = JitterScaled(seed, 216, s.motionBlurAmount, v, 0.10f, 0.0f, 2.0f);
    s.bloomAmount = JitterScaled(seed, 217, s.bloomAmount, v, 0.10f, 0.0f, 2.0f);
    s.lensDirtAmount = JitterScaled(seed, 218, s.lensDirtAmount, v, 0.10f, 0.0f, 2.0f);

    s.walkSpeed = JitterScaled(seed, 301, s.walkSpeed, v, kUserFraction, 0.1f, 8.0f);
    s.roomSpeed = JitterScaled(seed, 302, s.roomSpeed, v, kUserFraction, 0.1f, 8.0f);
    s.runSpeed = JitterScaled(seed, 303, s.runSpeed, v, kUserFraction, 0.1f, 12.0f);
    s.turnLookAheadTiles = JitterScaled(seed, 304, s.turnLookAheadTiles, v, kUserFraction, 0.0f, 8.0f);
    s.roomLookAheadTiles = JitterScaled(seed, 305, s.roomLookAheadTiles, v, kUserFraction, 0.0f, 10.0f);
    s.roomPauseChance = JitterScaled(seed, 306, s.roomPauseChance, v, kUserFraction, 0.0f, 1.0f);
    s.junctionScanChance = JitterScaled(seed, 307, s.junctionScanChance, v, kUserFraction, 0.0f, 1.0f);
    s.scanAngleDegrees = JitterScaled(seed, 308, s.scanAngleDegrees, v, kUserFraction, 0.0f, 160.0f);
    float lookBackMin = JitterScaled(seed, 309, s.lookBackMinSeconds, v, kUserFraction, 2.0f, 300.0f);
    float lookBackMax = JitterScaled(seed, 310, s.lookBackMaxSeconds, v, kUserFraction, 2.0f, 300.0f);
    s.lookBackMinSeconds = std::min(lookBackMin, lookBackMax);
    s.lookBackMaxSeconds = std::max(lookBackMin, lookBackMax);
    s.headBobAmount = JitterScaled(seed, 311, s.headBobAmount, v, kUserFraction, 0.0f, 0.4f);
    s.sideSwayAmount = JitterScaled(seed, 312, s.sideSwayAmount, v, kUserFraction, 0.0f, 0.3f);
    s.junctionScanBaseSeconds = JitterScaled(seed, 313, s.junctionScanBaseSeconds, v, kUserFraction, 0.0f, 4.0f);
    s.junctionScanBranchSeconds = JitterScaled(seed, 314, s.junctionScanBranchSeconds, v, kUserFraction, 0.0f, 3.0f);
    s.flashlightSwayAmount = JitterScaled(seed, 315, s.flashlightSwayAmount, v, kUserFraction, 0.0f, 4.0f);
    s.flashlightFollowSpeed = JitterScaled(seed, 316, s.flashlightFollowSpeed, v, kUserFraction, 0.1f, 4.0f);
    s.flashlightPanicDartAmount = JitterScaled(seed, 317, s.flashlightPanicDartAmount, v, kUserFraction, 0.0f, 4.0f);

    s.paperDensity = JitterScaled(seed, 401, s.paperDensity, v, kUserFraction, 0.0f, 4.0f);
    s.hallwayPaperRunDensity = JitterScaled(seed, 402, s.hallwayPaperRunDensity, v, kUserFraction, 0.0f, 4.0f);
    s.chairDensity = JitterScaled(seed, 403, s.chairDensity, v, kUserFraction, 0.0f, 4.0f);
    s.waterDamageDensity = JitterScaled(seed, 406, s.waterDamageDensity, v, kUserFraction, 0.0f, 4.0f);
    s.waterDamageEnabled = false;
    s.waterDamageDensity = 0.0f;
    s.bloodSplatterDensity = 0.0f;
    s.metalCabinetDensity = JitterScaled(seed, 407, s.metalCabinetDensity, v, kUserFraction, 0.0f, 4.0f);
    s.jumpscareFrequency = JitterScaled(seed, 408, s.jumpscareFrequency, v, kUserFraction, 0.0f, 1.0f);
    s.sparkEmitterRatio = JitterScaled(seed, 409, s.sparkEmitterRatio, v, kUserFraction, 0.0f, 1.0f);
    float sparkBurstMin = JitterScaled(seed, 410, s.sparkBurstMinSeconds, v, kUserFraction, 0.05f, 60.0f);
    float sparkBurstMax = JitterScaled(seed, 411, s.sparkBurstMaxSeconds, v, kUserFraction, 0.05f, 60.0f);
    s.sparkBurstMinSeconds = std::min(sparkBurstMin, sparkBurstMax);
    s.sparkBurstMaxSeconds = std::max(sparkBurstMin, sparkBurstMax);
    s.sparkMaxParticles = JitterInt(seed, 412, s.sparkMaxParticles, v, kUserFraction, 0, 1200);
    s.sparkSize = JitterScaled(seed, 413, s.sparkSize, v, kUserFraction, 0.1f, 5.0f);
    s.airParticleDensity = JitterScaled(seed, 414, s.airParticleDensity, v, kUserFraction, 0.0f, 4.0f);
    s.airParticleSize = JitterScaled(seed, 415, s.airParticleSize, v, kUserFraction, 0.20f, 4.0f);
    s.airParticleBlur = JitterScaled(seed, 416, s.airParticleBlur, v, kUserFraction, 0.0f, 3.0f);

    s.dreadDecayPerSecond = JitterScaled(seed, 501, s.dreadDecayPerSecond, v, 0.30f, 0.0f, 1.0f);
    s.dreadMonsterDistance = JitterScaled(seed, 502, s.dreadMonsterDistance, v, 0.22f, 1.0f, 60.0f);
    s.dreadMonsterGainPerSecond = JitterScaled(seed, 503, s.dreadMonsterGainPerSecond, v, 0.28f, 0.0f, 3.0f);
    s.dreadJumpscareGain = JitterScaled(seed, 504, s.dreadJumpscareGain, v, 0.30f, 0.0f, 1.0f);
    s.dreadFleshGain = JitterScaled(seed, 505, s.dreadFleshGain, v, 0.30f, 0.0f, 1.0f);
    s.dreadWalkSpeedBoost = JitterScaled(seed, 506, s.dreadWalkSpeedBoost, v, 0.24f, 0.0f, 2.0f);
    s.dreadRunSpeedBoost = JitterScaled(seed, 507, s.dreadRunSpeedBoost, v, 0.24f, 0.0f, 2.0f);
    s.dreadFlashlightFlicker = JitterScaled(seed, 508, s.dreadFlashlightFlicker, v, 0.30f, 0.0f, 3.0f);

    s.monsterScale = JitterScaled(seed, 601, s.monsterScale, v, 0.14f, 0.25f, 4.0f);
    s.monsterSpeed = JitterScaled(seed, 602, s.monsterSpeed, v, 0.22f, 0.1f, 4.0f);
    s.monsterSprintSpeed = JitterScaled(seed, 603, s.monsterSprintSpeed, v, 0.22f, 0.1f, 4.0f);
    s.monsterKillDistance = JitterScaled(seed, 604, s.monsterKillDistance, v, 0.10f, 0.2f, 4.0f);
    s.monsterVisibleDistance = JitterScaled(seed, 605, s.monsterVisibleDistance, v, 0.25f, 1.0f, 60.0f);
}
