// Effect debug viewer state, prop labels, and effect-tuning constants.
// Included from main.cpp before settings and renderer code.

enum class DebugSliceEffect {
    Blood = 0,
    FloorWater,
    CeilingWater,
    WallWater,
    CeilingLamps,
    BrokenLamps,
    AirVents,
    Props,
    Count
};

bool gEffectDebugViewer = false;
DebugSliceEffect gDebugSliceEffect = DebugSliceEffect::Blood;
int gDebugSliceTiles = 3;
int gDebugPropIndex = 0;
bool gBloodDebugEveryWall = false;

struct StartupProgressUpdate {
    const wchar_t* phase = L"";
    const wchar_t* detail = L"";
    int current = 0;
    int total = 1;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
};

using StartupProgressCallback = void (*)(void*, const StartupProgressUpdate&);

struct StartupProgressSink {
    StartupProgressCallback callback = nullptr;
    void* context = nullptr;

    void Report(const StartupProgressUpdate& update) const {
        if (callback) callback(context, update);
    }
};

const wchar_t* DebugSliceEffectName(DebugSliceEffect effect) {
    switch (effect) {
    case DebugSliceEffect::Blood: return L"Blood";
    case DebugSliceEffect::FloorWater: return L"Floor water";
    case DebugSliceEffect::CeilingWater: return L"Ceiling water";
    case DebugSliceEffect::WallWater: return L"Wall water";
    case DebugSliceEffect::CeilingLamps: return L"Ceiling lamps";
    case DebugSliceEffect::BrokenLamps: return L"Broken lamps";
    case DebugSliceEffect::AirVents: return L"Air vents";
    case DebugSliceEffect::Props: return L"Props";
    default: return L"Unknown";
    }
}

bool DebugSliceEffectIsWater(DebugSliceEffect effect) {
    return effect == DebugSliceEffect::FloorWater ||
        effect == DebugSliceEffect::CeilingWater ||
        effect == DebugSliceEffect::WallWater;
}

constexpr int kDebugPropCount = 16;

int WrapDebugPropIndex(int index) {
    int count = std::max(1, kDebugPropCount);
    index %= count;
    if (index < 0) index += count;
    return index;
}

const wchar_t* DebugPropName(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 0: return L"Office chair modern";
    case 1: return L"Office chair classic";
    case 2: return L"Office chair task";
    case 3: return L"Office chair tipped";
    case 4: return L"Filing cabinet";
    case 5: return L"Office desk";
    case 6: return L"Trash bin upright";
    case 7: return L"Trash bin tipped";
    case 8: return L"Desk lamp";
    case 9: return L"Audio cassette";
    case 10: return L"Air vent";
    case 11: return L"Exit sign";
    case 12: return L"Ceiling lamp 01";
    case 13: return L"Ceiling lamp 02";
    case 14: return L"Ceiling lamp 03";
    case 15: return L"Ceiling lamp 04";
    default: return L"Prop";
    }
}

float DebugPropCameraDistanceScale(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 4:
    case 5:
        return 2.15f;
    case 8:
    case 9:
        return 1.12f;
    case 10:
    case 11:
        return 1.30f;
    default:
        return 1.62f;
    }
}

float DebugPropCameraTargetY(int index) {
    switch (WrapDebugPropIndex(index)) {
    case 8:
    case 9:
        return 0.22f;
    case 4:
        return 0.76f;
    case 5:
        return 0.58f;
    case 10:
    case 11:
        return 0.46f;
    default:
        return 0.52f;
    }
}

struct EffectFloatRange {
    float min = 0.0f;
    float max = 0.0f;
};

struct EffectIntRange {
    int min = 0;
    int max = 0;
};

constexpr float kEffectBloodLoopSeconds = 56.0f;
constexpr float kEffectBloodFullSpreadAge = 48.0f;
constexpr float kEffectWaterLoopSeconds = 7.5f;
constexpr float kEffectAirVentLoopSeconds = 6.2f;
constexpr float kEffectBrokenLampLoopSeconds = 5.2f;
constexpr float kEffectStaticLoopSeconds = 8.0f;
constexpr EffectFloatRange kEffectBrokenLampSparkIntensity{2.2f, 4.1f};
constexpr float kEffectBrokenLampChainIntensityScale = 0.70f;
constexpr EffectIntRange kEffectBrokenLampChainBursts{1, 3};
constexpr EffectFloatRange kEffectAirVentSteamIntensity{1.9f, 3.2f};
constexpr int kEffectAirVentPanelDropEvery = 3;
constexpr float kEffectAirVentPanelDropChance = 1.0f / static_cast<float>(kEffectAirVentPanelDropEvery);
constexpr float kVentDropFloorY = 0.026f;
constexpr size_t kMaxVentDrops = 64;
constexpr float kEffectDebugAmbientLight = 0.32f;
constexpr float kEffectDebugExposureMax = 1.0f;
constexpr float kEffectDebugBloomMax = 0.30f;
constexpr float kEffectDebugFlashlightIntensity = 0.82f;
constexpr float kEffectDebugLampIntensity = 1.35f;
constexpr float kEffectBloodStreamCount = 30.0f;
constexpr float kEffectBloodStreamThickness = 0.88f;
constexpr float kEffectBloodShaderQuality = 1.0f;
constexpr float kEffectBloodRevealRadius = 100000.0f;
