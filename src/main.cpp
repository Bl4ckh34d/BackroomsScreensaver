#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

extern "C" BOOLEAN NTAPI SystemFunction036(PVOID RandomBuffer, ULONG RandomBufferLength);
#pragma comment(lib, "advapi32.lib")

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr int kMazeW = 25;
constexpr int kMazeH = 25;
constexpr float kTile = 1.6f;
constexpr float kWallH = 2.4f;
constexpr float kFloorTextureMeters = 1.8f;
constexpr float kWallTextureMeters = 1.8f;
constexpr float kCeilingTextureMeters = 0.0f;
constexpr int kTextureSize = 512;
constexpr int kMaterialCount = 26;
constexpr int kDynamicVertexCapacity = 220000;
constexpr int kOverlayVertexCapacity = 160000;
constexpr float kMonsterHeadForwardOffset = 0.34f;
constexpr float kMonsterSmokeBackOffset = -0.18f;
constexpr float kBloodFloorDecalLift = 0.006f;
constexpr float kBloodCeilingDecalInset = 0.008f;
constexpr float kBloodFloorDecalLayerStep = 0.00010f;
constexpr float kBloodCeilingDecalLayerStep = 0.00080f;

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

struct Vertex {
    XMFLOAT3 pos;
    XMFLOAT3 normal;
    XMFLOAT3 tangent;
    XMFLOAT2 uv;
    float material;
};
static_assert(sizeof(Vertex) == sizeof(float) * 12, "Vertex binary layout must stay packed for runtime mesh files.");

#pragma pack(push, 1)
struct PackedStaticPropVertexV2 {
    uint16_t px;
    uint16_t py;
    uint16_t pz;
    int16_t nx;
    int16_t ny;
    int16_t nz;
    int16_t tx;
    int16_t ty;
    int16_t tz;
    float u;
    float v;
    uint16_t material;
};

struct PackedStaticPropVertex {
    uint16_t px;
    uint16_t py;
    uint16_t pz;
    int16_t nx;
    int16_t ny;
    int16_t nz;
    int16_t tx;
    int16_t ty;
    int16_t tz;
    uint16_t u;
    uint16_t v;
    uint16_t material;
};
#pragma pack(pop)
static_assert(sizeof(PackedStaticPropVertexV2) == 28, "Packed V2 static prop binary layout changed.");
static_assert(sizeof(PackedStaticPropVertex) == 24, "Packed static prop binary layout changed.");

struct OverlayVertex {
    XMFLOAT2 pos;
    XMFLOAT4 color;
};

enum class RendererRuntimeMode {
    ScreensaverAutopilot,
    PlayableGame,
    DebugViewer,
    Preview
};

struct GameInputSnapshot {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float lookDeltaX = 0.0f;
    float lookDeltaY = 0.0f;
    bool jump = false;
    bool sprint = false;
    bool crouch = false;
    bool interact = false;
    bool pause = false;
};

struct SceneConstants {
    XMFLOAT4X4 viewProj;
    XMFLOAT4X4 lightViewProj;
    XMFLOAT4 cameraPosTime;
    XMFLOAT4 cameraDirAspect;
    XMFLOAT4 lighting0;
    XMFLOAT4 lighting1;
    XMFLOAT4 fog0;
    XMFLOAT4 ao0;
    XMFLOAT4 post0;
    XMFLOAT4 post1;
    XMFLOAT4 shadow0;
    XMFLOAT4 shadow1;
    XMFLOAT4 shadow2;
    XMFLOAT4 maze0;
    XMFLOAT4 maze1;
    XMFLOAT4 texture0;
    XMFLOAT4 transition0;
    XMFLOAT4 horror0;
    XMFLOAT4 sparkLight0;
    XMFLOAT4 sparkLight1;
    XMFLOAT4 blood0;
    XMFLOAT4 blood1;
    XMFLOAT4 blood2;
    XMFLOAT4 blood3;
    XMFLOAT4 blood4;
    XMFLOAT4 blood5;
    XMFLOAT4 blood6;
    XMFLOAT4 blood7;
    XMFLOAT4 blood8;
    XMFLOAT4 air0;
    XMFLOAT4 exitLight0;
    XMFLOAT4 monsterFog0;
};

struct Tile {
    int x = 0;
    int y = 0;
};

struct SparkEmitter {
    XMFLOAT3 pos{};
    float cooldown = 0.0f;
    bool triggered = false;
};

struct SparkParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 1.0f;
    float size = 0.025f;
};

struct SparkFlash {
    XMFLOAT3 pos{};
    float age = 0.0f;
    float life = 0.18f;
    float intensity = 1.0f;
};

struct SparkChain {
    XMFLOAT3 pos{};
    float timer = 0.0f;
    float intensity = 1.0f;
    int remaining = 0;
};

struct RuntimeLampState {
    Tile tile{};
    XMFLOAT3 pos{};
    float damage = 0.0f;
    float sparkTimer = 0.0f;
    bool broken = false;
};

struct SteamEmitter {
    XMFLOAT3 pos{};
    XMFLOAT3 dir{0.0f, 0.0f, 1.0f};
    float cooldown = 0.0f;
    bool panelDropped = false;
    bool triggered = false;
};

struct SteamParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 1.0f;
    float size = 0.25f;
};

struct VentDrop {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float yaw = 0.0f;
    float roll = 0.0f;
    float angular = 0.0f;
    float age = 0.0f;
    float life = 3.0f;
    float halfW = 0.46f;
    float halfH = 0.27f;
    bool landed = false;
};

struct AirParticle {
    XMFLOAT3 pos{};
    XMFLOAT3 vel{};
    float age = 0.0f;
    float life = 32.0f;
    float size = 0.018f;
    float seed = 0.0f;
    float angle = 0.0f;
    float spin = 0.0f;
    float aspect = 1.0f;
    float nearLayer = 0.0f;
};

struct BloodScarePoint {
    XMFLOAT3 pos{};
    XMFLOAT3 source{};
    XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
    float radius = 2.2f;
    float activationTime = -1000000.0f;
    float focusDelaySeconds = 1.20f;
    float dreadScale = 1.0f;
    bool triggered = false;
    bool focusTaken = false;
    bool requireFacing = false;
    bool revealBlood = true;
    bool waterLiquid = false;
};

struct BloodRevealRegion {
    XMFLOAT3 center{};
    float radius = 0.0f;
    float activationTime = -1000000.0f;
    bool waterLiquid = false;
};

bool operator==(Tile a, Tile b) {
    return a.x == b.x && a.y == b.y;
}

float Clamp01(float v) {
    return std::clamp(v, 0.0f, 1.0f);
}

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float MoveTowards(float current, float target, float maxDelta) {
    if (maxDelta <= 0.0f) return current;
    float delta = target - current;
    if (std::abs(delta) <= maxDelta) return target;
    return current + (delta > 0.0f ? maxDelta : -maxDelta);
}

float AngleWrap(float a) {
    while (a > kPi) a -= kPi * 2.0f;
    while (a < -kPi) a += kPi * 2.0f;
    return a;
}

float SmoothStep(float a, float b, float x) {
    float t = Clamp01((x - a) / (b - a));
    return t * t * (3.0f - 2.0f * t);
}

uint32_t Hash2(int x, int y, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u + static_cast<uint32_t>(y) * 668265263u + seed * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

uint32_t Hash3(int x, int y, int z, uint32_t seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u ^
        static_cast<uint32_t>(y) * 668265263u ^
        static_cast<uint32_t>(z) * 2246822519u ^
        seed * 3266489917u;
    h = (h ^ (h >> 16)) * 2246822519u;
    h = (h ^ (h >> 13)) * 3266489917u;
    return h ^ (h >> 16);
}

float Rand01(int x, int y, uint32_t seed) {
    return (Hash2(x, y, seed) & 0x00ffffffu) / 16777215.0f;
}

float Rand01(int x, int y, int z, uint32_t seed) {
    return (Hash3(x, y, z, seed) & 0x00ffffffu) / 16777215.0f;
}

float ValueNoise(float x, float y, uint32_t seed) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - static_cast<float>(ix);
    float fy = y - static_cast<float>(iy);
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    float a = Rand01(ix, iy, seed);
    float b = Rand01(ix + 1, iy, seed);
    float c = Rand01(ix, iy + 1, seed);
    float d = Rand01(ix + 1, iy + 1, seed);
    return Lerp(Lerp(a, b, fx), Lerp(c, d, fx), fy);
}

float FractalNoise(float x, float y, uint32_t seed) {
    float sum = 0.0f;
    float amp = 0.55f;
    float freq = 1.0f;
    float norm = 0.0f;
    for (int i = 0; i < 5; ++i) {
        sum += ValueNoise(x * freq, y * freq, seed + i * 97u) * amp;
        norm += amp;
        amp *= 0.5f;
        freq *= 2.13f;
    }
    return sum / norm;
}

uint8_t Byte(float v) {
    return static_cast<uint8_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
}

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

    ComPtr<IWICBitmapSource> source = frame;
    ComPtr<IWICBitmapScaler> scaler;
    hr = factory->CreateBitmapScaler(&scaler);
    if (SUCCEEDED(hr)) {
        hr = scaler->Initialize(frame.Get(), targetW, targetH, WICBitmapInterpolationModeFant);
        if (SUCCEEDED(hr)) source = scaler;
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

struct Settings {
    bool allowWarpFallback = false;
    bool gameFullscreen = false;
    int gameResolutionWidth = 1280;
    int gameResolutionHeight = 760;

    int mazeWidth = kMazeW;
    int mazeHeight = kMazeH;
    int roomCount = 3;
    int roomMinRadius = 1;
    int roomMaxRadius = 3;
    int roomCountRange = 1;
    int roomMinRadiusRange = 1;
    int roomMaxRadiusRange = 1;
    uint32_t mazeSeed = 0;
    bool mapOverlay = true;
    float runVariation = 0.1f;
    float tileWidthMeters = kTile;
    float tileLengthMeters = kTile;
    float wallHeightMeters = kWallH;

    float floorTextureMeters = kFloorTextureMeters;
    float wallTextureMeters = kWallTextureMeters;
    float ceilingTextureMeters = kCeilingTextureMeters;
    std::wstring assetFolder = L"assets\\PBRs";
    std::wstring wallStem = L"backrooms_wall";
    std::wstring floorStem = L"backrooms_carpet";
    std::wstring ceilingStem = L"backrooms_ceiling";
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
    int flashlightShadowMapSize = 4096;
    float ambientLight = 0.0f;
    float lampIntensity = 1.45f;
    float lampSpacing = 3.2f;
    float lampOnRatio = 0.18f;
    float lampFlickerRatio = 0.38f;
    float brokenZoneRatio = 0.26f;
    float darkLampVisibleRatio = 1.0f;
    float fogStartMeters = 0.0f;
    float fogEndMeters = 12.0f;
    float fogDarkness = 1.0f;
    float cornerAOIntensity = 0.45f;
    float cornerAORadius = 0.5f;
    float floorCeilingAOIntensity = 0.3f;
    float exposure = 1.0f;
    float gamma = 1.0f;
    float motionBlurAmount = 0.18f;
    float bloomAmount = 0.22f;
    float lensDirtAmount = 0.18f;

    float walkSpeed = 1.88f;
    float roomSpeed = 1.45f;
    float runSpeed = 3.05f;
    float turnLookAheadTiles = 2.1f;
    float roomLookAheadTiles = 2.4f;
    float roomPauseChance = 0.62f;
    float junctionScanChance = 0.88f;
    float scanAngleDegrees = 55.0f;
    float lookBackMinSeconds = 5.0f;
    float lookBackMaxSeconds = 90.0f;
    float headBobAmount = 0.075f;
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

    bool audioMuted = false;
    float audioMasterVolume = 1.0f;
    float audioEffectsVolume = 1.0f;
    float audioAmbienceVolume = 1.0f;
    float audioMonsterVolume = 1.0f;

    float paperDensity = 1.0f;
    float hallwayPaperRunDensity = 1.0f;
    float chairDensity = 1.0f;
    float waterDamageDensity = 1.0f;
    float metalCabinetDensity = 0.85f;
    float jumpscareFrequency = 0.3f;
    bool sparkParticles = true;
    float sparkEmitterRatio = 0.11f;
    float sparkBurstMinSeconds = 2.8f;
    float sparkBurstMaxSeconds = 8.8f;
    int sparkMaxParticles = 160;
    float sparkSize = 1.0f;
    bool airParticles = true;
    float airParticleDensity = 1.0f;
    float airParticleSize = 1.0f;
    float airParticleBlur = 1.0f;
    float bloodSplatterDensity = 0.05f;
    int bloodBurstCount = 20;
    float bloodWetness = 0.995f;
    int bloodStreamCount = static_cast<int>(kEffectBloodStreamCount);
    float bloodStreamThickness = kEffectBloodStreamThickness;
    float bloodShaderQuality = kEffectBloodShaderQuality;
    bool bloodWorldFlicker = false;
    bool bloodWorldAlwaysOn = false;
    float bloodWorldCoverage = 1.0f;
    float bloodWorldFlickerMinSeconds = 42.0f;
    float bloodWorldFlickerMaxSeconds = 110.0f;
    float bloodWorldFlickerDuration = 1.15f;
    float bloodWorldFlickerIntensity = 1.0f;
    bool bloodStudyView = false;
    bool fleshFlicker = true;
    bool fleshAlwaysOn = false;
    float fleshWetness = 0.995f;
    float fleshParallaxScale = 0.14f;
    float fleshFlickerMinSeconds = 34.0f;
    float fleshFlickerMaxSeconds = 92.0f;
    float fleshFlickerDuration = 0.75f;
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
    float effectAirVentPanelDropChance = kEffectAirVentPanelDropChance;

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
    float monsterKillDistance = 1.18f;
    float monsterVisibleDistance = 12.0f;
    std::wstring monsterSkullMesh = L"assets\\White-Tailed Deer Skull.obj";
    std::wstring monsterAltSkullMesh = L"assets\\models\\Ram_Skull\\Ram_Skull_Scan.OBJ";
    float monsterAltSkullChance = 0.35f;
    int monsterSkullMaxTriangles = 18000;
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

std::filesystem::path SettingsPath() {
    return ModuleDirectory() / L"BackroomsMaze.ini";
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
    wchar_t value[8]{};
    return GetEnvironmentVariableW(L"BACKROOMS_PROFILE_STARTUP", value, ARRAYSIZE(value)) > 0;
}

void StartupProfileLine(const std::wstring& line) {
    if (!StartupProfileEnabled()) return;
    std::wofstream out(ModuleDirectory() / L"BackroomsMaze.profile.log", std::ios::app);
    if (out) out << line << L"\r\n";
}

class StartupProfile {
public:
    explicit StartupProfile(const wchar_t* name) : name_(name), start_(GetTickCount64()), last_(start_) {
        StartupProfileLine(L"[" + name_ + L"]");
    }

    void Mark(const wchar_t* label) {
        ULONGLONG now = GetTickCount64();
        std::wostringstream line;
        line << name_ << L" " << label
             << L": +" << (now - last_) << L" ms"
             << L", total " << (now - start_) << L" ms";
        StartupProfileLine(line.str());
        last_ = now;
    }

private:
    std::wstring name_;
    ULONGLONG start_ = 0;
    ULONGLONG last_ = 0;
};

std::wstring DefaultConfigText() {
    std::wostringstream s;
    s << L"; Backrooms Maze Screensaver settings\r\n"
      << L"; Paths can be absolute, or relative to this SCR folder/current project folder.\r\n\r\n"
      << L"[Renderer]\r\n"
      << L"AllowWarpFallback=0\r\n\r\n"
      << L"[GameWindow]\r\n"
      << L"Fullscreen=0\r\n"
      << L"ResolutionWidth=1280\r\n"
      << L"ResolutionHeight=760\r\n\r\n"
      << L"[Maze]\r\n"
      << L"Width=25\r\n"
      << L"Height=25\r\n"
      << L"RoomCount=3\r\n"
      << L"RoomMinRadius=1.5\r\n"
      << L"RoomMaxRadius=3\r\n"
      << L"RoomCountRange=1\r\n"
      << L"RoomMinRadiusRange=1\r\n"
      << L"RoomMaxRadiusRange=1\r\n"
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
      << L"FloorStem=backrooms_carpet\r\n"
      << L"CeilingStem=backrooms_ceiling\r\n"
      << L"FleshStem=downloads\\Others001_4k\\others_0001\r\n"
      << L"WallScaleMeters=1.8\r\n"
      << L"FloorScaleMeters=1.8\r\n"
      << L"; 0 auto-aligns the ceiling texture as a 2x2 panel grid per maze tile.\r\n"
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
      << L"FlashlightShadowMapSize=4096\r\n"
      << L"AmbientLight=0\r\n"
      << L"LampIntensity=1.45\r\n"
      << L"LampSpacing=3.2\r\n"
      << L"LampOnRatio=0.18\r\n"
      << L"LampFlickerRatio=0.38\r\n"
      << L"BrokenZoneRatio=0.26\r\n"
      << L"DarkLampVisibleRatio=1.0\r\n"
      << L"FogStartMeters=0\r\n"
      << L"FogEndMeters=12\r\n"
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
      << L"WalkSpeed=1.88\r\n"
      << L"RoomSpeed=1.45\r\n"
      << L"RunSpeed=3.05\r\n"
      << L"TurnLookAheadTiles=2.1\r\n"
      << L"RoomLookAheadTiles=2.4\r\n"
      << L"RoomPauseChance=0.62\r\n"
      << L"JunctionScanChance=0.88\r\n"
      << L"ScanAngleDegrees=55\r\n"
      << L"LookBackMinSeconds=5\r\n"
      << L"LookBackMaxSeconds=90\r\n"
      << L"HeadBobAmount=0.075\r\n"
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
      << L"InvertMouseY=0\r\n\r\n"
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
      << L"WaterDamageDensity=1\r\n"
      << L"MetalCabinetDensity=0.85\r\n"
      << L"JumpscareFrequency=0.3\r\n"
      << L"SparkParticles=1\r\n"
      << L"SparkEmitterRatio=0.11\r\n"
      << L"SparkBurstMinSeconds=2.8\r\n"
      << L"SparkBurstMaxSeconds=8.8\r\n"
      << L"SparkMaxParticles=160\r\n"
      << L"SparkSize=1\r\n"
      << L"AirParticles=1\r\n"
      << L"AirParticleDensity=1\r\n"
      << L"AirParticleSize=1\r\n"
      << L"AirParticleBlur=1\r\n"
        << L"BloodSplatterDensity=0.05\r\n"
      << L"BloodBurstCount=20\r\n"
      << L"BloodWetness=0.995\r\n"
      << L"BloodStreamCount=30\r\n"
      << L"BloodStreamThickness=0.88\r\n"
      << L"BloodShaderQuality=1\r\n"
      << L"BloodWorldFlicker=0\r\n"
      << L"BloodWorldAlwaysOn=0\r\n"
      << L"BloodWorldCoverage=1\r\n"
      << L"BloodWorldFlickerMinSeconds=42\r\n"
      << L"BloodWorldFlickerMaxSeconds=110\r\n"
      << L"BloodWorldFlickerDuration=1.15\r\n"
      << L"BloodWorldFlickerIntensity=1\r\n"
      << L"BloodStudyView=0\r\n"
      << L"FleshFlicker=1\r\n"
      << L"FleshAlwaysOn=0\r\n"
      << L"FleshWetness=0.995\r\n"
      << L"FleshParallaxScale=0.14\r\n"
      << L"FleshFlickerMinSeconds=34\r\n"
      << L"FleshFlickerMaxSeconds=92\r\n"
      << L"FleshFlickerDuration=0.75\r\n"
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
      << L"AirVentPanelDropChance=0.333333\r\n\r\n"
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
      << L"[Monster]\r\n"
      << L"MonsterScale=1\r\n"
      << L"MonsterSpeed=0.68\r\n"
      << L"MonsterSprintSpeed=0.88\r\n"
      << L"MonsterKillDistance=1.18\r\n"
      << L"MonsterVisibleDistance=12\r\n"
      << L"SkullMesh=assets\\White-Tailed Deer Skull.obj\r\n"
      << L"AlternateSkullMesh=assets\\models\\Ram_Skull\\Ram_Skull_Scan.OBJ\r\n"
      << L"AlternateSkullChance=0.35\r\n"
      << L"SkullMaxTriangles=18000\r\n"
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
        WriteTextFile(path, DefaultConfigText());
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
    s.mazeSeed = static_cast<uint32_t>(std::clamp(IniInt(L"Maze", L"RandomSeed", static_cast<int>(s.mazeSeed)), 0, std::numeric_limits<int>::max()));
    s.mapOverlay = IniInt(L"Maze", L"MapOverlay", s.mapOverlay ? 1 : 0) != 0;
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

    s.audioMuted = IniInt(L"Audio", L"Muted", s.audioMuted ? 1 : 0) != 0;
    s.audioMasterVolume = std::clamp(IniFloat(L"Audio", L"MasterVolume", s.audioMasterVolume), 0.0f, 1.0f);
    s.audioEffectsVolume = std::clamp(IniFloat(L"Audio", L"EffectsVolume", s.audioEffectsVolume), 0.0f, 1.0f);
    s.audioAmbienceVolume = std::clamp(IniFloat(L"Audio", L"AmbienceVolume", s.audioAmbienceVolume), 0.0f, 1.0f);
    s.audioMonsterVolume = std::clamp(IniFloat(L"Audio", L"MonsterVolume", s.audioMonsterVolume), 0.0f, 1.0f);

    s.paperDensity = std::clamp(IniFloat(L"Atmosphere", L"PaperDensity", s.paperDensity), 0.0f, 4.0f);
    s.hallwayPaperRunDensity = std::clamp(IniFloat(L"Atmosphere", L"HallwayPaperRunDensity", s.hallwayPaperRunDensity), 0.0f, 4.0f);
    s.chairDensity = std::clamp(IniFloat(L"Atmosphere", L"ChairDensity", s.chairDensity), 0.0f, 4.0f);
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
    s.bloodBurstCount = std::clamp(IniInt(L"Atmosphere", L"BloodBurstCount", s.bloodBurstCount), 0, 160);
    s.bloodWetness = std::clamp(IniFloat(L"Atmosphere", L"BloodWetness", s.bloodWetness), 0.0f, 3.0f);
    s.bloodStreamCount = std::clamp(IniInt(L"Atmosphere", L"BloodStreamCount", s.bloodStreamCount), 4, 32);
    s.bloodStreamThickness = std::clamp(IniFloat(L"Atmosphere", L"BloodStreamThickness", s.bloodStreamThickness), 0.10f, 2.0f);
    s.bloodShaderQuality = std::clamp(IniFloat(L"Atmosphere", L"BloodShaderQuality", s.bloodShaderQuality), 0.25f, 1.0f);
    s.bloodWorldFlicker = IniInt(L"Atmosphere", L"BloodWorldFlicker", s.bloodWorldFlicker ? 1 : 0) != 0;
    s.bloodWorldAlwaysOn = IniInt(L"Atmosphere", L"BloodWorldAlwaysOn", s.bloodWorldAlwaysOn ? 1 : 0) != 0;
    s.bloodWorldCoverage = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldCoverage", s.bloodWorldCoverage), 0.0f, 1.0f);
    s.bloodWorldFlickerMinSeconds = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerMinSeconds", s.bloodWorldFlickerMinSeconds), 3.0f, 600.0f);
    s.bloodWorldFlickerMaxSeconds = std::max(s.bloodWorldFlickerMinSeconds, std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerMaxSeconds", s.bloodWorldFlickerMaxSeconds), 3.0f, 900.0f));
    s.bloodWorldFlickerDuration = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerDuration", s.bloodWorldFlickerDuration), 0.15f, 8.0f);
    s.bloodWorldFlickerIntensity = std::clamp(IniFloat(L"Atmosphere", L"BloodWorldFlickerIntensity", s.bloodWorldFlickerIntensity), 0.0f, 2.0f);
    s.bloodStudyView = IniInt(L"Atmosphere", L"BloodStudyView", s.bloodStudyView ? 1 : 0) != 0;
    s.fleshFlicker = IniInt(L"Atmosphere", L"FleshFlicker", s.fleshFlicker ? 1 : 0) != 0;
    s.fleshAlwaysOn = IniInt(L"Atmosphere", L"FleshAlwaysOn", s.fleshAlwaysOn ? 1 : 0) != 0;
    s.fleshWetness = std::clamp(IniFloat(L"Atmosphere", L"FleshWetness", s.fleshWetness), 0.0f, 4.0f);
    s.fleshParallaxScale = std::clamp(IniFloat(L"Atmosphere", L"FleshParallaxScale", s.fleshParallaxScale), 0.0f, 0.32f);
    s.fleshFlickerMinSeconds = std::clamp(IniFloat(L"Atmosphere", L"FleshFlickerMinSeconds", s.fleshFlickerMinSeconds), 3.0f, 600.0f);
    s.fleshFlickerMaxSeconds = std::max(s.fleshFlickerMinSeconds, std::clamp(IniFloat(L"Atmosphere", L"FleshFlickerMaxSeconds", s.fleshFlickerMaxSeconds), 3.0f, 900.0f));
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
    s.monsterKillDistance = std::clamp(IniFloat(L"Monster", L"MonsterKillDistance", s.monsterKillDistance), 0.2f, 4.0f);
    s.monsterVisibleDistance = std::clamp(IniFloat(L"Monster", L"MonsterVisibleDistance", s.monsterVisibleDistance), 1.0f, 60.0f);
    s.monsterSkullMesh = IniString(L"Monster", L"SkullMesh", s.monsterSkullMesh.c_str());
    s.monsterAltSkullMesh = IniString(L"Monster", L"AlternateSkullMesh", s.monsterAltSkullMesh.c_str());
    s.monsterAltSkullChance = std::clamp(IniFloat(L"Monster", L"AlternateSkullChance", s.monsterAltSkullChance), 0.0f, 1.0f);
    s.monsterSkullMaxTriangles = std::clamp(IniInt(L"Monster", L"SkullMaxTriangles", s.monsterSkullMaxTriangles), 0, 60000);
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

struct Maze {
    int w = kMazeW;
    int h = kMazeH;
    float tileW = kTile;
    float tileD = kTile;
    std::vector<uint8_t> open;
    Tile start{1, 1};
    Tile exit{kMazeW - 2, kMazeH - 2};
    std::mt19937 rng{0xBACC2026u};

    bool InBounds(int x, int y) const {
        return x >= 0 && y >= 0 && x < w && y < h;
    }

    bool IsOpen(int x, int y) const {
        return InBounds(x, y) && open[static_cast<size_t>(y * w + x)] != 0;
    }

    void SetOpen(int x, int y, bool v = true) {
        if (InBounds(x, y)) open[static_cast<size_t>(y * w + x)] = v ? 1 : 0;
    }

    XMFLOAT3 WorldCenter(Tile t, float y = 0.0f) const {
        float ox = -static_cast<float>(w) * tileW * 0.5f;
        float oz = -static_cast<float>(h) * tileD * 0.5f;
        return {ox + (static_cast<float>(t.x) + 0.5f) * tileW, y, oz + (static_cast<float>(t.y) + 0.5f) * tileD};
    }

    Tile TileFromWorld(float x, float z) const {
        float ox = -static_cast<float>(w) * tileW * 0.5f;
        float oz = -static_cast<float>(h) * tileD * 0.5f;
        return {
            static_cast<int>(std::floor((x - ox) / tileW)),
            static_cast<int>(std::floor((z - oz) / tileD))
        };
    }

    float TileAverage() const {
        return (tileW + tileD) * 0.5f;
    }

    float TileMinimum() const {
        return std::min(tileW, tileD);
    }

    std::vector<Tile> Neighbors(Tile t) const {
        std::vector<Tile> out;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            Tile n{t.x + d[0], t.y + d[1]};
            if (IsOpen(n.x, n.y)) out.push_back(n);
        }
        return out;
    }

    int OpenNeighborCount(Tile t) const {
        int count = 0;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (auto& d : dirs) {
            if (IsOpen(t.x + d[0], t.y + d[1])) ++count;
        }
        return count;
    }

    int LocalOpenCount(Tile t, int radius = 2) const {
        int count = 0;
        for (int y = t.y - radius; y <= t.y + radius; ++y) {
            for (int x = t.x - radius; x <= t.x + radius; ++x) {
                if (IsOpen(x, y)) ++count;
            }
        }
        return count;
    }

    void Generate(const Settings& settings) {
        open.assign(static_cast<size_t>(w * h), 0);
        start = {1, 1};
        std::vector<Tile> stack;
        stack.push_back(start);
        SetOpen(start.x, start.y);
        const std::array<Tile, 4> dirs = {{{2, 0}, {-2, 0}, {0, 2}, {0, -2}}};

        while (!stack.empty()) {
            Tile cur = stack.back();
            std::array<Tile, 4> shuffled = dirs;
            std::shuffle(shuffled.begin(), shuffled.end(), rng);
            bool carved = false;
            for (Tile d : shuffled) {
                Tile nxt{cur.x + d.x, cur.y + d.y};
                if (nxt.x <= 0 || nxt.y <= 0 || nxt.x >= w - 1 || nxt.y >= h - 1 || IsOpen(nxt.x, nxt.y)) {
                    continue;
                }
                SetOpen(cur.x + d.x / 2, cur.y + d.y / 2);
                SetOpen(nxt.x, nxt.y);
                stack.push_back(nxt);
                carved = true;
                break;
            }
            if (!carved) stack.pop_back();
        }

        int margin = std::max(3, settings.roomMaxRadius + 1);
        std::uniform_int_distribution<int> pos(margin, std::max(margin, w - margin - 1));
        for (int i = 0; i < settings.roomCount; ++i) {
            int cx = pos(rng) | 1;
            int cy = pos(rng) | 1;
            int span = std::max(1, settings.roomMaxRadius - settings.roomMinRadius + 1);
            int rw = settings.roomMinRadius + static_cast<int>(rng() % span);
            int rh = settings.roomMinRadius + static_cast<int>(rng() % span);
            for (int y = cy - rh; y <= cy + rh; ++y) {
                for (int x = cx - rw; x <= cx + rw; ++x) {
                    if (x > 0 && y > 0 && x < w - 1 && y < h - 1) SetOpen(x, y);
                }
            }
            for (int door = 0; door < 4; ++door) {
                int dx = door < 2 ? (door == 0 ? -rw - 1 : rw + 1) : 0;
                int dy = door >= 2 ? (door == 2 ? -rh - 1 : rh + 1) : 0;
                int px = std::clamp(cx + dx, 1, w - 2);
                int py = std::clamp(cy + dy, 1, h - 2);
                SetOpen(px, py);
            }
        }

        exit = FarthestPerimeterReachable(start);
    }

    void GenerateBloodDebugCorridor() {
        w = std::max(9, w);
        h = std::max(5, h);
        open.assign(static_cast<size_t>(w * h), 0);
        int row = std::clamp(h / 2, 1, h - 2);
        for (int x = 1; x < w - 1; ++x) {
            SetOpen(x, row);
        }
        start = {1, row};
        exit = {w - 2, row};
    }

    void GenerateDebugSlice(int tiles) {
        tiles = std::clamp(tiles, 1, 5);
        w = tiles + 2;
        h = tiles + 2;
        open.assign(static_cast<size_t>(w * h), 0);
        for (int y = 1; y <= tiles; ++y) {
            for (int x = 1; x <= tiles; ++x) {
                SetOpen(x, y);
            }
        }
        int mid = 1 + tiles / 2;
        start = {mid, tiles};
        exit = {mid, 1};
    }

    Tile FarthestReachable(Tile from) const {
        std::vector<int> dist = ReachableDistances(from);
        Tile best = from;
        auto idx = [this](Tile t) { return static_cast<size_t>(t.y * w + t.x); };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                Tile t{x, y};
                if (dist[idx(t)] > dist[idx(best)]) best = t;
            }
        }
        return best;
    }

    Tile FarthestPerimeterReachable(Tile from) const {
        std::vector<int> dist = ReachableDistances(from);
        Tile best = from;
        int bestDist = -1;
        auto idx = [this](Tile t) { return static_cast<size_t>(t.y * w + t.x); };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (!IsOpen(x, y)) continue;
                bool perimeterPortal = (x == 1) || (x == w - 2) || (y == 1) || (y == h - 2);
                if (!perimeterPortal) continue;
                int d = dist[idx({x, y})];
                if (d > bestDist) {
                    bestDist = d;
                    best = {x, y};
                }
            }
        }
        return bestDist >= 0 ? best : FarthestReachable(from);
    }

    std::vector<int> ReachableDistances(Tile from) const {
        std::vector<int> dist(static_cast<size_t>(w * h), -1);
        std::queue<Tile> q;
        q.push(from);
        dist[static_cast<size_t>(from.y * w + from.x)] = 0;
        while (!q.empty()) {
            Tile t = q.front();
            q.pop();
            int base = dist[static_cast<size_t>(t.y * w + t.x)];
            for (Tile n : Neighbors(t)) {
                auto idx = static_cast<size_t>(n.y * w + n.x);
                if (dist[idx] >= 0) continue;
                dist[idx] = base + 1;
                q.push(n);
            }
        }
        return dist;
    }

    bool LineClear(Tile a, Tile b) const {
        XMFLOAT3 aw = WorldCenter(a, 1.5f);
        XMFLOAT3 bw = WorldCenter(b, 1.5f);
        float dx = bw.x - aw.x;
        float dz = bw.z - aw.z;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(2, static_cast<int>(len / (TileMinimum() * 0.18f)));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            Tile sample = TileFromWorld(aw.x + dx * t, aw.z + dz * t);
            if (!IsOpen(sample.x, sample.y)) return false;
        }
        return true;
    }

    std::vector<Tile> Path(Tile from, Tile to) const {
        if (!IsOpen(from.x, from.y) || !IsOpen(to.x, to.y)) return {};
        const int count = w * h;
        std::vector<float> cost(static_cast<size_t>(count), std::numeric_limits<float>::infinity());
        std::vector<int> parent(static_cast<size_t>(count), -1);
        auto idx = [this](Tile t) { return t.y * w + t.x; };
        auto heuristic = [](Tile a, Tile b) {
            return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
        };
        struct Node {
            Tile t;
            float f;
            bool operator<(const Node& other) const { return f > other.f; }
        };
        std::priority_queue<Node> pq;
        cost[static_cast<size_t>(idx(from))] = 0.0f;
        pq.push({from, heuristic(from, to)});
        while (!pq.empty()) {
            Tile cur = pq.top().t;
            pq.pop();
            if (cur == to) break;
            for (Tile n : Neighbors(cur)) {
                int ni = idx(n);
                float next = cost[static_cast<size_t>(idx(cur))] + 1.0f;
                if (next >= cost[static_cast<size_t>(ni)]) continue;
                cost[static_cast<size_t>(ni)] = next;
                parent[static_cast<size_t>(ni)] = idx(cur);
                pq.push({n, next + heuristic(n, to)});
            }
        }
        if (!std::isfinite(cost[static_cast<size_t>(idx(to))])) return {};
        std::vector<Tile> out;
        int at = idx(to);
        while (at != -1) {
            out.push_back({at % w, at / w});
            if (at == idx(from)) break;
            at = parent[static_cast<size_t>(at)];
        }
        std::reverse(out.begin(), out.end());
        return out;
    }
};

enum class MonsterPreviewView {
    Orbit,
    Front,
    Side,
    LeftSide,
    Top
};

class Renderer {
public:
    bool Initialize(HWND hwnd, const Settings* settingsOverride = nullptr, bool monsterPreview = false,
                    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit,
                    const StartupProgressSink* startupProgress = nullptr) {
        struct StartupProgressScope {
            Renderer* renderer = nullptr;
            ~StartupProgressScope() {
                if (renderer) renderer->startupProgress_ = nullptr;
            }
        } progressScope{this};
        startupProgress_ = startupProgress;
        startupProgressStep_ = 0;
        startupProgressTotal_ = kStartupProgressPreShaderSteps + 9 + kStartupProgressPostShaderSteps;
        startupShaderDone_ = 0;
        startupShaderTotal_ = 0;
        startupShaderCompiled_ = 0;
        startupShaderCached_ = 0;
        ReportStartupActivity(L"Starting renderer", L"Reading settings and preparing Direct3D.");

        StartupProfile profile(L"Initialize");
        settings_ = settingsOverride ? *settingsOverride : LoadSettings();
        monsterPreview_ = monsterPreview;
        monsterPreviewView_ = monsterPreviewView;
        runtimeMode_ = monsterPreview_ ? RendererRuntimeMode::Preview :
            (gEffectDebugViewer ? RendererRuntimeMode::DebugViewer : runtimeMode_);
        profile.Mark(L"LoadSettings");
        ReportStartupStep(L"Settings loaded", L"Creating Direct3D device.");
        hwnd_ = hwnd;
        RECT rc{};
        GetClientRect(hwnd_, &rc);
        width_ = std::max(1L, rc.right - rc.left);
        height_ = std::max(1L, rc.bottom - rc.top);

        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 2;
        scd.BufferDesc.Width = static_cast<UINT>(width_);
        scd.BufferDesc.Height = static_cast<UINT>(height_);
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hwnd_;
        scd.SampleDesc.Count = 1;
        scd.Windowed = TRUE;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0
        };
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
            levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
            &scd, &swapChain_, &device_, &featureLevel_, &context_);
        if (FAILED(hr) && settings_.allowWarpFallback) {
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                &scd, &swapChain_, &device_, &featureLevel_, &context_);
        }
        if (FAILED(hr)) return false;
        profile.Mark(L"CreateDeviceAndSwapChain");
        startupShaderTotal_ = featureLevel_ >= D3D_FEATURE_LEVEL_11_0 ? 9 : 7;
        startupProgressTotal_ = kStartupProgressPreShaderSteps + startupShaderTotal_ + kStartupProgressPostShaderSteps;
        ReportStartupStep(L"Direct3D device ready", L"Creating render targets.");

        if (!CreateBackBuffer()) return false;
        profile.Mark(L"CreateBackBuffer");
        ReportStartupStep(L"Back buffer ready", L"Checking shader cache.");
        ReportStartupActivity(L"Loading shaders", ShaderProgressDetail(L"Checking shader cache", nullptr, nullptr, false));
        if (!CreateShaders()) return false;
        profile.Mark(L"CreateShaders");
        ReportStartupActivity(L"Shaders ready", L"Creating render states.");
        if (!CreateStates()) return false;
        profile.Mark(L"CreateStates");
        ReportStartupStep(L"Render states ready", L"Allocating shadow map.");
        if (!CreateShadowResources()) return false;
        profile.Mark(L"CreateShadowResources");
        ReportStartupStep(L"Shadow resources ready", L"Building material textures.");
        ReportStartupActivity(L"Loading textures", L"Checking texture cache.");
        if (!CreateTextures()) return false;
        profile.Mark(L"CreateTextures");
        ReportStartupStep(L"Textures ready", L"Loading flashlight pattern.");
        if (!CreateFlashlightPatternTexture()) return false;
        profile.Mark(L"CreateFlashlightPatternTexture");
        ReportStartupStep(L"Flashlight pattern ready", L"Creating constant buffers.");
        if (!CreateConstantBuffer()) return false;
        profile.Mark(L"CreateConstantBuffer");
        ReportStartupStep(L"GPU buffers ready", L"Loading monster mesh.");

        runtimeSeed_ = ResolveRuntimeSeed(settings_.mazeSeed);
        ApplyRuntimeVariation(settings_, runtimeSeed_);
        gameplaySettings_ = settings_;
        if (gEffectDebugViewer) ApplyDebugSliceSettings();
        LoadMonsterSkullMesh();
        profile.Mark(L"LoadMonsterSkullMesh");
        ReportStartupStep(L"Monster mesh ready", L"Loading prop meshes.");
        LoadPropMeshes();
        profile.Mark(L"LoadPropMeshes");
        ReportStartupStep(L"Prop meshes ready", L"Generating maze layout.");
        maze_.rng.seed(runtimeSeed_);
        rng_.seed(runtimeSeed_ ^ 0x9e3779b9u);

        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.exit = {maze_.w - 2, maze_.h - 2};
        if (gEffectDebugViewer) {
            maze_.GenerateDebugSlice(gDebugSliceTiles);
        } else if (gBloodDebugEveryWall) {
            maze_.GenerateBloodDebugCorridor();
        } else {
            maze_.Generate(settings_);
        }
        profile.Mark(L"GenerateMaze");
        ReportStartupStep(L"Maze generated", L"Uploading maze mask.");
        if (!CreateMazeMaskTexture()) return false;
        profile.Mark(L"CreateMazeMaskTexture");
        ReportStartupStep(L"Maze mask ready", L"Building maze geometry.");
        ResetSimulation();
        CreateMazeMesh();
        ResetDebugSliceLoopState();
        profile.Mark(L"CreateMazeMesh");
        ReportStartupStep(L"Ready", L"Entering maze.");
        lastTicks_ = GetTickCount64();
        bloodDebugStartTicks_ = lastTicks_;
        return true;
    }

    void Resize(int w, int h) {
        if (!device_ || w <= 0 || h <= 0) return;
        width_ = w;
        height_ = h;
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        rtv_.Reset();
        dsv_.Reset();
        depth_.Reset();
        sceneColorSrv_.Reset();
        sceneColorRtv_.Reset();
        sceneColor_.Reset();
        HRESULT hr = swapChain_->ResizeBuffers(0, static_cast<UINT>(w), static_cast<UINT>(h), DXGI_FORMAT_UNKNOWN, 0);
        if (SUCCEEDED(hr)) CreateBackBuffer();
    }

    void Tick() {
        ULONGLONG now = GetTickCount64();
        float dt = std::min(0.05f, static_cast<float>(now - lastTicks_) / 1000.0f);
        lastTicks_ = now;
        TickFrame(dt);
    }

    void TickFixed(float dt) {
        lastTicks_ = GetTickCount64();
        TickFrame(std::clamp(dt, 0.0f, 0.05f));
    }

    void TickFrame(float dt) {
        time_ += dt;
        UpdateAirParticlePerformanceBudget(dt);
        UpdateSimulation(dt);
        Render();
    }

    void SetPresentSyncInterval(UINT syncInterval) {
        presentSyncInterval_ = syncInterval;
    }

    void SetPresentFlags(UINT flags) {
        presentFlags_ = flags;
    }

    void SetPresentEnabled(bool enabled) {
        presentEnabled_ = enabled;
    }

    bool LastPresentCompleted() const {
        return lastPresentCompleted_;
    }

    void SetRuntimeMode(RendererRuntimeMode mode) {
        runtimeMode_ = mode;
    }

    RendererRuntimeMode RuntimeMode() const {
        return runtimeMode_;
    }

    void SetGameInput(const GameInputSnapshot& input) {
        gameInput_ = input;
    }

    void ApplyGameSettings(const Settings& settings) {
        auto applyLive = [&](Settings& target) {
            target.gameFullscreen = settings.gameFullscreen;
            target.gameResolutionWidth = settings.gameResolutionWidth;
            target.gameResolutionHeight = settings.gameResolutionHeight;
            target.allowWarpFallback = settings.allowWarpFallback;
            target.exposure = settings.exposure;
            target.bloomAmount = settings.bloomAmount;
            target.motionBlurAmount = settings.motionBlurAmount;
            target.airParticleDensity = settings.airParticleDensity;
            target.mouseSensitivity = settings.mouseSensitivity;
            target.invertMouseY = settings.invertMouseY;
            target.audioMuted = settings.audioMuted;
            target.audioMasterVolume = settings.audioMasterVolume;
            target.audioEffectsVolume = settings.audioEffectsVolume;
            target.audioAmbienceVolume = settings.audioAmbienceVolume;
            target.audioMonsterVolume = settings.audioMonsterVolume;
        };
        applyLive(gameplaySettings_);
        applyLive(settings_);
    }

    void RestartGameRun() {
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        runtimeMode_ = RendererRuntimeMode::PlayableGame;
        settings_ = gameplaySettings_;
        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.exit = {maze_.w - 2, maze_.h - 2};
        RestartMaze();
    }

    void EnterDebugViewer(DebugSliceEffect effect = DebugSliceEffect::Blood, int tiles = 3) {
        runtimeMode_ = RendererRuntimeMode::DebugViewer;
        gEffectDebugViewer = true;
        gBloodDebugEveryWall = effect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(effect);
        ConfigureDebugSlice(effect, tiles);
    }

    void SetMonsterPreviewOrbit(float yaw, float pitch, float distance) {
        if (!monsterPreview_) return;
        monsterPreviewManualOrbit_ = true;
        monsterPreviewOrbitYaw_ = yaw;
        monsterPreviewOrbitPitch_ = std::clamp(pitch, -1.15f, 0.75f);
        monsterPreviewOrbitDistance_ = std::clamp(distance, 1.25f, 8.0f);
        SetMonsterPreviewCamera(time_);
    }

    void SetMonsterPreviewEyeCalibration(const Settings& settings) {
        if (!monsterPreview_) return;
        settings_.monsterRightEyeX = settings.monsterRightEyeX;
        settings_.monsterRightEyeY = settings.monsterRightEyeY;
        settings_.monsterRightEyeZ = settings.monsterRightEyeZ;
        settings_.monsterLeftEyeX = settings.monsterLeftEyeX;
        settings_.monsterLeftEyeY = settings.monsterLeftEyeY;
        settings_.monsterLeftEyeZ = settings.monsterLeftEyeZ;
        settings_.monsterAltRightEyeX = settings.monsterAltRightEyeX;
        settings_.monsterAltRightEyeY = settings.monsterAltRightEyeY;
        settings_.monsterAltRightEyeZ = settings.monsterAltRightEyeZ;
        settings_.monsterAltLeftEyeX = settings.monsterAltLeftEyeX;
        settings_.monsterAltLeftEyeY = settings.monsterAltLeftEyeY;
        settings_.monsterAltLeftEyeZ = settings.monsterAltLeftEyeZ;
        settings_.monsterSkullYawDegrees = settings.monsterSkullYawDegrees;
        settings_.monsterSkullPitchDegrees = settings.monsterSkullPitchDegrees;
        settings_.monsterSkullRollDegrees = settings.monsterSkullRollDegrees;
        settings_.monsterAltSkullYawDegrees = settings.monsterAltSkullYawDegrees;
        settings_.monsterAltSkullPitchDegrees = settings.monsterAltSkullPitchDegrees;
        settings_.monsterAltSkullRollDegrees = settings.monsterAltSkullRollDegrees;
        SetMonsterPreviewCamera(time_);
    }

    void ConfigureDebugSlice(DebugSliceEffect effect, int tiles) {
        if (!gEffectDebugViewer) return;
        gDebugSliceEffect = effect;
        gDebugSliceTiles = std::clamp(tiles, gDebugSliceEffect == DebugSliceEffect::Props ? 3 : 1, 5);
        ApplyDebugSliceSettings();
        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.exit = {maze_.w - 2, maze_.h - 2};
        maze_.GenerateDebugSlice(gDebugSliceTiles);
        CreateMazeMaskTexture();
        ResetSimulation();
        CreateMazeMesh();
        ResetDebugSliceLoopState();
        bloodDebugStartTicks_ = GetTickCount64();
        lastTicks_ = bloodDebugStartTicks_;
        InvalidateRect(hwnd_, nullptr, FALSE);
    }

    void ResetDebugSliceAnimation() {
        if (!gEffectDebugViewer) return;
        ResetDebugSliceLoopState();
        bloodDebugStartTicks_ = GetTickCount64();
        lastTicks_ = bloodDebugStartTicks_;
        InvalidateRect(hwnd_, nullptr, FALSE);
    }

private:
    static constexpr int kStartupProgressPreShaderSteps = 3;
    static constexpr int kStartupProgressPostShaderSteps = 10;

    struct StaticPropMesh {
        std::vector<Vertex> vertices;
        XMFLOAT3 min{};
        XMFLOAT3 max{};
        bool generatedUvFallback = false;
    };

    static bool StaticPropNeedsGeneratedUv(const StaticPropMesh& mesh) {
        if (mesh.vertices.size() < 3) return false;
        XMFLOAT2 minUv{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT2 maxUv{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        for (const Vertex& v : mesh.vertices) {
            if (!std::isfinite(v.uv.x) || !std::isfinite(v.uv.y)) return true;
            minUv.x = std::min(minUv.x, v.uv.x);
            minUv.y = std::min(minUv.y, v.uv.y);
            maxUv.x = std::max(maxUv.x, v.uv.x);
            maxUv.y = std::max(maxUv.y, v.uv.y);
        }
        return (maxUv.x - minUv.x) < 0.035f || (maxUv.y - minUv.y) < 0.035f;
    }

    static std::wstring WidenAscii(const char* text) {
        if (!text) return {};
        return std::wstring(text, text + std::strlen(text));
    }

    void ReportStartupActivity(const wchar_t* phase, const std::wstring& detail = L"") {
        if (!startupProgress_ || !startupProgress_->callback) return;
        StartupProgressUpdate update{};
        update.phase = phase;
        update.detail = detail.c_str();
        update.current = startupProgressStep_;
        update.total = std::max(1, startupProgressTotal_);
        update.shaderDone = startupShaderDone_;
        update.shaderTotal = startupShaderTotal_;
        update.shaderCompiled = startupShaderCompiled_;
        update.shaderCached = startupShaderCached_;
        startupProgress_->Report(update);
    }

    void ReportStartupStep(const wchar_t* phase, const std::wstring& detail = L"") {
        startupProgressStep_ = std::min(startupProgressStep_ + 1, std::max(1, startupProgressTotal_));
        ReportStartupActivity(phase, detail);
    }

    std::wstring ShaderProgressDetail(const wchar_t* action, const char* entry, const char* profile, bool completed) const {
        std::wostringstream detail;
        if (startupShaderTotal_ > 0) {
            int shown = startupShaderDone_ + (completed ? 0 : 1);
            shown = std::clamp(shown, 0, startupShaderTotal_);
            detail << L"Shader " << shown << L"/" << startupShaderTotal_;
        } else {
            detail << L"Shaders";
        }
        if (action && *action) detail << L": " << action;
        if (entry && *entry) detail << L" " << WidenAscii(entry);
        if (profile && *profile) detail << L" (" << WidenAscii(profile) << L")";
        if (startupShaderCompiled_ > 0 || startupShaderCached_ > 0) {
            detail << L" - compiled " << startupShaderCompiled_ << L", cached " << startupShaderCached_;
        }
        return detail.str();
    }

    void ReportShaderActivity(const wchar_t* action, const char* entry, const char* profile) {
        ReportStartupActivity(L"Loading shaders", ShaderProgressDetail(action, entry, profile, false));
    }

    void ReportShaderComplete(const wchar_t* action, const char* entry, const char* profile, bool compiled) {
        ++startupShaderDone_;
        if (compiled) {
            ++startupShaderCompiled_;
        } else {
            ++startupShaderCached_;
        }
        ReportStartupStep(L"Loading shaders", ShaderProgressDetail(action, entry, profile, true));
    }

    void ApplyDebugSliceSettings() {
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
        if (gDebugSliceEffect == DebugSliceEffect::Props) {
            gDebugSliceTiles = std::max(gDebugSliceTiles, 3);
        }
        bool liquidDebug = gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect);
        gBloodDebugEveryWall = gEffectDebugViewer && liquidDebug;

        settings_.mazeWidth = gDebugSliceTiles + 2;
        settings_.mazeHeight = gDebugSliceTiles + 2;
        settings_.roomCount = 0;
        settings_.roomCountRange = 0;
        settings_.roomMinRadiusRange = 0;
        settings_.roomMaxRadiusRange = 0;
        settings_.runVariation = 0.0f;
        settings_.mapOverlay = false;
        settings_.ambientLight = kEffectDebugAmbientLight;
        settings_.exposure = std::min(settings_.exposure, kEffectDebugExposureMax);
        settings_.bloomAmount = std::min(settings_.bloomAmount, kEffectDebugBloomMax);
        settings_.fogStartMeters = 1000.0f;
        settings_.fogEndMeters = 1100.0f;
        settings_.fogDarkness = 0.0f;
        settings_.flashlightIntensity = kEffectDebugFlashlightIntensity;
        settings_.flashlightShadows = false;
        settings_.paperDensity = 0.0f;
        settings_.hallwayPaperRunDensity = 0.0f;
        settings_.chairDensity = 0.0f;
        settings_.metalCabinetDensity = 0.0f;
        settings_.jumpscareFrequency = 0.0f;
        settings_.airParticles = gDebugSliceEffect == DebugSliceEffect::AirVents;
        settings_.sparkParticles = gDebugSliceEffect == DebugSliceEffect::BrokenLamps;
        settings_.bloodStudyView = false;
        settings_.bloodWorldFlicker = false;
        settings_.bloodWorldAlwaysOn = false;
        settings_.bloodWorldCoverage = 0.0f;
        settings_.fleshFlicker = false;

        settings_.waterDamageDensity = 0.0f;
        settings_.bloodSplatterDensity = 0.0f;
        settings_.bloodBurstCount = 0;
        settings_.bloodStreamCount = liquidDebug
            ? std::max(settings_.bloodStreamCount, 30)
            : 0;
        settings_.bloodStreamThickness = std::max(settings_.bloodStreamThickness, 0.88f);
        settings_.bloodShaderQuality = 1.0f;

        settings_.lampSpacing = settings_.tileWidthMeters;
        settings_.lampIntensity = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? kEffectDebugLampIntensity : 0.0f;
        settings_.lampFlickerRatio = 0.0f;
        if (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) {
            settings_.lampOnRatio = 1.0f;
            settings_.darkLampVisibleRatio = 1.0f;
            settings_.brokenZoneRatio = 0.0f;
        } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            settings_.lampOnRatio = 0.0f;
            settings_.darkLampVisibleRatio = 0.0f;
            settings_.brokenZoneRatio = 1.0f;
            settings_.sparkEmitterRatio = 1.0f;
        } else {
            settings_.lampOnRatio = 0.0f;
            settings_.darkLampVisibleRatio = 0.0f;
            settings_.brokenZoneRatio = 0.0f;
        }
        if (gDebugSliceEffect == DebugSliceEffect::Props) {
            settings_.ambientLight = std::max(settings_.ambientLight, 0.42f);
            settings_.flashlightIntensity = std::max(settings_.flashlightIntensity, 2.4f);
        }
    }

    float DebugSliceClockSeconds() const {
        if (!gEffectDebugViewer || bloodDebugStartTicks_ == 0) return time_;
        return static_cast<float>(GetTickCount64() - bloodDebugStartTicks_) / 1000.0f;
    }

    float DebugSliceLoopSeconds() const {
        if (gDebugSliceEffect == DebugSliceEffect::Blood) return settings_.effectBloodLoopSeconds;
        if (DebugSliceEffectIsWater(gDebugSliceEffect)) return std::max(settings_.effectWaterLoopSeconds, settings_.effectBloodLoopSeconds * 0.82f);
        if (gDebugSliceEffect == DebugSliceEffect::AirVents) return settings_.effectAirVentLoopSeconds;
        if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) return settings_.effectBrokenLampLoopSeconds;
        return settings_.effectStaticLoopSeconds;
    }

    float DebugSliceLoopPhase() const {
        float loopSeconds = std::max(0.1f, DebugSliceLoopSeconds());
        return std::fmod(std::max(0.0f, DebugSliceClockSeconds()), loopSeconds) / loopSeconds;
    }

    void ResetDebugSliceLoopState() {
        debugSliceLoopCycle_ = -1;
        sparks_.clear();
        sparkFlashes_.clear();
        sparkChains_.clear();
        steam_.clear();
        ventDrops_.clear();
        for (SparkEmitter& emitter : sparkEmitters_) {
            emitter.triggered = false;
        }
        for (SteamEmitter& emitter : steamEmitters_) {
            emitter.triggered = false;
            emitter.panelDropped = false;
        }
    }

    void UpdateDebugSliceLoop(float) {
        if (!gEffectDebugViewer) return;
        float loopSeconds = std::max(0.1f, DebugSliceLoopSeconds());
        int cycle = static_cast<int>(std::floor(DebugSliceClockSeconds() / loopSeconds));
        if (cycle == debugSliceLoopCycle_) return;
        debugSliceLoopCycle_ = cycle;

        if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
            sparks_.clear();
            sparkFlashes_.clear();
            sparkChains_.clear();
            for (SparkEmitter& emitter : sparkEmitters_) {
                emitter.triggered = false;
                float intensity = PickBrokenLampSparkIntensity();
                int chainBursts = PickBrokenLampChainBursts();
                SpawnSparkBurst(emitter, intensity);
                ScheduleSparkChain(emitter.pos, intensity * settings_.effectBrokenLampChainIntensityScale, chainBursts);
            }
        } else if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
            steam_.clear();
            sparks_.clear();
            sparkFlashes_.clear();
            sparkChains_.clear();
            int emitterIndex = 0;
            for (SteamEmitter& emitter : steamEmitters_) {
                emitter.triggered = false;
                SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                if (!emitter.panelDropped &&
                    ((emitterIndex + cycle) % std::max(1, settings_.effectAirVentPanelDropEvery)) == 0 &&
                    SpawnVentDrop(emitter)) {
                    emitter.panelDropped = true;
                }
                ++emitterIndex;
            }
        }
    }

    HWND hwnd_ = nullptr;
    LONG width_ = 1;
    LONG height_ = 1;
    float time_ = 0.0f;
    float effectAnimationStartTime_ = 0.0f;
    ULONGLONG lastTicks_ = 0;
    ULONGLONG bloodDebugStartTicks_ = 0;
    int debugSliceLoopCycle_ = -1;
    const StartupProgressSink* startupProgress_ = nullptr;
    int startupProgressStep_ = 0;
    int startupProgressTotal_ = 1;
    int startupShaderDone_ = 0;
    int startupShaderTotal_ = 0;
    int startupShaderCompiled_ = 0;
    int startupShaderCached_ = 0;

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGISwapChain> swapChain_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    ComPtr<ID3D11Texture2D> depth_;
    ComPtr<ID3D11DepthStencilView> dsv_;
    ComPtr<ID3D11Texture2D> sceneColor_;
    ComPtr<ID3D11RenderTargetView> sceneColorRtv_;
    ComPtr<ID3D11ShaderResourceView> sceneColorSrv_;
    ComPtr<ID3D11VertexShader> vertexShader_;
    ComPtr<ID3D11HullShader> hullShader_;
    ComPtr<ID3D11DomainShader> domainShader_;
    ComPtr<ID3D11PixelShader> pixelShader_;
    ComPtr<ID3D11VertexShader> overlayVertexShader_;
    ComPtr<ID3D11PixelShader> overlayPixelShader_;
    ComPtr<ID3D11VertexShader> postVertexShader_;
    ComPtr<ID3D11PixelShader> postPixelShader_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    ComPtr<ID3D11InputLayout> overlayInputLayout_;
    ComPtr<ID3D11Buffer> vertexBuffer_;
    ComPtr<ID3D11Buffer> indexBuffer_;
    ComPtr<ID3D11Buffer> monsterBuffer_;
    ComPtr<ID3D11Buffer> dynamicBuffer_;
    ComPtr<ID3D11Buffer> overlayBuffer_;
    ComPtr<ID3D11Buffer> constantBuffer_;
    ComPtr<ID3D11ShaderResourceView> albedoSrv_;
    ComPtr<ID3D11ShaderResourceView> normalSrv_;
    ComPtr<ID3D11ShaderResourceView> materialPropsSrv_;
    ComPtr<ID3D11ShaderResourceView> flashlightPatternSrv_;
    ComPtr<ID3D11ShaderResourceView> mazeSrv_;
    ComPtr<ID3D11Texture2D> lampDamageTexture_;
    ComPtr<ID3D11ShaderResourceView> lampDamageSrv_;
    ComPtr<ID3D11SamplerState> sampler_;
    ComPtr<ID3D11RasterizerState> rasterState_;
    ComPtr<ID3D11DepthStencilState> depthState_;
    ComPtr<ID3D11DepthStencilState> depthLessState_;
    ComPtr<ID3D11DepthStencilState> depthReadOnlyState_;
    ComPtr<ID3D11DepthStencilState> depthDisabledState_;
    ComPtr<ID3D11BlendState> alphaBlend_;
    ComPtr<ID3D11PixelShader> shadowPixelShader_;
    ComPtr<ID3D11Texture2D> shadowDepth_;
    ComPtr<ID3D11DepthStencilView> shadowDsv_;
    ComPtr<ID3D11ShaderResourceView> shadowSrv_;
    ComPtr<ID3D11SamplerState> shadowSampler_;
    ComPtr<ID3D11SamplerState> postSampler_;
    ComPtr<ID3D11RasterizerState> shadowRasterState_;
    D3D_FEATURE_LEVEL featureLevel_ = D3D_FEATURE_LEVEL_10_0;
    UINT shadowMapSize_ = 2048;
    UINT presentSyncInterval_ = 1;
    UINT presentFlags_ = 0;
    bool presentEnabled_ = true;
    bool lastPresentCompleted_ = true;
    UINT indexCount_ = 0;
    UINT floorCeilingStartIndex_ = 0;
    UINT floorCeilingIndexCount_ = 0;
    UINT staticWaterStartIndex_ = 0;
    UINT staticWaterIndexCount_ = 0;
    UINT staticTransparentStartIndex_ = 0;
    UINT staticTransparentIndexCount_ = 0;
    UINT staticPropShadowStartIndex_ = 0;
    UINT staticPropShadowIndexCount_ = 0;
    UINT dynamicOpaqueVertexCount_ = 0;
    UINT dynamicTransparentVertexCount_ = 0;
    UINT dynamicVertexCount_ = 0;
    uint32_t runtimeSeed_ = 1;
    std::vector<Vertex> skullMesh_;
    bool monsterUsingAltSkull_ = false;
    std::array<StaticPropMesh, 3> chairPropMeshes_;
    StaticPropMesh cabinetPropMesh_;
    StaticPropMesh deskPropMesh_;
    StaticPropMesh trashBinPropMesh_;
    StaticPropMesh deskLampPropMesh_;
    StaticPropMesh cassettePropMesh_;
    StaticPropMesh airVentPropMesh_;
    StaticPropMesh exitSignPropMesh_;
    std::array<StaticPropMesh, 4> ceilingLampPropMeshes_;
    RendererRuntimeMode runtimeMode_ = RendererRuntimeMode::ScreensaverAutopilot;
    GameInputSnapshot gameInput_{};
    Settings gameplaySettings_{};
    float playerHealth_ = 100.0f;
    float playerStamina_ = 100.0f;
    float playerVerticalOffset_ = 0.0f;
    float playerVerticalVelocity_ = 0.0f;
    float playerStaminaRegenDelay_ = 0.0f;
    bool playerGrounded_ = true;
    bool monsterPreview_ = false;
    MonsterPreviewView monsterPreviewView_ = MonsterPreviewView::Orbit;
    bool monsterPreviewManualOrbit_ = false;
    float monsterPreviewOrbitYaw_ = 0.0f;
    float monsterPreviewOrbitPitch_ = -0.18f;
    float monsterPreviewOrbitDistance_ = 3.15f;

    Settings settings_;
    Maze maze_;
    XMFLOAT3 camera_{};
    float yaw_ = 0.0f;
    float bodyYaw_ = 0.0f;
    float turnLookBlend_ = 0.0f;
    float turnLookYaw_ = 0.0f;
    float lookPitch_ = -0.055f;
    float manualLookYawDelta_ = 0.0f;
    float manualLookPitchDelta_ = 0.0f;
    float stepPhase_ = 0.0f;
    float headScanTimer_ = 0.0f;
    float headScanDuration_ = 0.0f;
    float headScanCenter_ = 0.0f;
    float stopTimer_ = 0.0f;
    float nextLookBackTime_ = 9.0f;
    bool lookBack_ = false;
    bool junctionScanActive_ = false;
    int junctionScanCount_ = 0;
    std::array<float, 4> junctionScanYaws_{};
    Tile junctionScanTile_{-1000, -1000};
    float branchLookTimer_ = 0.0f;
    float branchLookDuration_ = 0.0f;
    float branchLookYaw_ = 0.0f;
    float branchLookPitch_ = -0.045f;
    float branchLookCooldown_ = 0.0f;
    bool branchLookPaused_ = false;
    Tile lastBranchLookTile_{-1000, -1000};
    float roomSurveyTimer_ = 0.0f;
    float roomSurveyDuration_ = 0.0f;
    float roomSurveyCenter_ = 0.0f;
    float roomSurveySpan_ = 0.0f;
    float roomSurveyDirection_ = 1.0f;
    std::array<float, 6> roomSurveyYaws_{};
    std::array<float, 6> roomSurveyPitches_{};
    int roomSurveyYawCount_ = 0;
    int roomSurveyPitchCount_ = 0;
    float roomSurveyCooldown_ = 0.0f;
    Tile lastTile_{-1000, -1000};
    Tile previousTile_{-1000, -1000};
    std::vector<Tile> path_;
    size_t pathIndex_ = 0;
    std::mt19937 rng_{0x51515151u};
    std::vector<XMFLOAT3> propLookPoints_;
    std::vector<uint16_t> visitedTiles_;
    std::vector<AirParticle> airParticles_;
    std::vector<Vertex> dynamicOpaqueVerts_;
    std::vector<Vertex> dynamicTransparentVerts_;
    float airParticleBudgetScale_ = 1.0f;
    float airParticleFrameDt_ = 0.0f;
    std::vector<SparkEmitter> sparkEmitters_;
    std::vector<RuntimeLampState> runtimeLamps_;
    std::vector<uint8_t> lampDamagePixels_;
    std::vector<SteamEmitter> steamEmitters_;
    std::vector<SparkParticle> sparks_;
    std::vector<SparkFlash> sparkFlashes_;
    std::vector<SparkChain> sparkChains_;
    std::vector<SteamParticle> steam_;
    std::vector<VentDrop> ventDrops_;
    std::vector<BloodScarePoint> bloodScarePoints_;
    std::vector<BloodRevealRegion> bloodRevealRegions_;
    int activeBloodScareIndex_ = -1;
    float bloodScareActiveUntil_ = 0.0f;
    float bloodWorldFlickerCooldown_ = 22.0f;
    float bloodWorldFlickerTimer_ = 0.0f;
    float bloodWorldFlickerDuration_ = 1.0f;
    float bloodWorldActivationTime_ = -1000.0f;
    float bloodFocusTimer_ = 0.0f;
    float bloodFocusDuration_ = 0.0f;
    int bloodFocusReactionsTaken_ = 0;
    float bloodFocusReactionCooldown_ = 0.0f;
    float proximityBloodPulseCooldown_ = 0.0f;
    XMFLOAT3 bloodFocusTarget_{};
    Tile bloodStudyTile_{-1000, -1000};
    float sparkCooldown_ = 3.0f;
    float scareCooldown_ = 2.0f;
    float fleshFlickerCooldown_ = 18.0f;
    float fleshFlickerTimer_ = 0.0f;
    float fleshFlickerDuration_ = 1.0f;
    bool lampDamageDirty_ = false;
    Tile scareEventTile_{-1000, -1000};
    XMFLOAT3 propLookTarget_{};
    float propLookTimer_ = 0.0f;
    float propLookDuration_ = 0.0f;
    float propLookCooldown_ = 0.0f;
    float propLookScanSeed_ = 0.0f;
    XMFLOAT3 lastPropLookTarget_{};
    bool hasLastPropLookTarget_ = false;
    float secondsSinceLookBack_ = 0.0f;
    bool exitSpotted_ = false;
    float exitLookBlend_ = 0.0f;
    XMFLOAT3 exitLookFocus_{};
    float threatRepath_ = 0.0f;
    float dangerLevel_ = 0.0f;
    float dreadLevel_ = 0.0f;
    float dreadMeterLevel_ = 0.0f;
    float chaseMemoryTimer_ = 0.0f;
    float chasePanic_ = 0.0f;
    float monsterRunLaunchMeters_ = 3.0f;
    bool monsterRunLaunchActive_ = false;
    float smoothedMoveSpeed_ = 0.0f;
    float runIntensity_ = 0.0f;
    float runEffort_ = 0.0f;
    float breathPhase_ = 0.0f;
    bool threatVisibleLast_ = false;
    float panicFlashlightTimer_ = 0.0f;
    float panicFlashlightDuration_ = 0.0f;
    bool monsterSpottedLast_ = false;
    float monsterSightDreadCooldown_ = 0.0f;
    float chaseLookBackTimer_ = 0.0f;
    float chaseLookBackDuration_ = 0.0f;
    float chaseLookBackCooldown_ = 0.0f;
    float chaseLookBackYaw_ = 0.0f;
    float chaseLookBackPitch_ = -0.035f;
    float stumbleTimer_ = 0.0f;
    float stumbleDuration_ = 0.0f;
    float stumbleYawOffset_ = 0.0f;
    float flashlightYaw_ = 0.0f;
    float flashlightPitch_ = -0.055f;
    float previousCameraYaw_ = 0.0f;
    float previousCameraPitch_ = -0.055f;
    XMFLOAT2 cameraMotionBlur_{};
    float flashlightAgitation_ = 0.0f;
    float flashlightDartTimer_ = 0.0f;
    float flashlightDartDuration_ = 0.0f;
    float flashlightDartCooldown_ = 0.0f;
    float flashlightDartYaw_ = 0.0f;
    float flashlightDartPitch_ = 0.0f;
    float flashlightSnapTimer_ = 0.0f;
    float flashlightSnapDuration_ = 0.0f;
    float flashlightSnapCooldown_ = 0.0f;
    float flashlightSnapYaw_ = 0.0f;
    float flashlightSnapPitch_ = 0.0f;
    bool flashlightSnapSharp_ = false;
    float flashlightHoldYaw_ = 0.0f;
    float flashlightHoldPitch_ = 0.0f;
    float airFocusDistance_ = 3.5f;
    float ventReactionTimer_ = 0.0f;
    float ventReactionDuration_ = 0.0f;
    float ventReactionLookDelay_ = 0.0f;
    float ventReactionBackDuration_ = 0.0f;
    float ventReactionScanSeed_ = 0.0f;
    XMFLOAT3 ventReactionTarget_{};
    XMFLOAT3 ventReactionAway_{0.0f, 0.0f, 1.0f};
    bool exitTransitionActive_ = false;
    float exitTransitionTimer_ = 0.0f;
    float fadeInTimer_ = 1.2f;
    XMFLOAT3 exitStartCamera_{};
    float exitStartYaw_ = 0.0f;
    XMFLOAT3 exitDoorCenter_{};
    XMFLOAT3 exitDoorNormal_{0.0f, 0.0f, 1.0f};
    XMFLOAT3 exitDoorRight_{1.0f, 0.0f, 0.0f};
    XMFLOAT3 exitDoorHinge_{};
    XMFLOAT3 exitSignLightPos_{};
    float exitSignLightStrength_ = 0.0f;
    float exitDoorAngle_ = 0.0f;
    bool deathActive_ = false;
    float deathTimer_ = 0.0f;

    XMFLOAT3 monster_{};
    std::vector<Tile> monsterPath_;
    size_t monsterPathIndex_ = 0;
    float monsterRepath_ = 0.0f;
    float monsterYaw_ = 0.0f;
    Tile monsterGoal_{-1000, -1000};
    Tile monsterSoundTile_{-1000, -1000};
    Tile monsterLastKnownTile_{-1000, -1000};
    Tile monsterRoamTile_{-1000, -1000};
    bool monsterHasSound_ = false;
    bool monsterHasLastKnown_ = false;
    bool monsterChasingVisible_ = false;
    float monsterSearchTimer_ = 0.0f;
    float monsterRoamTimer_ = 0.0f;
    bool monsterRecognitionActive_ = false;
    bool monsterRecognizedForChase_ = false;
    float monsterRecognitionTimer_ = 0.0f;
    float monsterRecognitionDuration_ = 0.0f;
    float monsterHeadBobPhase_ = 0.0f;
    float monsterHeadScanPhase_ = 0.0f;
    float monsterHeadYawOffset_ = 0.0f;
    float monsterHeadPitchOffset_ = 0.0f;
    float monsterHeadLockAmount_ = 0.0f;
    float monsterHeadChaseBlend_ = 0.0f;
    bool monsterCanSeePlayerNow_ = false;

    bool CreateBackBuffer() {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return false;
        hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &rtv_);
        if (FAILED(hr)) return false;

        D3D11_TEXTURE2D_DESC sceneDesc{};
        sceneDesc.Width = static_cast<UINT>(width_);
        sceneDesc.Height = static_cast<UINT>(height_);
        sceneDesc.MipLevels = 1;
        sceneDesc.ArraySize = 1;
        sceneDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sceneDesc.SampleDesc.Count = 1;
        sceneDesc.Usage = D3D11_USAGE_DEFAULT;
        sceneDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        hr = device_->CreateTexture2D(&sceneDesc, nullptr, &sceneColor_);
        if (FAILED(hr)) return false;
        hr = device_->CreateRenderTargetView(sceneColor_.Get(), nullptr, &sceneColorRtv_);
        if (FAILED(hr)) return false;
        hr = device_->CreateShaderResourceView(sceneColor_.Get(), nullptr, &sceneColorSrv_);
        if (FAILED(hr)) return false;

        D3D11_TEXTURE2D_DESC dd{};
        dd.Width = static_cast<UINT>(width_);
        dd.Height = static_cast<UINT>(height_);
        dd.MipLevels = 1;
        dd.ArraySize = 1;
        dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        dd.SampleDesc.Count = 1;
        dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        hr = device_->CreateTexture2D(&dd, nullptr, &depth_);
        if (FAILED(hr)) return false;
        hr = device_->CreateDepthStencilView(depth_.Get(), nullptr, &dsv_);
        return SUCCEEDED(hr);
    }

    bool CompileShader(const char* src, const char* entry, const char* profile, ComPtr<ID3DBlob>& blob) {
        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        uint64_t cacheHash = ShaderCacheHash(src, entry, profile, flags);
        ReportShaderActivity(L"Checking cache for", entry, profile);
        if (LoadShaderCache(entry, cacheHash, blob)) {
            ReportShaderComplete(L"Loaded cached shader", entry, profile, false);
            return true;
        }

        ReportShaderActivity(L"Compiling", entry, profile);
        ComPtr<ID3DBlob> errors;
        HRESULT hr = D3DCompile(src, std::strlen(src), nullptr, nullptr, nullptr, entry, profile, flags, 0, &blob, &errors);
        if (FAILED(hr) && errors) {
            const char* errorText = static_cast<const char*>(errors->GetBufferPointer());
            OutputDebugStringA(errorText);
            std::string bytes(errorText, errorText + errors->GetBufferSize());
            StartupProfileLine(L"Shader compile failed for " + std::wstring(entry, entry + std::strlen(entry)) + L": " + std::wstring(bytes.begin(), bytes.end()));
        }
        if (SUCCEEDED(hr)) {
            SaveShaderCache(entry, cacheHash, blob.Get());
            ReportShaderComplete(L"Compiled shader", entry, profile, true);
        }
        return SUCCEEDED(hr);
    }

    static uint64_t Fnv1aAppend(uint64_t hash, const void* data, size_t size) {
        const uint8_t* bytes = static_cast<const uint8_t*>(data);
        for (size_t i = 0; i < size; ++i) {
            hash ^= bytes[i];
            hash *= 1099511628211ull;
        }
        return hash;
    }

    static uint64_t ShaderCacheHash(const char* src, const char* entry, const char* profile, UINT flags) {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeShaderCacheV4";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, entry, std::strlen(entry) + 1);
        hash = Fnv1aAppend(hash, profile, std::strlen(profile) + 1);
        hash = Fnv1aAppend(hash, &flags, sizeof(flags));
        hash = Fnv1aAppend(hash, src, std::strlen(src));
        return hash;
    }

    static std::wstring ShaderCacheFileName(const char* entry) {
        std::wstring safeEntry;
        for (const char* p = entry; *p; ++p) {
            safeEntry.push_back((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')
                ? static_cast<wchar_t>(*p)
                : L'_');
        }
        return L"BackroomsMaze_" + safeEntry + L".cso";
    }

    static std::filesystem::path ShaderCachePath(const char* entry) {
        return CacheDirectory() / ShaderCacheFileName(entry);
    }

    static bool LoadShaderCache(const char* entry, uint64_t hash, ComPtr<ID3DBlob>& blob) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t size;
            uint32_t reserved;
        };
        std::filesystem::path bundledPath = ModuleDirectory() / L"ShaderCache" / ShaderCacheFileName(entry);
        std::filesystem::path writablePath = ShaderCachePath(entry);
        const std::array<std::filesystem::path, 2> candidates = {writablePath, bundledPath};
        for (const auto& candidate : candidates) {
            Header header{};
            std::ifstream in(candidate, std::ios::binary);
            if (!in) continue;
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (!in || std::memcmp(header.magic, "BRMCSO1", 7) != 0 || header.hash != hash || header.size < 4 || header.size > 16u * 1024u * 1024u) {
                continue;
            }
            ComPtr<ID3DBlob> loaded;
            if (FAILED(D3DCreateBlob(header.size, &loaded))) continue;
            in.read(static_cast<char*>(loaded->GetBufferPointer()), header.size);
            if (!in) continue;
            if (std::memcmp(loaded->GetBufferPointer(), "DXBC", 4) != 0) continue;
            blob = loaded;
            return true;
        }
        return false;
    }

    static void SaveShaderCache(const char* entry, uint64_t hash, ID3DBlob* blob) {
        if (!blob || blob->GetBufferSize() > 16u * 1024u * 1024u) return;
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t size;
            uint32_t reserved;
        };
        Header header{{'B', 'R', 'M', 'C', 'S', 'O', '1', '\0'}, hash, static_cast<uint32_t>(blob->GetBufferSize()), 0};
        std::ofstream out(ShaderCachePath(entry), std::ios::binary | std::ios::trunc);
        if (!out) return;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(static_cast<const char*>(blob->GetBufferPointer()), blob->GetBufferSize());
    }

    static uint64_t HashWide(uint64_t hash, const std::wstring& text) {
        return Fnv1aAppend(hash, text.data(), text.size() * sizeof(wchar_t));
    }

    static uint64_t HashFileSignature(uint64_t hash, const std::filesystem::path& path) {
        std::error_code ec;
        uintmax_t size = std::filesystem::file_size(path, ec);
        hash = Fnv1aAppend(hash, &size, sizeof(size));
        std::ifstream in(path, std::ios::binary);
        if (!in) return hash;

        constexpr std::streamoff kSignatureChunk = 64 * 1024;
        std::array<char, static_cast<size_t>(kSignatureChunk)> buffer{};
        auto readChunk = [&](std::streamoff offset, std::streamsize wanted) {
            if (wanted <= 0) return;
            in.clear();
            in.seekg(offset, std::ios::beg);
            in.read(buffer.data(), wanted);
            std::streamsize read = in.gcount();
            if (read > 0) {
                hash = Fnv1aAppend(hash, buffer.data(), static_cast<size_t>(read));
            }
        };

        std::streamsize firstBytes = static_cast<std::streamsize>(std::min<uintmax_t>(size, static_cast<uintmax_t>(kSignatureChunk)));
        readChunk(0, firstBytes);
        if (size > static_cast<uintmax_t>(kSignatureChunk)) {
            std::streamoff lastOffset = static_cast<std::streamoff>(size - static_cast<uintmax_t>(kSignatureChunk));
            readChunk(lastOffset, static_cast<std::streamsize>(kSignatureChunk));
        }
        return hash;
    }

    uint64_t TextureCacheHash() const {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeTextureCacheV10";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, &kTextureSize, sizeof(kTextureSize));
        hash = Fnv1aAppend(hash, &kMaterialCount, sizeof(kMaterialCount));
        hash = HashWide(hash, settings_.wallStem);
        hash = HashWide(hash, settings_.floorStem);
        hash = HashWide(hash, settings_.ceilingStem);
        hash = HashWide(hash, settings_.fleshStem);
        hash = Fnv1aAppend(hash, &settings_.useExternalNormals, sizeof(settings_.useExternalNormals));
        hash = Fnv1aAppend(hash, &settings_.maxNormalMapMB, sizeof(settings_.maxNormalMapMB));

        auto addResolvedAsset = [&](const std::wstring& logicalName, const std::filesystem::path& path) {
            hash = HashWide(hash, logicalName);
            std::error_code ec;
            bool exists = !path.empty() && std::filesystem::exists(path, ec);
            hash = Fnv1aAppend(hash, &exists, sizeof(exists));
            if (exists) {
                hash = HashFileSignature(hash, path);
            }
        };

        auto addPbrAsset = [&](const std::wstring& stem, const wchar_t* suffix) {
            if (stem.empty()) {
                uint8_t missing = 0;
                hash = Fnv1aAppend(hash, &missing, sizeof(missing));
                return;
            }
            std::wstring logicalName = stem + suffix;
            addResolvedAsset(logicalName, ResolveAsset(settings_, logicalName));
        };

        const std::wstring stems[] = {settings_.wallStem, settings_.floorStem, settings_.ceilingStem, settings_.fleshStem};
        for (const std::wstring& stem : stems) {
            addPbrAsset(stem, L"_color_4k.jpg");
            addPbrAsset(stem, L"_height_4k.png");
            addPbrAsset(stem, L"_normal_directx_4k.png");
            addPbrAsset(stem, L"_ao_4k.jpg");
            addPbrAsset(stem, L"_roughness_4k.jpg");
        }

        const wchar_t* runtimeTextures[] = {
            L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg",
            L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg",
            L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png",
            L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png"
        };
        for (const wchar_t* texture : runtimeTextures) {
            addResolvedAsset(texture, ResolveConfiguredAssetPath(texture));
        }
        return hash;
    }

    static std::wstring TextureCacheFileName() {
#if defined(BACKROOMS_GAME_EXE)
        return L"BackroomsMazeGame_textures.bin";
#else
        return L"BackroomsMaze_textures.bin";
#endif
    }

    static std::filesystem::path TextureCachePath() {
        return CacheDirectory() / TextureCacheFileName();
    }

    static bool LoadTextureCache(uint64_t hash, std::vector<uint8_t>& albedo, std::vector<uint8_t>& normal, std::vector<uint8_t>& props) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t textureSize;
            uint32_t materialCount;
            uint64_t albedoSize;
            uint64_t normalSize;
            uint64_t propsSize;
        };
        std::filesystem::path bundledPath = ModuleDirectory() / L"TextureCache" / TextureCacheFileName();
        std::filesystem::path writablePath = TextureCachePath();
        const std::array<std::filesystem::path, 2> candidates = {writablePath, bundledPath};
        for (const auto& candidate : candidates) {
            Header header{};
            std::ifstream in(candidate, std::ios::binary);
            if (!in) continue;
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (!in ||
                std::memcmp(header.magic, "BRMTEX2", 7) != 0 ||
                header.hash != hash ||
                header.textureSize != kTextureSize ||
                header.materialCount != kMaterialCount ||
                header.albedoSize != albedo.size() ||
                header.normalSize != normal.size() ||
                header.propsSize != props.size()) {
                continue;
            }
            in.read(reinterpret_cast<char*>(albedo.data()), static_cast<std::streamsize>(albedo.size()));
            if (!in) continue;
            in.read(reinterpret_cast<char*>(normal.data()), static_cast<std::streamsize>(normal.size()));
            if (!in) continue;
            in.read(reinterpret_cast<char*>(props.data()), static_cast<std::streamsize>(props.size()));
            if (in) return true;
        }
        return false;
    }

    static void SaveTextureCache(uint64_t hash, const std::vector<uint8_t>& albedo, const std::vector<uint8_t>& normal, const std::vector<uint8_t>& props) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t textureSize;
            uint32_t materialCount;
            uint64_t albedoSize;
            uint64_t normalSize;
            uint64_t propsSize;
        };
        Header header{{'B', 'R', 'M', 'T', 'E', 'X', '2', '\0'}, hash, kTextureSize, kMaterialCount, albedo.size(), normal.size(), props.size()};
        std::ofstream out(TextureCachePath(), std::ios::binary | std::ios::trunc);
        if (!out) return;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(albedo.data()), static_cast<std::streamsize>(albedo.size()));
        out.write(reinterpret_cast<const char*>(normal.data()), static_cast<std::streamsize>(normal.size()));
        out.write(reinterpret_cast<const char*>(props.data()), static_cast<std::streamsize>(props.size()));
    }

    std::filesystem::path ResolveConfiguredAssetPath(const std::wstring& configuredPath) const {
        if (configuredPath.empty()) return {};
        std::filesystem::path p(configuredPath);
        std::vector<std::filesystem::path> candidates;
        auto add = [&](std::filesystem::path c) {
            c = c.lexically_normal();
            if (std::find(candidates.begin(), candidates.end(), c) == candidates.end()) candidates.push_back(std::move(c));
        };
        if (p.is_absolute()) {
            add(p);
        } else {
            std::filesystem::path module = ModuleDirectory();
            add(module / p);
            add(module.parent_path() / p);
            add(module.parent_path().parent_path() / p);
            add(std::filesystem::current_path() / p);
            std::filesystem::path resolved = ResolveAsset(settings_, configuredPath);
            if (!resolved.empty()) add(resolved);
        }
        for (const auto& candidate : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(candidate, ec)) return candidate;
        }
        return {};
    }

    uint64_t MonsterMeshCacheHash(const std::filesystem::path& path) const {
        uint64_t hash = 1469598103934665603ull;
        const char* version = "BackroomsMazeMonsterMeshCacheV8";
        hash = Fnv1aAppend(hash, version, std::strlen(version));
        hash = Fnv1aAppend(hash, &settings_.monsterSkullMaxTriangles, sizeof(settings_.monsterSkullMaxTriangles));
        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        hash = HashWide(hash, ext);
        return HashFileSignature(hash, path);
    }

    static std::wstring MonsterMeshCacheFileName(uint64_t hash) {
        std::wstringstream name;
        name << L"BackroomsMaze_skullmesh_" << std::hex << hash << L".bin";
        return name.str();
    }

    static std::filesystem::path MonsterMeshCachePath(uint64_t hash) {
        return CacheDirectory() / MonsterMeshCacheFileName(hash);
    }

    static bool LoadMonsterMeshCache(uint64_t hash, std::vector<Vertex>& out) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t vertexCount;
            uint32_t reserved;
        };
        std::filesystem::path bundledPath = ModuleDirectory() / L"MeshCache" / MonsterMeshCacheFileName(hash);
        std::filesystem::path writablePath = MonsterMeshCachePath(hash);
        const std::array<std::filesystem::path, 2> candidates = {writablePath, bundledPath};
        for (const auto& candidate : candidates) {
            Header header{};
            std::ifstream in(candidate, std::ios::binary);
            if (!in) continue;
            in.read(reinterpret_cast<char*>(&header), sizeof(header));
            if (!in || std::memcmp(header.magic, "BRMMSH1", 7) != 0 || header.hash != hash ||
                header.vertexCount == 0 || header.vertexCount > static_cast<uint32_t>(kDynamicVertexCapacity)) {
                continue;
            }
            out.resize(header.vertexCount);
            in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size() * sizeof(Vertex)));
            if (in.good()) return true;
            out.clear();
        }
        return false;
    }

    static bool LoadMonsterMeshFile(const std::filesystem::path& path, std::vector<Vertex>& out) {
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t vertexCount;
            uint32_t reserved;
        };
        Header header{};
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!in || std::memcmp(header.magic, "BRMMSH1", 7) != 0 ||
            header.vertexCount == 0 || header.vertexCount > static_cast<uint32_t>(kDynamicVertexCapacity)) {
            return false;
        }
        out.resize(header.vertexCount);
        in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size() * sizeof(Vertex)));
        if (in.good()) return true;
        out.clear();
        return false;
    }

    static void SaveMonsterMeshCache(uint64_t hash, const std::vector<Vertex>& mesh) {
        if (mesh.empty() || mesh.size() > static_cast<size_t>(kDynamicVertexCapacity)) return;
        struct Header {
            char magic[8];
            uint64_t hash;
            uint32_t vertexCount;
            uint32_t reserved;
        };
        Header header{{'B', 'R', 'M', 'M', 'S', 'H', '1', '\0'}, hash, static_cast<uint32_t>(mesh.size()), 0};
        std::ofstream out(MonsterMeshCachePath(hash), std::ios::binary | std::ios::trunc);
        if (!out) return;
        out.write(reinterpret_cast<const char*>(&header), sizeof(header));
        out.write(reinterpret_cast<const char*>(mesh.data()), static_cast<std::streamsize>(mesh.size() * sizeof(Vertex)));
    }

    static int ParseObjFaceIndex(const char*& p, int vertexCount) {
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) return std::numeric_limits<int>::min();
        char* end = nullptr;
        long raw = std::strtol(p, &end, 10);
        if (end == p) {
            while (*p && !std::isspace(static_cast<unsigned char>(*p))) ++p;
            return std::numeric_limits<int>::min();
        }
        while (*end && !std::isspace(static_cast<unsigned char>(*end))) ++end;
        p = end;
        if (raw > 0) return static_cast<int>(raw - 1);
        if (raw < 0) return vertexCount + static_cast<int>(raw);
        return std::numeric_limits<int>::min();
    }

    struct ObjFaceVertex {
        int vertex = std::numeric_limits<int>::min();
        int texcoord = std::numeric_limits<int>::min();
    };

    static int ResolveObjIndex(long raw, int count) {
        if (raw > 0) return static_cast<int>(raw - 1);
        if (raw < 0) return count + static_cast<int>(raw);
        return std::numeric_limits<int>::min();
    }

    static ObjFaceVertex ParseObjFaceVertex(const char*& p, int vertexCount, int texcoordCount) {
        ObjFaceVertex result{};
        while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
        if (!*p) return result;

        char* end = nullptr;
        long rawVertex = std::strtol(p, &end, 10);
        if (end == p) {
            while (*p && !std::isspace(static_cast<unsigned char>(*p))) ++p;
            return result;
        }
        result.vertex = ResolveObjIndex(rawVertex, vertexCount);

        const char* q = end;
        if (*q == '/') {
            ++q;
            if (*q && *q != '/') {
                char* uvEnd = nullptr;
                long rawUv = std::strtol(q, &uvEnd, 10);
                if (uvEnd != q) {
                    result.texcoord = ResolveObjIndex(rawUv, texcoordCount);
                    q = uvEnd;
                }
            }
        }
        while (*q && !std::isspace(static_cast<unsigned char>(*q))) ++q;
        p = q;
        return result;
    }

    bool LoadMonsterSkullObj(const std::filesystem::path& path, std::vector<Vertex>& out) const {
        struct Cluster {
            XMFLOAT3 sum{0.0f, 0.0f, 0.0f};
            XMFLOAT3 normal{0.0f, 0.0f, 0.0f};
            int count = 0;
        };
        struct Tri {
            int a;
            int b;
            int c;
        };

        int maxTriangles = std::max(0, settings_.monsterSkullMaxTriangles);
        if (maxTriangles <= 0) return false;

        std::ifstream in(path);
        if (!in) return false;

        std::vector<XMFLOAT3> positions;
        positions.reserve(250000);
        XMFLOAT3 minP{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT3 maxP{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        std::string line;
        while (std::getline(in, line)) {
            if (line.size() < 2 || line[0] != 'v' || line[1] != ' ') continue;
            const char* p = line.c_str() + 2;
            char* end = nullptr;
            float x = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float y = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float z = std::strtof(p, &end);
            if (end == p) continue;
            positions.push_back({x, y, z});
            minP.x = std::min(minP.x, x);
            minP.y = std::min(minP.y, y);
            minP.z = std::min(minP.z, z);
            maxP.x = std::max(maxP.x, x);
            maxP.y = std::max(maxP.y, y);
            maxP.z = std::max(maxP.z, z);
        }
        if (positions.empty()) return false;

        XMFLOAT3 center{(minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f};
        float maxDim = std::max({maxP.x - minP.x, maxP.y - minP.y, maxP.z - minP.z, 0.001f});
        float scale = 1.12f / maxDim;
        auto localPos = [&](int idx) {
            const XMFLOAT3& p = positions[static_cast<size_t>(idx)];
            return XMFLOAT3{(p.x - center.x) * scale, (p.z - center.z) * scale, (p.y - center.y) * scale};
        };

        auto buildClustered = [&](int grid, std::vector<Cluster>& clusters, std::vector<Tri>& tris) {
            clusters.clear();
            tris.clear();
            std::unordered_map<int, int> clusterByCell;
            std::unordered_set<uint64_t> seen;
            clusterByCell.reserve(static_cast<size_t>(grid * grid * 4));
            seen.reserve(static_cast<size_t>(maxTriangles * 2));
            auto clusterIndex = [&](int idx) {
                XMFLOAT3 p = localPos(idx);
                int ix = std::clamp(static_cast<int>((p.x / 1.12f + 0.5f) * static_cast<float>(grid)), 0, grid - 1);
                int iy = std::clamp(static_cast<int>((p.y / 1.12f + 0.5f) * static_cast<float>(grid)), 0, grid - 1);
                int iz = std::clamp(static_cast<int>((p.z / 1.12f + 0.5f) * static_cast<float>(grid)), 0, grid - 1);
                int key = ix + iy * grid + iz * grid * grid;
                auto found = clusterByCell.find(key);
                int ci = 0;
                if (found == clusterByCell.end()) {
                    ci = static_cast<int>(clusters.size());
                    clusterByCell.emplace(key, ci);
                    clusters.push_back({});
                } else {
                    ci = found->second;
                }
                Cluster& c = clusters[static_cast<size_t>(ci)];
                c.sum = Add3(c.sum, p);
                ++c.count;
                return ci;
            };
            auto addTri = [&](int ia, int ib, int ic) {
                int ca = clusterIndex(ia);
                int cb = clusterIndex(ib);
                int cc = clusterIndex(ic);
                if (ca == cb || cb == cc || ca == cc) return;
                int sa = ca;
                int sb = cb;
                int sc = cc;
                if (sa > sb) std::swap(sa, sb);
                if (sb > sc) std::swap(sb, sc);
                if (sa > sb) std::swap(sa, sb);
                if (sc >= (1 << 21)) return;
                uint64_t key = static_cast<uint64_t>(sa) |
                    (static_cast<uint64_t>(sb) << 21) |
                    (static_cast<uint64_t>(sc) << 42);
                if (!seen.insert(key).second) return;

                XMFLOAT3 a = localPos(ia);
                XMFLOAT3 b = localPos(ib);
                XMFLOAT3 c = localPos(ic);
                XMFLOAT3 rawNormal = Cross3(Sub3(b, a), Sub3(c, a));
                XMFLOAT3 n = Normalize3(rawNormal, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 faceCenter = Scale3(Add3(Add3(a, b), c), 1.0f / 3.0f);
                if (Dot3(n, faceCenter) < 0.0f) rawNormal = Scale3(rawNormal, -1.0f);
                clusters[static_cast<size_t>(ca)].normal = Add3(clusters[static_cast<size_t>(ca)].normal, rawNormal);
                clusters[static_cast<size_t>(cb)].normal = Add3(clusters[static_cast<size_t>(cb)].normal, rawNormal);
                clusters[static_cast<size_t>(cc)].normal = Add3(clusters[static_cast<size_t>(cc)].normal, rawNormal);
                tris.push_back({ca, cb, cc});
            };

            std::ifstream faces(path);
            if (!faces) return;
            std::string faceLine;
            while (std::getline(faces, faceLine)) {
                if (faceLine.size() < 2 || faceLine[0] != 'f' || faceLine[1] != ' ') continue;
                std::vector<int> poly;
                poly.reserve(8);
                const char* p = faceLine.c_str() + 2;
                while (*p) {
                    int idx = ParseObjFaceIndex(p, static_cast<int>(positions.size()));
                    if (idx >= 0 && idx < static_cast<int>(positions.size())) poly.push_back(idx);
                }
                if (poly.size() < 3) continue;
                for (size_t i = 1; i + 1 < poly.size(); ++i) {
                    addTri(poly[0], poly[i], poly[i + 1]);
                }
            }
        };

        std::vector<Cluster> clusters;
        std::vector<Tri> tris;
        int grid = std::clamp(static_cast<int>(std::sqrt(static_cast<float>(maxTriangles) / 3.0f)), 18, 128);
        for (int attempt = 0; attempt < 4; ++attempt) {
            buildClustered(grid, clusters, tris);
            if (!tris.empty() && tris.size() <= static_cast<size_t>(maxTriangles)) break;
            grid = std::max(10, static_cast<int>(static_cast<float>(grid) * 0.82f));
        }
        if (clusters.empty() || tris.empty()) return false;
        if (tris.size() > static_cast<size_t>(maxTriangles)) {
            std::vector<Tri> reduced;
            reduced.reserve(static_cast<size_t>(maxTriangles));
            double step = static_cast<double>(tris.size()) / static_cast<double>(maxTriangles);
            for (int i = 0; i < maxTriangles; ++i) {
                reduced.push_back(tris[static_cast<size_t>(std::min<double>(tris.size() - 1, i * step))]);
            }
            tris = std::move(reduced);
        }

        out.clear();
        out.reserve(tris.size() * 3);
        auto pushVertex = [&](int cluster, XMFLOAT2 uv) {
            const Cluster& c = clusters[static_cast<size_t>(cluster)];
            XMFLOAT3 p = c.count > 0 ? Scale3(c.sum, 1.0f / static_cast<float>(c.count)) : XMFLOAT3{};
            XMFLOAT3 radial = Normalize3({p.x * 0.86f, p.y * 1.18f, p.z * 0.86f}, Normalize3(p, {0.0f, 1.0f, 0.0f}));
            XMFLOAT3 averaged = Normalize3(c.normal, radial);
            if (Dot3(averaged, radial) < 0.18f) averaged = radial;
            XMFLOAT3 n = Normalize3(Lerp3(averaged, radial, 0.62f), radial);
            XMFLOAT3 tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            out.push_back({p, n, tangent, uv, 9.65f});
        };
        for (const Tri& tri : tris) {
            pushVertex(tri.a, {0.0f, 0.0f});
            pushVertex(tri.b, {1.0f, 0.0f});
            pushVertex(tri.c, {0.5f, 1.0f});
        }
        return !out.empty();
    }

    bool LoadMonsterSkullMesh() {
        skullMesh_.clear();
        monsterUsingAltSkull_ = false;
        std::wstring configuredPath = settings_.monsterSkullMesh;
        if (!settings_.monsterAltSkullMesh.empty() &&
            Rand01(913, 917, runtimeSeed_) < std::clamp(settings_.monsterAltSkullChance, 0.0f, 1.0f)) {
            configuredPath = settings_.monsterAltSkullMesh;
            monsterUsingAltSkull_ = true;
        }
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty()) {
            StartupProfileLine(L"Monster skull mesh not found: " + configuredPath);
            return false;
        }
        uint64_t hash = MonsterMeshCacheHash(path);
        if (LoadMonsterMeshCache(hash, skullMesh_)) {
            StartupProfileLine(L"Loaded cached monster skull mesh: " + std::to_wstring(skullMesh_.size() / 3) + L" tris");
            return true;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        bool ok = false;
        if (ext == L".bin") {
            ok = LoadMonsterMeshFile(path, skullMesh_);
        } else if (ext == L".obj") {
            ok = LoadMonsterSkullObj(path, skullMesh_);
        }
        if (ok) {
            if (ext != L".bin") {
                SaveMonsterMeshCache(hash, skullMesh_);
            }
            StartupProfileLine(L"Loaded monster skull mesh: " + path.wstring() + L" (" + std::to_wstring(skullMesh_.size() / 3) + L" tris)");
        } else {
            skullMesh_.clear();
            StartupProfileLine(L"Could not load monster skull mesh: " + path.wstring());
        }
        return ok;
    }

    static bool LoadStaticPropBinary(const std::filesystem::path& path, StaticPropMesh& out) {
        struct Header {
            char magic[8];
            uint32_t vertexCount;
            uint32_t vertexStride;
            float min[3];
            float max[3];
            float extra[4];
        };

        Header header{};
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        in.read(reinterpret_cast<char*>(&header), sizeof(header));
        constexpr uint32_t kMaxStaticPropVertices = 1000000;
        bool isRawVertexFile = std::memcmp(header.magic, "BRMPRP1", 7) == 0;
        bool isPackedVertexFileV2 = std::memcmp(header.magic, "BRMPRP2", 7) == 0;
        bool isPackedVertexFile = std::memcmp(header.magic, "BRMPRP3", 7) == 0;
        if (!in || (!isRawVertexFile && !isPackedVertexFileV2 && !isPackedVertexFile) ||
            header.vertexCount == 0 ||
            header.vertexCount > kMaxStaticPropVertices) {
            return false;
        }

        if (isRawVertexFile) {
            if (header.vertexStride != sizeof(Vertex)) return false;
            out.vertices.resize(header.vertexCount);
            in.read(reinterpret_cast<char*>(out.vertices.data()),
                static_cast<std::streamsize>(out.vertices.size() * sizeof(Vertex)));
            if (!in) {
                out = {};
                return false;
            }
        } else if (isPackedVertexFileV2) {
            if (header.vertexStride != sizeof(PackedStaticPropVertexV2)) return false;
            std::vector<PackedStaticPropVertexV2> packed(header.vertexCount);
            in.read(reinterpret_cast<char*>(packed.data()),
                static_cast<std::streamsize>(packed.size() * sizeof(PackedStaticPropVertexV2)));
            if (!in) {
                out = {};
                return false;
            }
            XMFLOAT3 minP{header.min[0], header.min[1], header.min[2]};
            XMFLOAT3 extent{
                header.max[0] - header.min[0],
                header.max[1] - header.min[1],
                header.max[2] - header.min[2]
            };
            auto unpackUnit = [](uint16_t v) {
                return static_cast<float>(v) * (1.0f / 65535.0f);
            };
            auto unpackSnorm = [](int16_t v) {
                return std::clamp(static_cast<float>(v) * (1.0f / 32767.0f), -1.0f, 1.0f);
            };
            out.vertices.reserve(header.vertexCount);
            for (const PackedStaticPropVertexV2& src : packed) {
                XMFLOAT3 pos{
                    minP.x + extent.x * unpackUnit(src.px),
                    minP.y + extent.y * unpackUnit(src.py),
                    minP.z + extent.z * unpackUnit(src.pz)
                };
                XMFLOAT3 normal = Normalize3({unpackSnorm(src.nx), unpackSnorm(src.ny), unpackSnorm(src.nz)}, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 tangent = Normalize3({unpackSnorm(src.tx), unpackSnorm(src.ty), unpackSnorm(src.tz)}, {1.0f, 0.0f, 0.0f});
                out.vertices.push_back({pos, normal, tangent, {src.u, src.v}, static_cast<float>(src.material)});
            }
        } else {
            if (header.vertexStride != sizeof(PackedStaticPropVertex)) return false;
            std::vector<PackedStaticPropVertex> packed(header.vertexCount);
            in.read(reinterpret_cast<char*>(packed.data()),
                static_cast<std::streamsize>(packed.size() * sizeof(PackedStaticPropVertex)));
            if (!in) {
                out = {};
                return false;
            }
            XMFLOAT3 minP{header.min[0], header.min[1], header.min[2]};
            XMFLOAT3 extent{
                header.max[0] - header.min[0],
                header.max[1] - header.min[1],
                header.max[2] - header.min[2]
            };
            float uvMinU = header.extra[0];
            float uvMinV = header.extra[1];
            float uvExtentU = header.extra[2] - header.extra[0];
            float uvExtentV = header.extra[3] - header.extra[1];
            auto unpackUnit = [](uint16_t v) {
                return static_cast<float>(v) * (1.0f / 65535.0f);
            };
            auto unpackSnorm = [](int16_t v) {
                return std::clamp(static_cast<float>(v) * (1.0f / 32767.0f), -1.0f, 1.0f);
            };
            out.vertices.reserve(header.vertexCount);
            for (const PackedStaticPropVertex& src : packed) {
                XMFLOAT3 pos{
                    minP.x + extent.x * unpackUnit(src.px),
                    minP.y + extent.y * unpackUnit(src.py),
                    minP.z + extent.z * unpackUnit(src.pz)
                };
                XMFLOAT3 normal = Normalize3({unpackSnorm(src.nx), unpackSnorm(src.ny), unpackSnorm(src.nz)}, {0.0f, 1.0f, 0.0f});
                XMFLOAT3 tangent = Normalize3({unpackSnorm(src.tx), unpackSnorm(src.ty), unpackSnorm(src.tz)}, {1.0f, 0.0f, 0.0f});
                XMFLOAT2 uv{
                    uvMinU + uvExtentU * unpackUnit(src.u),
                    uvMinV + uvExtentV * unpackUnit(src.v)
                };
                out.vertices.push_back({pos, normal, tangent, uv, static_cast<float>(src.material)});
            }
        }
        out.min = {header.min[0], header.min[1], header.min[2]};
        out.max = {header.max[0], header.max[1], header.max[2]};
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
    }

    bool LoadStaticPropObj(const std::wstring& configuredPath, float material, StaticPropMesh& out) const {
        out = {};
        std::filesystem::path path = ResolveConfiguredAssetPath(configuredPath);
        if (path.empty()) {
            std::filesystem::path fallbackConfigured(configuredPath);
            std::wstring ext = fallbackConfigured.extension().wstring();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
            if (ext == L".brmesh") {
                fallbackConfigured.replace_extension(L".obj");
                path = ResolveConfiguredAssetPath(fallbackConfigured.wstring());
            }
        }
        if (path.empty()) {
            StartupProfileLine(L"Prop mesh not found: " + configuredPath);
            return false;
        }

        std::wstring ext = path.extension().wstring();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
        if (ext == L".brmesh") {
            if (LoadStaticPropBinary(path, out)) return true;

            std::filesystem::path fallbackPath = path;
            fallbackPath.replace_extension(L".obj");
            std::error_code ec;
            if (std::filesystem::exists(fallbackPath, ec)) {
                out = {};
                path = fallbackPath;
            } else {
                StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
                return false;
            }
        } else if (ext == L".bin") {
            if (LoadStaticPropBinary(path, out)) return true;
            StartupProfileLine(L"Could not load prop mesh: " + path.wstring());
            return false;
        }

        std::ifstream in(path);
        if (!in) return false;

        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT2> texcoords;
        positions.reserve(20000);
        texcoords.reserve(20000);
        XMFLOAT3 minP{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        XMFLOAT3 maxP{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
        std::string line;
        while (std::getline(in, line)) {
            if (line.size() >= 3 && line[0] == 'v' && line[1] == 't' && std::isspace(static_cast<unsigned char>(line[2]))) {
                const char* p = line.c_str() + 3;
                char* end = nullptr;
                float u = std::strtof(p, &end);
                if (end == p) continue;
                p = end;
                float v = std::strtof(p, &end);
                if (end == p) continue;
                texcoords.push_back({u, v});
                continue;
            }
            if (line.size() < 2 || line[0] != 'v' || line[1] != ' ') continue;
            const char* p = line.c_str() + 2;
            char* end = nullptr;
            float x = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float y = std::strtof(p, &end);
            if (end == p) continue;
            p = end;
            float z = std::strtof(p, &end);
            if (end == p) continue;
            positions.push_back({x, y, z});
            minP.x = std::min(minP.x, x);
            minP.y = std::min(minP.y, y);
            minP.z = std::min(minP.z, z);
            maxP.x = std::max(maxP.x, x);
            maxP.y = std::max(maxP.y, y);
            maxP.z = std::max(maxP.z, z);
        }
        if (positions.empty()) return false;

        auto uvFor = [&](XMFLOAT3 p, XMFLOAT3 n, int texcoord) {
            if (texcoord >= 0 && texcoord < static_cast<int>(texcoords.size())) {
                return texcoords[static_cast<size_t>(texcoord)];
            }
            if (std::abs(n.y) > 0.62f) return XMFLOAT2{p.x * 1.7f + 0.5f, p.z * 1.7f + 0.5f};
            if (std::abs(n.x) > std::abs(n.z)) return XMFLOAT2{p.z * 1.7f + 0.5f, p.y * 1.7f};
            return XMFLOAT2{p.x * 1.7f + 0.5f, p.y * 1.7f};
        };
        auto addTri = [&](ObjFaceVertex va, ObjFaceVertex vb, ObjFaceVertex vc, float faceMaterial) {
            XMFLOAT3 a = positions[static_cast<size_t>(va.vertex)];
            XMFLOAT3 b = positions[static_cast<size_t>(vb.vertex)];
            XMFLOAT3 c = positions[static_cast<size_t>(vc.vertex)];
            XMFLOAT3 n = Normalize3(Cross3(Sub3(b, a), Sub3(c, a)), {0.0f, 1.0f, 0.0f});
            XMFLOAT3 tangent = Normalize3(Sub3(b, a), {1.0f, 0.0f, 0.0f});
            if (std::abs(Dot3(tangent, n)) > 0.92f) {
                tangent = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, n), {1.0f, 0.0f, 0.0f});
            }
            out.vertices.push_back({a, n, tangent, uvFor(a, n, va.texcoord), faceMaterial});
            out.vertices.push_back({b, n, tangent, uvFor(b, n, vb.texcoord), faceMaterial});
            out.vertices.push_back({c, n, tangent, uvFor(c, n, vc.texcoord), faceMaterial});
        };

        std::ifstream faces(path);
        if (!faces) return false;
        float currentMaterial = material;
        while (std::getline(faces, line)) {
            if (line.rfind("usemtl ", 0) == 0) {
                const char* p = line.c_str() + 7;
                while (*p && std::isspace(static_cast<unsigned char>(*p))) ++p;
                currentMaterial = material;
                if (*p == 'p' || *p == 'P') {
                    char* end = nullptr;
                    long id = std::strtol(p + 1, &end, 10);
                    if (end != p + 1 && id >= 0 && id < kMaterialCount) {
                        currentMaterial = static_cast<float>(id);
                    }
                }
                continue;
            }
            if (line.size() < 2 || line[0] != 'f' || line[1] != ' ') continue;
            std::vector<ObjFaceVertex> poly;
            poly.reserve(8);
            const char* p = line.c_str() + 2;
            while (*p) {
                ObjFaceVertex fv = ParseObjFaceVertex(p, static_cast<int>(positions.size()), static_cast<int>(texcoords.size()));
                if (fv.vertex >= 0 && fv.vertex < static_cast<int>(positions.size())) poly.push_back(fv);
            }
            if (poly.size() < 3) continue;
            for (size_t i = 1; i + 1 < poly.size(); ++i) {
                addTri(poly[0], poly[i], poly[i + 1], currentMaterial);
            }
        }

        if (out.vertices.empty()) return false;
        out.min = minP;
        out.max = maxP;
        out.generatedUvFallback = StaticPropNeedsGeneratedUv(out);
        return true;
    }

    bool LoadPropMeshes() {
        for (StaticPropMesh& mesh : chairPropMeshes_) mesh = {};
        cabinetPropMesh_ = {};
        deskPropMesh_ = {};
        trashBinPropMesh_ = {};
        deskLampPropMesh_ = {};
        cassettePropMesh_ = {};
        airVentPropMesh_ = {};
        exitSignPropMesh_ = {};
        for (StaticPropMesh& mesh : ceilingLampPropMeshes_) mesh = {};

        int loaded = 0;
        const std::array<std::wstring, 3> chairPaths = {
            L"assets\\models\\runtime\\office_chair_modern.brmesh",
            L"assets\\models\\runtime\\office_chair_classic.brmesh",
            L"assets\\models\\runtime\\office_chair_task.brmesh"
        };
        const std::array<float, 3> chairMaterials = {16.0f, 17.0f, 22.0f};
        for (size_t i = 0; i < chairPaths.size(); ++i) {
            if (LoadStaticPropObj(chairPaths[i], chairMaterials[i], chairPropMeshes_[i])) ++loaded;
        }
        bool cabinetLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\filing_cabinet.brmesh", 10.0f, cabinetPropMesh_);
        bool deskLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\office_desk.brmesh", 8.0f, deskPropMesh_);
        bool trashBinLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\trashbin.brmesh", 10.0f, trashBinPropMesh_);
        bool deskLampLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\desklamp.brmesh", 21.0f, deskLampPropMesh_);
        bool cassetteLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\audio_caset.brmesh", 23.0f, cassettePropMesh_);
        bool airVentLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\air_vent.brmesh", 10.0f, airVentPropMesh_);
        bool exitSignLoaded = LoadStaticPropObj(L"assets\\models\\runtime\\emergency_exit_sign.brmesh", 7.0f, exitSignPropMesh_);
        int ceilingLampLoaded = 0;
        for (size_t i = 0; i < ceilingLampPropMeshes_.size(); ++i) {
            wchar_t path[96]{};
            swprintf_s(path, L"assets\\models\\runtime\\ceiling_lamp_%02zu.brmesh", i + 1);
            if (LoadStaticPropObj(path, 21.0f, ceilingLampPropMeshes_[i])) {
                ++ceilingLampLoaded;
            }
        }
        if (cabinetLoaded) ++loaded;
        if (deskLoaded) ++loaded;
        if (trashBinLoaded) ++loaded;
        if (deskLampLoaded) ++loaded;
        if (cassetteLoaded) ++loaded;
        if (airVentLoaded) ++loaded;
        if (exitSignLoaded) ++loaded;
        loaded += ceilingLampLoaded;
        StartupProfileLine(L"Loaded prop meshes: " + std::to_wstring(loaded) + L"/14");
        return loaded > 0;
    }

    bool CreateShaders() {
        static const char* shader = R"(
#define BRM_ENABLE_HIGH_BLOOD 0
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 gViewProj;
    row_major float4x4 gLightViewProj;
    float4 gCameraPosTime;
    float4 gCameraDirAspect;
    float4 gLighting0;
    float4 gLighting1;
    float4 gFog0;
float4 gAO0;
float4 gPost0;
float4 gPost1;
float4 gShadow0;
float4 gShadow1;
float4 gShadow2;
float4 gMaze0;
float4 gMaze1;
float4 gTexture0;
float4 gTransition0;
    float4 gHorror0;
    float4 gSparkLight0;
float4 gSparkLight1;
    float4 gBlood0;
    float4 gBlood1;
    float4 gBlood2;
    float4 gBlood3;
    float4 gBlood4;
    float4 gBlood5;
    float4 gBlood6;
    float4 gBlood7;
    float4 gBlood8;
    float4 gAir0;
    float4 gExitLight0;
    float4 gMonsterFog0;
};

Texture2DArray gAlbedo : register(t0);
Texture2DArray gNormalHeight : register(t1);
Texture2D gShadowMap : register(t2);
Texture2D gMazeOpen : register(t3);
Texture2DArray gMaterialProps : register(t4);
Texture2D gFlashlightPattern : register(t5);
Texture2D gLampDamage : register(t6);
SamplerState gSampler : register(s0);
SamplerComparisonState gShadowSampler : register(s1);

struct VSIn
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float2 uv : TEXCOORD0;
    float material : TEXCOORD1;
};

struct VSOut
{
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3 tangent : TEXCOORD2;
    float2 uv : TEXCOORD3;
    nointerpolation float material : TEXCOORD4;
};

VSOut VSMain(VSIn input)
{
    VSOut o;
    o.worldPos = input.pos;
    o.normal = input.normal;
    o.tangent = input.tangent;
    o.uv = input.uv;
    o.material = input.material;
    if (input.material > 11.05 && input.material < 11.45)
    {
        float seed = frac(input.material * 17.37);
        float time = gCameraPosTime.w;
        float vertical = saturate(input.uv.y);
        float edge = abs(input.uv.x - 0.5) * 2.0;
        float lowSpread = smoothstep(0.18, 1.0, vertical);
        float wave = sin(time * (0.62 + seed * 0.45) + input.pos.y * 2.7 + seed * 19.0);
        float flutter = sin(time * (1.41 + seed * 0.51) + input.pos.x * 1.8 + input.pos.z * 2.1);
        o.worldPos += input.normal * (wave * (0.055 + lowSpread * 0.115) + (1.0 - edge) * flutter * 0.038);
        o.worldPos += input.tangent * (flutter * (0.035 + lowSpread * 0.070));
        o.worldPos.y += sin(time * 0.52 + seed * 11.0 + input.uv.x * 4.2) * 0.028 * (1.0 - vertical * 0.35);
    }
    o.pos = mul(float4(o.worldPos, 1.0), gViewProj);
    return o;
}

float MaterialId(float material)
{
    return clamp(floor(material), 0.0, 25.0);
}

void ShadowPS(VSOut input)
{
    float materialId = MaterialId(input.material);
    bool transparentPaper = materialId > 8.5 && materialId < 9.5 && frac(input.material) < 0.5;
    if ((materialId > 0.5 && materialId < 2.5) ||
        transparentPaper ||
        (materialId > 10.5 && materialId < 11.5) ||
        (materialId > 11.5 && materialId < 12.5) ||
        (materialId > 12.5 && materialId < 13.5) ||
        (materialId > 13.5 && materialId < 14.5) ||
        (materialId > 14.5 && materialId < 15.5) ||
        (materialId > 24.5 && materialId < 25.5))
    {
        discard;
    }
}

float3 MaterialUV(float2 uv, float material)
{
    float slice = MaterialId(material);
    return float3(uv, slice);
}

float3 BackroomsBaseColor(float3 base, float materialId)
{
    float luma = dot(base, float3(0.299, 0.587, 0.114));
    if (materialId < 0.5)
    {
        float3 classicWall = float3(0.94, 0.74, 0.29) * (0.68 + luma * 0.68);
        return saturate(lerp(base * 1.08, classicWall, 0.14));
    }
    if (materialId > 0.5 && materialId < 1.5)
    {
        float3 classicCarpet = float3(0.74, 0.62, 0.34) * (0.70 + luma * 0.58);
        return saturate(lerp(base * 1.10, classicCarpet, 0.34));
    }
    if (materialId > 1.5 && materialId < 2.5)
    {
        float3 classicCeiling = float3(0.92, 0.76, 0.30) * (0.78 + luma * 0.42);
        return saturate(lerp(base * float3(1.06, 1.00, 0.76), classicCeiling, 0.72));
    }
    return base;
}

void UnderlyingSurface(float3 worldPos, float3 normal, out float materialId, out float2 uv)
{
    float wallScale = max(gTexture0.x, 0.20);
    float floorScale = max(gTexture0.y, 0.20);
    float explicitCeilingScale = gTexture0.z;
    float2 ceilingScale = explicitCeilingScale > 0.001
        ? max(float2(explicitCeilingScale, explicitCeilingScale), float2(0.20, 0.20))
        : max(gMaze0.zw, float2(0.20, 0.20));
    if (normal.y > 0.55)
    {
        materialId = 1.0;
        uv = worldPos.xz / floorScale;
    }
    else if (normal.y < -0.55)
    {
        materialId = 2.0;
        uv = (worldPos.xz - gMaze0.xy) / ceilingScale;
    }
    else if (abs(normal.z) >= abs(normal.x))
    {
        materialId = 0.0;
        uv = float2(worldPos.x, worldPos.y) / wallScale;
    }
    else
    {
        materialId = 0.0;
        uv = float2(worldPos.z, worldPos.y) / wallScale;
    }
}

float3 UnderlyingSurfaceColor(float3 worldPos, float3 normal, out float aoMap)
{
    float materialId = 0.0;
    float2 surfaceUv = float2(0.0, 0.0);
    UnderlyingSurface(worldPos, normal, materialId, surfaceUv);
    float mipBias = materialId > 0.5 && materialId < 1.5 ? 1.15 : 0.45;
    float3 materialUv = float3(surfaceUv, materialId);
    float4 base = gAlbedo.SampleBias(gSampler, materialUv, mipBias);
    float4 pbr = gMaterialProps.SampleBias(gSampler, materialUv, mipBias);
    aoMap = saturate(pbr.r);
    return BackroomsBaseColor(base.rgb, materialId);
}

float2 AspectSoftenedBloodUv(float2 uv, float3 worldPos)
{
    float2 duvDx = ddx(uv);
    float2 duvDy = ddy(uv);
    float det = duvDx.x * duvDy.y - duvDx.y * duvDy.x;
    if (abs(det) < 0.00001)
    {
        return uv;
    }
    float3 dpDx = ddx(worldPos);
    float3 dpDy = ddy(worldPos);
    float3 dpDu = (dpDx * duvDy.y - dpDy * duvDx.y) / det;
    float3 dpDv = (-dpDx * duvDy.x + dpDy * duvDx.x) / det;
    float uMeters = length(dpDu);
    float vMeters = length(dpDv);
    float minMeters = max(min(uMeters, vMeters), 0.05);
    float2 aspect = min(float2(uMeters, vMeters) / minMeters, float2(2.65, 2.65));
    aspect = lerp(float2(1.0, 1.0), aspect, 0.42);
    return (uv - 0.5) * aspect + 0.5;
}

float2 BloodUvWorldMeters(float2 uv, float3 worldPos)
{
    float2 duvDx = ddx(uv);
    float2 duvDy = ddy(uv);
    float3 dpDx = ddx(worldPos);
    float3 dpDy = ddy(worldPos);
    float det = duvDx.x * duvDy.y - duvDx.y * duvDy.x;
    if (abs(det) < 0.00000008)
    {
        float m0 = length(dpDx) / max(length(duvDx), 0.00001);
        float m1 = length(dpDy) / max(length(duvDy), 0.00001);
        float fallback = clamp(max(m0, m1), 0.08, 4.0);
        return float2(fallback, fallback);
    }
    float3 dpDu = (dpDx * duvDy.y - dpDy * duvDx.y) / det;
    float3 dpDv = (-dpDx * duvDy.x + dpDy * duvDx.x) / det;
    return clamp(float2(length(dpDu), length(dpDv)), float2(0.05, 0.05), float2(4.0, 4.0));
}

float Hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float Hash31(float3 p)
{
    p = frac(p * float3(127.1, 311.7, 74.7));
    p += dot(p, p.yzx + 19.19);
    return frac((p.x + p.y) * p.z);
}

float Len2Inf(float2 v)
{
    v = abs(v);
    return max(v.x, v.y);
}

float Noise3(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    float3 u = f * f * (3.0 - 2.0 * f);
    float n000 = Hash31(i + float3(0.0, 0.0, 0.0));
    float n100 = Hash31(i + float3(1.0, 0.0, 0.0));
    float n010 = Hash31(i + float3(0.0, 1.0, 0.0));
    float n110 = Hash31(i + float3(1.0, 1.0, 0.0));
    float n001 = Hash31(i + float3(0.0, 0.0, 1.0));
    float n101 = Hash31(i + float3(1.0, 0.0, 1.0));
    float n011 = Hash31(i + float3(0.0, 1.0, 1.0));
    float n111 = Hash31(i + float3(1.0, 1.0, 1.0));
    float nx00 = lerp(n000, n100, u.x);
    float nx10 = lerp(n010, n110, u.x);
    float nx01 = lerp(n001, n101, u.x);
    float nx11 = lerp(n011, n111, u.x);
    float nxy0 = lerp(nx00, nx10, u.y);
    float nxy1 = lerp(nx01, nx11, u.y);
    return lerp(nxy0, nxy1, u.z);
}

float Fbm3(float3 p)
{
    float v = 0.0;
    float a = 0.52;
    [loop]
    for (int i = 0; i < 5; ++i)
    {
        v += Noise3(p) * a;
        p = p * 2.07 + float3(17.1, 31.7, 11.3);
        a *= 0.50;
    }
    return v;
}

float SmokeFbm(float3 p, float seed, float time)
{
    p = p * 0.60 + float3(seed * 17.0, seed * 7.0, seed * 13.0);
    float v = Noise3(p);
    p *= 0.30;
    v = lerp(v, Noise3(p + time * float3(0.11, -0.31, 0.07)), 0.70);
    p *= 0.30;
    v = lerp(v, Noise3(p - time * float3(0.07, 0.23, 0.13)), 0.70);
    return v;
}

float BlackSmokeDensity(float3 p, float seed, float time)
{
    float3 uvw = p;
    float3 lmn = (p + 1.0) * 63.5;
    float t = time * (0.055 + seed * 0.010) + 32.0;

    float d2 = SmokeFbm(float3(0.6, 0.3, 0.6) * lmn + float3(0.0, 8.0 * t, 0.0), seed, time);
    float phase = d2 * 6.2831853;
    float d1 = SmokeFbm(0.3 * lmn + float3(0.0, 4.0 * t, 0.0) +
        5.0 * float3(cos(phase), 2.0 * d2, sin(phase)), seed + 0.37, time);
    d1 = pow(max(d1, 0.0001), lerp(4.0, 12.0, smoothstep(0.56, 1.0, Len2Inf(uvw.xz))));

    float density = 0.18 * smoothstep(0.0, 0.02, d1) +
        0.50 * smoothstep(0.02, 0.08, d1) +
        0.18 * smoothstep(0.08, 1.0, d1);
    float radialFade = 1.0 - smoothstep(0.68, 1.18, length(p.xz * float2(0.92, 1.04)));
    float verticalFade = smoothstep(-1.10, -0.74, p.y) * (1.0 - smoothstep(0.86, 1.18, p.y));
    float boxFade = 1.0 - smoothstep(0.90, 1.20, max(Len2Inf(p.xz), abs(p.y) * 0.82));
    return saturate(density * radialFade * verticalFade * boxFade);
}

float2 Rotate2(float2 p, float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2(p.x * c - p.y * s, p.x * s + p.y * c);
}

)" R"(
float BloodShape(float2 uv, float3 worldPos, float3 normal, float seed, out float drips, out float thickness)
{
    drips = 0.0;
    thickness = 0.0;
    if (uv.x <= 0.001 || uv.x >= 0.999 || uv.y <= 0.001 || uv.y >= 0.999)
    {
        return 0.0;
    }

    float wallMask = saturate(1.0 - abs(normal.y));
    float ceilingMask = smoothstep(0.45, 0.90, -normal.y);
    float2 local = uv * 2.0 - 1.0;
    float angle = seed * 6.2831853 + Hash21(floor(worldPos.xz * 0.19 + seed * 17.0)) * 0.75;
    float2 q = Rotate2(local, angle);
    float3 volumeP = float3(q * (2.75 + seed * 1.20), seed * 21.7 + dot(worldPos, float3(0.071, 0.137, 0.093)));
    float largeNoise = Fbm3(volumeP * 1.95);
    float midNoise = Fbm3(volumeP * 6.40 + 9.1);
    float fineNoise = Fbm3(volumeP * 18.5 + 31.0);

    float2 axis = float2(0.74 + Hash21(float2(seed, 2.1)) * 0.30, 0.48 + Hash21(float2(seed, 5.7)) * 0.22);
    float radial = length(q / axis);
    float volumeField = 0.70 - radial * 1.55 + (largeNoise - 0.5) * 0.42 + (midNoise - 0.5) * 0.28;
    float impact = smoothstep(0.20, 0.48, volumeField) * (1.0 - smoothstep(0.15, 0.70, radial));
    float tornHoles = smoothstep(0.48, 0.78, fineNoise + radial * 0.30);
    impact *= lerp(0.72, 0.18, tornHoles);

    float2 sprayDir = normalize(float2(cos(angle + Hash21(float2(seed, 12.7)) * 1.65),
                                      sin(angle + Hash21(float2(seed, 14.9)) * 1.65)));
    float satellites = 0.0;
    float tinyMist = 0.0;
    [loop]
    for (int i = 0; i < 76; ++i)
    {
        float fi = (float)i;
        float chooseDir = Hash21(float2(seed * 29.0 + fi, 11.0));
        float a = fi * 2.39996 + seed * 8.3 + Hash21(float2(fi, seed * 17.0)) * 1.1;
        float2 radialDir = float2(cos(a), sin(a));
        float2 dir = normalize(lerp(radialDir, sprayDir, step(0.54, chooseDir) * (0.30 + Hash21(float2(fi, seed * 43.0)) * 0.55)));
        float r = 0.10 + pow(Hash21(float2(seed * 23.0 + fi, 3.0)), 0.52) * 1.02;
        float2 c = dir * r;
        c += (float2(Hash21(float2(fi, seed * 41.0)), Hash21(float2(seed * 53.0, fi))) - 0.5) * 0.22;
        float2 p = local - c;
        float dropletRand = Hash21(c + seed);
        float sizeRand = pow(dropletRand, 2.65);
        float dotRadius = 0.0038 + sizeRand * 0.040;
        dotRadius *= lerp(0.62, 1.0, step(0.88, Hash21(float2(fi, seed + 88.0))));
        float stretch = 0.78 + Hash21(c + 12.0) * 1.18;
        stretch = lerp(stretch, 1.65 + Hash21(c + 51.0) * 2.10, step(0.74, chooseDir));
        float2 pr = Rotate2(p, atan2(dir.y, dir.x));
        pr.x /= stretch;
        float spot = exp(-dot(pr, pr) / max(0.00012, dotRadius * dotRadius));
        float breakup = smoothstep(0.22, 0.62, Fbm3(float3(pr * 28.0, seed * 14.0 + fi)));
        float satellite = spot * lerp(0.36, 0.92, breakup);
        satellites = max(satellites, satellite);
        tinyMist = max(tinyMist, spot * (1.0 - smoothstep(0.011, 0.046, dotRadius)) * lerp(0.46, 1.0, breakup));
    }

    float streaks = 0.0;
    [loop]
    for (int k = 0; k < 16; ++k)
    {
        float fk = (float)k;
        float armAngle = atan2(sprayDir.y, sprayDir.x) + (Hash21(float2(fk, seed * 9.0)) - 0.5) * (1.05 + Hash21(float2(seed, fk + 3.0)) * 1.7);
        float2 dir = float2(cos(armAngle), sin(armAngle));
        float2 origin = (float2(Hash21(float2(seed * 13.0, fk)), Hash21(float2(fk, seed * 19.0))) - 0.5) * 0.28;
        float2 p = local - origin;
        float along = dot(p, dir);
        float across = abs(dot(local, float2(-dir.y, dir.x)));
        float len = 0.22 + Hash21(float2(seed + 4.0, fk)) * 0.72;
        float width = 0.0035 + Hash21(float2(seed * 7.0, fk + 5.0)) * 0.013;
        float gate = smoothstep(0.010, 0.060, along) * (1.0 - smoothstep(len, len + 0.11, along));
        float strand = exp(-(across * across) / max(0.00005, width * width)) * gate;
        float breakup = smoothstep(0.20, 0.68, Fbm3(float3(p * (12.0 + fk), seed * 19.0 + fk)));
        float2 tip = p - dir * len;
        float tipDrop = exp(-dot(tip, tip) / max(0.00018, width * width * 7.5));
        streaks = max(streaks, max(strand * breakup, tipDrop * 0.76));
    }

    [loop]
    for (int j = 0; j < 18; ++j)
    {
        float fj = (float)j;
        float x = 0.14 + Hash21(float2(seed * 41.0 + fj, 91.0)) * 0.72;
        float top = 0.13 + Hash21(float2(seed * 13.0, fj)) * 0.42;
        float len = 0.16 + Hash21(float2(seed * 7.0, fj + 2.0)) * 0.76;
        float width = 0.0025 + Hash21(float2(seed * 31.0, fj + 4.0)) * 0.014;
        float yRel = uv.y - top;
        float fall = smoothstep(0.0, 0.026, yRel) * (1.0 - smoothstep(len, len + 0.075, yRel));
        float wander = (Fbm3(float3(fj * 3.1, yRel * 6.8, seed * 18.0)) - 0.5) * 0.060;
        float taper = lerp(0.80, 0.20, saturate(yRel / max(0.001, len)));
        float xDelta = uv.x - x - wander;
        float trail = exp(-(xDelta * xDelta) / max(0.00002, width * width * taper * taper * 5.5)) * fall;
        float broken = smoothstep(0.17, 0.56, Fbm3(float3(uv * 21.0 + fj, seed * 29.0)));
        float bead = exp(-((uv.x - x - wander) * (uv.x - x - wander)) / max(0.00015, width * width * 8.0)) *
                     exp(-((uv.y - (top + len)) * (uv.y - (top + len))) / max(0.00020, width * width * 38.0));
        float nose = max(bead * 1.38, trail * smoothstep(0.55, 1.0, saturate(yRel / max(0.001, len))));
        drips = max(drips, max(trail * broken, nose));
    }
    drips *= wallMask;

    float ceilingBlebs = ceilingMask * smoothstep(0.62, 0.88, Fbm3(float3(local * 13.0, seed * 33.0))) *
        (1.0 - smoothstep(0.24, 1.05, radial));
    float pepper = smoothstep(0.78, 0.965, Fbm3(float3(local * 73.0 + seed * 3.1, seed * 61.0))) *
        smoothstep(1.14, 0.20, radial);
    float merged = max(max(impact * 0.50, satellites * 0.96), max(streaks * 0.82, max(drips, max(ceilingBlebs * 0.54, max(tinyMist * 1.12, pepper * 0.70)))));
    float edgeNoise = Fbm3(float3(local * 13.0, seed * 39.0));
    float alpha = smoothstep(0.20, 0.48, merged + (edgeNoise - 0.5) * 0.16);
    float fullBorder = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    float wallBorder = min(min(uv.x, 1.0 - uv.x), uv.y);
    float border = lerp(fullBorder, wallBorder, wallMask);
    alpha *= smoothstep(0.006, 0.052 + edgeNoise * 0.035, border);
    thickness = saturate(impact * 0.24 + satellites * 0.42 + tinyMist * 0.22 + pepper * 0.10 + streaks * 0.28 + drips * 0.96 + ceilingBlebs * 0.34 + alpha * 0.32);
    return alpha;
}

float CenterSeepPool(float2 uv, float3 worldPos, float seed, float age, float speed, float maxRadius, out float thickness)
{
    float grow = smoothstep(0.0, 1.0, saturate(age * speed));
    float2 p = uv * 2.0 - 1.0;
    float darkCore = 0.0;
    float soakedLayer = 0.0;
    float sourceField = 0.0;
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        float fi = (float)i;
        float enabled = i == 0 ? 1.0 : step(i == 1 ? 0.54 : 0.82, Hash21(float2(seed * 61.0 + fi, 17.0)));
        float localGrow = grow * enabled;
        float2 offset = float2(0.0, 0.0);
        if (i > 0)
        {
            float a = seed * 6.2831853 + fi * 2.43 + Hash21(float2(seed * 71.0, fi + 19.0)) * 1.25;
            float r = 0.22 + Hash21(float2(seed * 73.0 + fi, 23.0)) * 0.26;
            offset = float2(cos(a), sin(a)) * r;
        }
        float2 q = p - offset;
        float angle = seed * 6.2831853 + Hash21(float2(seed * 41.0 + fi, 19.0)) * 1.2;
        q = Rotate2(q, angle);
        q *= float2(0.94 + Hash21(float2(seed * 47.0 + fi, 23.0)) * 0.14,
                    0.92 + Hash21(float2(seed * 53.0, fi + 29.0)) * 0.16);
        float radial = length(q);
        float broad = Fbm3(float3(worldPos.xz * (3.4 + seed * 1.1 + fi * 0.35) + seed * 11.0 + fi, seed * 31.0));
        float fine = Noise3(float3(worldPos.xz * (15.0 + seed * 4.0 + fi * 1.7) + seed * 7.0, seed * 71.0 + fi));
        float cellular = Fbm3(float3(worldPos.xz * (8.4 + fi * 1.5) + seed * 19.0, seed * 89.0 + fi));
        float edgeNoise = (broad - 0.5) * (0.18 + localGrow * 0.22) + (fine - 0.5) * 0.055;
        float spotScale = i == 0 ? 1.0 : (0.54 + Hash21(float2(seed * 79.0, fi + 31.0)) * 0.20);
        float radius = lerp(0.022, maxRadius * spotScale, localGrow);
        float coreEdge = (broad - 0.5) * 0.028 + (fine - 0.5) * 0.012;
        float coreGrow = smoothstep(0.0, 1.0, saturate(age * speed * (0.58 + fi * 0.08)));
        float coreNoise = (cellular - 0.5) * (0.020 + coreGrow * 0.035) + coreEdge;
        float core = 1.0 - smoothstep(radius * (0.070 + coreGrow * 0.040) + coreNoise,
            radius * (0.18 + coreGrow * 0.145) + coreNoise, radial);
        float front = 1.0 - smoothstep(radius * 0.36 + edgeNoise,
            radius + 0.34 + edgeNoise, radial);
        float soakMask = smoothstep(0.20, 0.82, broad + (cellular - 0.5) * 0.44 + (fine - 0.5) * 0.14);
        float feather = smoothstep(0.03, 0.82, front);
        float capillary = smoothstep(0.64, 0.93, cellular + fine * 0.10) *
            (1.0 - smoothstep(radius * 0.70, radius + 0.48, radial + (broad - 0.5) * 0.12));
        darkCore = max(darkCore, core * enabled);
        sourceField = max(sourceField, front * enabled);
        soakedLayer = max(soakedLayer, max(feather * soakMask, capillary * 0.45) * enabled);
    }
    float fibers = smoothstep(0.46, 0.90, Fbm3(float3(worldPos.xz * 18.0 + seed * 13.0, seed * 79.0)));
    float dryBreak = smoothstep(0.58, 0.91, Fbm3(float3(worldPos.xz * 10.5 + seed * 23.0, seed * 101.0)));
    float mottledSoak = sourceField * (0.24 + soakedLayer * 0.36) + soakedLayer * (0.88 + fibers * 0.20);
    float sharedSoak = smoothstep(0.13, 0.64, mottledSoak);
    sharedSoak *= 1.0 - dryBreak * (0.28 + grow * 0.18);
    float shape = max(darkCore * 0.94, sharedSoak * (0.30 + fibers * 0.16));
    float border = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
    shape *= smoothstep(0.004, 0.032, border) * smoothstep(0.02, 0.55, grow);
    thickness = saturate(darkCore * (0.78 + grow * 0.14) + sharedSoak * 0.12);
    return shape;
}

)" R"(
float FleshMaterialEligible(float materialId)
{
    return (gHorror0.x > 0.01 && materialId < 2.5) ? 1.0 : 0.0;
}

float LampDamageAtWorld(float2 worldXZ)
{
    int2 tile = (int2)floor((worldXZ - gMaze0.xy) / gMaze0.zw);
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return 0.0;
    }
    return gLampDamage.Load(int3(tile, 0)).r;
}

float LampFailureMultiplier(float damage, float seed, float time)
{
    if (damage >= 0.995)
    {
        return 0.0;
    }
    float fail = smoothstep(0.06, 0.96, damage);
    float tickRate = lerp(2.2, 48.0, fail);
    float tick = floor(time * tickRate + seed * 19.0);
    float eventChance = lerp(0.98, 0.48, fail);
    float dropoutEvent = step(eventChance, Hash21(float2(seed * 37.0 + tick, seed * 11.0 + 211.0)));
    float violent = 0.22 + 1.38 * saturate(sin(time * (42.0 + seed * 91.0)) * 0.5 + 0.5);
    float pulse = lerp(1.0, violent, fail);
    pulse *= lerp(1.0, 0.035, dropoutEvent * fail);
    float brownout = lerp(1.0, 0.24, fail * fail);
    return max(0.0, pulse * brownout);
}

struct HSConstData
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSConstData HSPatchConstants(InputPatch<VSOut, 3> patch, uint patchId : SV_PrimitiveID)
{
    HSConstData o;
    float materialId = MaterialId(patch[0].material);
    float eligible = FleshMaterialEligible(materialId);
    float maxTess = lerp(10.0, 24.0, saturate(gHorror0.w / 0.22));
    float tess = eligible > 0.5 ? floor(maxTess * saturate(gHorror0.x) + 0.5) : 1.0;
    tess = max(tess, 1.0);
    o.edges[0] = tess;
    o.edges[1] = tess;
    o.edges[2] = tess;
    o.inside = tess;
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatchConstants")]
VSOut HSMain(InputPatch<VSOut, 3> patch, uint i : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    return patch[i];
}

[domain("tri")]
VSOut DSMain(HSConstData input, const OutputPatch<VSOut, 3> patch, float3 bary : SV_DomainLocation)
{
    VSOut o;
    float3 worldPos = patch[0].worldPos * bary.x + patch[1].worldPos * bary.y + patch[2].worldPos * bary.z;
    float3 normal = normalize(patch[0].normal * bary.x + patch[1].normal * bary.y + patch[2].normal * bary.z);
    float3 tangent = normalize(patch[0].tangent * bary.x + patch[1].tangent * bary.y + patch[2].tangent * bary.z);
    float2 rawUv = patch[0].uv * bary.x + patch[1].uv * bary.y + patch[2].uv * bary.z;
    float material = patch[0].material;
    float materialId = MaterialId(material);
    float eligible = FleshMaterialEligible(materialId);
    if (eligible > 0.5)
    {
        float2 fleshUv = rawUv * 0.72 + float2(Hash21(floor(worldPos.xz * 0.27)), Hash21(floor(worldPos.zx * 0.31))) * 0.23;
        float height = gNormalHeight.SampleLevel(gSampler, float3(fleshUv, 15.0), 0).a;
        float lowHeight = gNormalHeight.SampleLevel(gSampler, float3(fleshUv, 15.0), 4).a;
        float foldRelief = (height - lowHeight) * 2.15 + (lowHeight - 0.50) * 0.42;
        float wallEdgeFade = 1.0;
        if (abs(normal.y) < 0.35)
        {
            wallEdgeFade = smoothstep(0.035, 0.24, worldPos.y) * smoothstep(0.035, 0.24, gMaze1.z - worldPos.y);
        }
        float displacement = foldRelief * gHorror0.w * (1.35 + gHorror0.x * 0.95) * wallEdgeFade;
        displacement = clamp(displacement, -0.13, 0.22);
        worldPos += normal * displacement;
    }
    o.worldPos = worldPos;
    o.normal = normal;
    o.tangent = tangent;
    o.uv = rawUv;
    o.material = material;
    o.pos = mul(float4(worldPos, 1.0), gViewProj);
    return o;
}

float FixturePower(float3 worldPos, float time)
{
    float strideTiles = max(1.0, floor(gLighting0.w / max(0.001, gMaze1.w) + 0.5));
    float2 stride = gMaze0.zw * strideTiles;
    float2 lampOrigin = gMaze0.xy + gMaze0.zw * 1.5;
    float2 cell = floor((worldPos.xz - lampOrigin) / stride + 0.5);
    float brokenZone = step(1.0 - gLighting1.w, Hash21(floor(cell / 3.0)));
    float h = Hash21(cell);
    float variation = lerp(0.86, 1.14, Hash21(cell + 53.0));
    float isOn = step(1.0 - gLighting1.y, h);
    float flickerFixture = step(1.0 - gLighting1.z, Hash21(cell + 17.0));
    float tick = floor(time * (1.3 + Hash21(cell + 37.0) * 2.5));
    float event = step(0.86, Hash21(cell + tick + 71.0));
    float flutter = 0.18 + 0.82 * saturate(sin(time * (41.0 + h * 50.0)) * 0.5 + 0.5);
    float buzz = 0.98 + 0.02 * sin(time * (55.0 + h * 80.0));
    float basePower = (1.0 - brokenZone) * isOn * lerp(1.0, flutter, flickerFixture * event) * buzz * variation;
    return basePower * LampFailureMultiplier(LampDamageAtWorld(worldPos.xz), h, time);
}

float LampVisualPower(float material, float3 worldPos, float time)
{
    float seed = frac(material);
    float variation = lerp(0.86, 1.14, frac(seed * 23.71 + 0.31));
    float flickerFixture = step(1.0 - gLighting1.z, frac(seed * 37.31 + 0.17));
    float tick = floor(time * (1.3 + seed * 2.5));
    float event = step(0.86, frac(seed * 19.17 + tick * 0.331));
    float flutter = 0.18 + 0.82 * saturate(sin(time * (41.0 + seed * 50.0)) * 0.5 + 0.5);
    float buzz = 0.98 + 0.02 * sin(time * (55.0 + seed * 80.0));
    float basePower = lerp(1.0, flutter, flickerFixture * event) * buzz * variation;
    return basePower * LampFailureMultiplier(LampDamageAtWorld(worldPos.xz), seed, time);
}

float2 MazeTile(float2 worldXZ)
{
    return floor((worldXZ - gMaze0.xy) / gMaze0.zw);
}

float MazeOpenAt(int2 tile)
{
    if (tile.x < 0 || tile.y < 0 || tile.x >= (int)gMaze1.x || tile.y >= (int)gMaze1.y)
    {
        return 0.0;
    }
    return gMazeOpen.Load(int3(tile, 0)).r;
}

float LampRayClear(float2 startXZ, float2 endXZ);

float NearestClosedCellDistance(float2 cell)
{
    int2 baseTile = (int2)floor(cell);
    float best = 4.0;

    [loop]
    for (int yy = -1; yy <= 1; ++yy)
    {
        [loop]
        for (int xx = -1; xx <= 1; ++xx)
        {
            int2 tile = baseTile + int2(xx, yy);
            if (MazeOpenAt(tile) < 0.5)
            {
                float2 lo = (float2)tile;
                float2 hi = lo + 1.0;
                float2 outside = max(max(lo - cell, cell - hi), 0.0);
                best = min(best, length(outside));
            }
        }
    }

    return best;
}

float2 LampAreaSample(int index)
{
    if (index == 0) return float2(0.0, 0.0);
    if (index == 1) return float2(0.66, 0.0);
    if (index == 2) return float2(-0.66, 0.0);
    if (index == 3) return float2(0.0, 0.58);
    if (index == 4) return float2(0.0, -0.58);
    if (index == 5) return float2(0.48, 0.42);
    if (index == 6) return float2(-0.48, 0.42);
    if (index == 7) return float2(0.48, -0.42);
    return float2(-0.48, -0.42);
}

float LampAreaRayVisibility(float2 startXZ, float2 lampXZ, float2 dir, float2 perp, float tileSize, float distFade)
{
    float sourceWidth = tileSize * lerp(0.18, 0.58, distFade);
    float sourceLength = tileSize * lerp(0.10, 0.30, distFade);
    float receiverWidth = tileSize * lerp(0.040, 0.210, distFade);
    float vis = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int i = 0; i < 9; ++i)
    {
        float2 s = LampAreaSample(i);
        float diagonal = step(0.82, abs(s.x) + abs(s.y));
        float center = 1.0 - step(0.01, dot(s, s));
        float weight = lerp(1.0, 0.72, diagonal) + center * 0.38;
        float2 source = lampXZ + perp * (s.x * sourceWidth) + dir * (s.y * sourceLength);
        float2 receiver = startXZ - perp * (s.x * receiverWidth * 0.42) - dir * (s.y * receiverWidth * 0.18);
        vis += LampRayClear(receiver, source) * weight;
        weightSum += weight;
    }

    return vis / max(weightSum, 0.001);
}

float LampVisibility(float2 worldXZ, float3 worldN, float2 lampXZ)
{
    float tileSize = gMaze1.w;
    float2 startXZ = worldXZ + worldN.xz * tileSize * 0.16;
    float2 delta = lampXZ - startXZ;
    float dist = max(length(delta), 0.001);
    float2 dir = delta / dist;
    float2 perp = float2(-dir.y, dir.x);
    float distFade = saturate(dist / max(tileSize * 4.5, 0.001));
    float2 deltaCells = abs(delta / max(tileSize, 0.001));
    float hallFill = max(exp2(-deltaCells.x * deltaCells.x * 1.95),
                         exp2(-deltaCells.y * deltaCells.y * 1.95));
    float localFill = exp2(-dot(deltaCells, deltaCells) * 0.36);
    float areaFill = saturate(max(hallFill * 0.95, localFill * 0.58));

    float2 startCell = (startXZ - gMaze0.xy) / gMaze0.zw;
    float clearance = NearestClosedCellDistance(startCell);
    float cornerFeather = smoothstep(0.045, 0.44 + distFade * 0.20, clearance);

    float rayArea = LampAreaRayVisibility(startXZ, lampXZ, dir, perp, tileSize, distFade);
    float raySoft = smoothstep(-0.04, 1.04, rayArea);
    raySoft = saturate(raySoft + rayArea * (1.0 - rayArea) * (0.18 + distFade * 0.12));
    float occludedBounce = 0.10 + cornerFeather * 0.12 + distFade * 0.045;
    float occlusion = lerp(occludedBounce, 1.0, raySoft);
    return saturate(areaFill * occlusion * (0.58 + cornerFeather * 0.42));
}

float LampRayClear(float2 startXZ, float2 endXZ)
{
    float2 startCell = (startXZ - gMaze0.xy) / gMaze0.zw;
    float2 endCell = (endXZ - gMaze0.xy) / gMaze0.zw;
    int2 tile = (int2)floor(startCell);
    int2 endTile = (int2)floor(endCell);
    if (MazeOpenAt(tile) < 0.5 || MazeOpenAt(endTile) < 0.5)
    {
        return 0.0;
    }

    float2 ray = endCell - startCell;
    float2 safeRay = float2(
        abs(ray.x) < 0.0001 ? (ray.x < 0.0 ? -0.0001 : 0.0001) : ray.x,
        abs(ray.y) < 0.0001 ? (ray.y < 0.0 ? -0.0001 : 0.0001) : ray.y);
    int2 stepTile = int2(ray.x >= 0.0 ? 1 : -1, ray.y >= 0.0 ? 1 : -1);
    float2 absRay = max(abs(ray), float2(0.0001, 0.0001));
    float2 nextBoundary = float2(
        stepTile.x > 0 ? (float)tile.x + 1.0 : (float)tile.x,
        stepTile.y > 0 ? (float)tile.y + 1.0 : (float)tile.y);
    float2 tMax = abs((nextBoundary - startCell) / safeRay);
    float2 tDelta = 1.0 / absRay;

    [loop]
    for (int i = 0; i < 96; ++i)
    {
        if (tile.x == endTile.x && tile.y == endTile.y)
        {
            return 1.0;
        }
        if (tMax.x < tMax.y)
        {
            tile.x += stepTile.x;
            tMax.x += tDelta.x;
        }
        else
        {
            tile.y += stepTile.y;
            tMax.y += tDelta.y;
        }
        if (MazeOpenAt(tile) < 0.5)
        {
            return 0.0;
        }
    }
    return 0.0;
}

)" R"(
float LocalLampLight(float3 worldPos, float3 worldN, float time)
{
    float strideTiles = max(1.0, floor(gLighting0.w / max(0.001, gMaze1.w) + 0.5));
    float2 stride = gMaze0.zw * strideTiles;
    float spacing = gMaze1.w * strideTiles;
    float2 lampOrigin = gMaze0.xy + gMaze0.zw * 1.5;
    float2 baseCell = floor((worldPos.xz - lampOrigin) / stride + 0.5);
    float light = 0.0;

    [loop]
    for (int yy = -1; yy <= 1; ++yy)
    {
        [loop]
        for (int xx = -1; xx <= 1; ++xx)
        {
            float2 cell = baseCell + float2(xx, yy);
            float2 lampXZ = lampOrigin + cell * stride;
            if (MazeOpenAt((int2)MazeTile(lampXZ)) < 0.5)
            {
                continue;
            }
            float3 lampPos = float3(lampXZ.x, gMaze1.z - 0.09, lampXZ.y);
            float3 L = lampPos - worldPos;
            float d2 = dot(L, L);
            float distXZ = length(lampPos.xz - worldPos.xz);
            float reach = spacing * 1.18 + gMaze1.w * 0.75;
            float roomFootprint = 1.0 - smoothstep(reach * 0.48, reach, distXZ);
            float power = FixturePower(lampPos, time);
            if (roomFootprint <= 0.002 || power <= 0.002)
            {
                continue;
            }
            float visibility = LampVisibility(worldPos.xz, worldN, lampPos.xz);
            float3 Ln = normalize(L);
            float diffuse = saturate(dot(worldN, Ln) * 0.65 + 0.35);
            float falloff = visibility * roomFootprint / (1.0 + d2 * 0.035);
            light += power * falloff * diffuse;
        }
    }

    return light;
}

float CornerAO(float3 worldPos, float3 normal)
{
    float radius = max(0.02, gAO0.y);
    float floorContact = 1.0 - smoothstep(0.0, radius * 1.20, worldPos.y);
    float ceilingContact = 1.0 - smoothstep(0.0, radius * 1.20, gMaze1.z - worldPos.y);
    float verticalContact = max(floorContact, ceilingContact);
    float wallMask = saturate(1.0 - abs(normal.y));
    float ao = wallMask * verticalContact * gAO0.z;
    return saturate(ao * gAO0.x);
}

float ShadowVisibility(float3 worldPos, float3 normal)
{
    if (gShadow0.w <= 0.001)
    {
        return 1.0;
    }

    float3 samplePos = worldPos + normalize(normal) * 0.018;
    float4 lightPos = mul(float4(samplePos, 1.0), gLightViewProj);
    if (lightPos.w <= 0.0001)
    {
        return 1.0;
    }

    float3 ndc = lightPos.xyz / lightPos.w;
    float2 uv = float2(ndc.x * 0.5 + 0.5, -ndc.y * 0.5 + 0.5);
    if (uv.x <= 0.001 || uv.x >= 0.999 || uv.y <= 0.001 || uv.y >= 0.999 || ndc.z <= 0.0 || ndc.z >= 1.0)
    {
        return 1.0;
    }

    float3 lightPosWorld = gShadow0.xyz;
    float3 toLight = normalize(lightPosWorld - samplePos);
    float slope = 1.0 - saturate(dot(normalize(normal), toLight));
    float compareDepth = ndc.z - max(gShadow1.w * (1.0 + slope * 2.4), 0.00045);
    float texel = max(gShadow2.x, 0.0001);
    float receiver = saturate(length(worldPos - lightPosWorld) / max(gShadow2.y, 0.01));
    float penumbra = pow(receiver, 0.72);
    float filterRadius = lerp(0.85, 5.20, penumbra);
    float2 stableCell = floor(worldPos.xz * 3.1 + worldPos.y * 1.7);
    float2 jitter = (float2(Hash21(stableCell + 13.0), Hash21(stableCell + 71.0)) - 0.5) * texel * lerp(0.16, 0.42, penumbra);
    float visible = 0.0;
    float weightSum = 0.0;

    [loop]
    for (int y = -2; y <= 2; ++y)
    {
        [loop]
        for (int x = -2; x <= 2; ++x)
        {
            float wx = 3.0 - abs((float)x);
            float wy = 3.0 - abs((float)y);
            float weight = wx * wy;
            float2 offset = float2(x, y) * texel * filterRadius + jitter;
            visible += gShadowMap.SampleCmpLevelZero(gShadowSampler, uv + offset, compareDepth) * weight;
            weightSum += weight;
        }
    }
    visible /= max(weightSum, 0.001);
    visible = saturate(visible + (1.0 - visible) * penumbra * 0.035);
    float strength = gShadow0.w * lerp(1.0, 0.74, penumbra);
    return lerp(1.0 - strength, 1.0, visible);
}

float FlashlightAmount(float3 worldPos, float3 worldN)
{
    float3 lightPos = gShadow0.xyz;
    float3 lightDir = normalize(gShadow1.xyz);
    float3 fromLight = worldPos - lightPos;
    float dist = length(fromLight);
    if (dist <= 0.001)
    {
        return 0.0;
    }

    float3 ray = fromLight / dist;
    float3 toLight = -ray;
    float cone = smoothstep(gShadow2.z, gShadow2.w, dot(ray, lightDir));
    float attenuation = 1.0 / (1.0 + gLighting0.y * dist * dist);
    float diffuse = saturate(dot(worldN, toLight));
    float shadow = ShadowVisibility(worldPos, worldN);
    float4 lightClip = mul(float4(worldPos, 1.0), gLightViewProj);
    float2 lightUv = lightClip.xy / max(lightClip.w, 0.0001) * 0.5 + 0.5;
    float glass = gFlashlightPattern.Sample(gSampler, saturate(lightUv)).r;
    float2 centered = lightUv * 2.0 - 1.0;
    float lensFalloff = smoothstep(1.18, 0.18, dot(centered, centered));
    float pattern = lerp(0.52, 1.22, glass) * (0.72 + lensFalloff * 0.28);
    return cone * attenuation * gLighting0.x * (0.16 + diffuse * 1.12) * shadow * pattern;
}

float SparkLightOne(float3 worldPos, float3 worldN, float4 lightData)
{
    if (lightData.w <= 0.001)
    {
        return 0.0;
    }
    float3 L = lightData.xyz - worldPos;
    float d = length(L);
    if (d <= 0.001 || d > 5.0)
    {
        return 0.0;
    }
    float visibility = LampRayClear(worldPos.xz + worldN.xz * 0.05, lightData.xz);
    float3 Ln = L / d;
    float diffuse = saturate(dot(worldN, Ln) * 0.72 + 0.28);
    float falloff = pow(saturate(1.0 - d / 5.0), 2.1) / (1.0 + d * d * 0.42);
    return lightData.w * falloff * diffuse * visibility;
}

float SparkLight(float3 worldPos, float3 worldN)
{
    return SparkLightOne(worldPos, worldN, gSparkLight0) + SparkLightOne(worldPos, worldN, gSparkLight1);
}

float3 ExitSignLight(float3 worldPos, float3 worldN)
{
    float strength = gExitLight0.w * (1.0 - saturate(gTransition0.z));
    if (strength <= 0.001)
    {
        return float3(0.0, 0.0, 0.0);
    }
    float3 L = gExitLight0.xyz - worldPos;
    float d = length(L);
    if (d <= 0.001 || d > 3.65)
    {
        return float3(0.0, 0.0, 0.0);
    }
    float visibility = LampRayClear(worldPos.xz + worldN.xz * 0.035, gExitLight0.xz);
    float3 Ln = L / d;
    float diffuse = saturate(dot(worldN, Ln) * 0.72 + 0.28);
    float falloff = pow(saturate(1.0 - d / 3.65), 2.0) / (1.0 + d * d * 0.62);
    return float3(0.045, 0.92, 0.34) * strength * falloff * diffuse * visibility;
}

float3 ApplyPost(float3 color)
{
    color = max(color, 0.0);
    color = 1.0 - exp(-color * gPost0.x);
    color = pow(saturate(color), 1.0 / max(gPost0.y, 0.1));
    float danger = saturate(gPost0.z);
    float death = saturate(gPost0.w);
    float time = gCameraPosTime.w;
    float blinkRate = 5.0 + danger * 14.0 + death * 36.0;
    float blinkPhase = frac(time * blinkRate + sin(time * 17.13) * 0.17 + sin(time * 43.7) * 0.07);
    float blinkGate = step(0.78 - danger * 0.28 - death * 0.42, blinkPhase);
    float blackout = saturate(blinkGate * (danger * danger * 0.30 + death * 0.62));
    color *= 1.0 - blackout;
    color = lerp(color, float3(0.0, 0.0, 0.0), smoothstep(0.76, 1.0, death));
    color = lerp(color, float3(0.0, 0.0, 0.0), saturate(gTransition0.x));
    return color;
}

float DistanceFogBlock(float dist, float scale)
{
    float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
    fog = 1.0 - exp(-fog * fog * 3.2);
    return saturate(fog * gFog0.z * scale);
}

float MonsterVicinityFog(float3 worldPos)
{
    float radius = max(gMonsterFog0.z, 0.001);
    float nearMonster = saturate(1.0 - length(worldPos.xz - gMonsterFog0.xy) / radius);
    return saturate(nearMonster * gMonsterFog0.w);
}

float SceneFogBlock(float dist, float3 worldPos, float scale)
{
    float distanceFog = DistanceFogBlock(dist, scale);
    float monsterFog = MonsterVicinityFog(worldPos);
    return saturate(distanceFog + monsterFog * (1.0 - distanceFog * 0.35));
}

void SelectBloodRevealSlot(float4 slot, float active, float2 worldXZ, float time, inout float bestMask, inout float bestAge)
{
    float enabled = saturate(active) * step(0.001, slot.w);
    if (enabled <= 0.001)
    {
        return;
    }
    float dist = length(worldXZ - slot.xy);
    float mask = 1.0 - smoothstep(slot.w * 0.72, slot.w, dist);
    float age = max(0.0, time - slot.z);
    float score = mask * smoothstep(0.0, 0.22, age) * enabled;
    if (score > bestMask)
    {
        bestMask = score;
        bestAge = age;
    }
}

)" R"(
float4 PSMain(VSOut input) : SV_TARGET
{
    float materialId = MaterialId(input.material);
    float3 cam = gCameraPosTime.xyz;
    float time = gCameraPosTime.w;
    float3 V = normalize(cam - input.worldPos);
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    float3 B = normalize(cross(N, T));
    float2 rawUv = input.uv;
    float2 uv = frac(rawUv);

    if ((materialId > 13.5 && materialId < 14.5) || (materialId > 24.5 && materialId < 25.5))
    {
        float waterLiquid = step(24.5, materialId);
        float rawSeed = frac(input.material);
        float wallLeakSurface = step(0.96, rawSeed);
        float centerSeepSurface = step(0.43, rawSeed) * (1.0 - step(0.95, rawSeed));
        float seed = rawSeed;
        if (wallLeakSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.965) / 0.025);
        }
        else if (centerSeepSurface > 0.5)
        {
            seed = saturate((rawSeed - 0.43) / 0.52);
        }
        else if (rawSeed >= 0.02 && rawSeed <= 0.42)
        {
            seed = saturate((rawSeed - 0.02) / 0.40);
        }
        float wet = lerp(saturate(gHorror0.y), 1.0, waterLiquid);
        float2 bloodUv = lerp(uv, rawUv, waterLiquid);
        float2 bloodUvMeters = BloodUvWorldMeters(rawUv, input.worldPos);
        float drips = 0.0;
        float thickness = 0.0;
        float wallMask = saturate(1.0 - abs(N.y));
        float floorMask = smoothstep(0.45, 0.82, N.y);
        float ceilingMask = smoothstep(0.45, 0.82, -N.y);
        float animMask = 0.0;
        float leakAge = 0.0;
        if (waterLiquid > 0.5)
        {
            float waterDebugActive = step(1.0, gTransition0.w);
            float waterDebugPhase = frac(gTransition0.w);
            if (waterDebugActive > 0.5)
            {
                animMask = 1.0;
                leakAge = waterDebugPhase * 54.0;
            }
            else
            {
                SelectBloodRevealSlot(float4(gBlood0.xy, gBlood0.z, gBlood1.y), gBlood0.w, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood2, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood3, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood4, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood5, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood6, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood7, 1.0, input.worldPos.xz, time, animMask, leakAge);
                SelectBloodRevealSlot(gBlood8, 1.0, input.worldPos.xz, time, animMask, leakAge);
            }
        }
        else
        {
            SelectBloodRevealSlot(float4(gBlood0.xy, gBlood0.z, gBlood1.y), gBlood0.w, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood2, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood3, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood4, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood5, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood6, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood7, 1.0, input.worldPos.xz, time, animMask, leakAge);
            SelectBloodRevealSlot(gBlood8, 1.0, input.worldPos.xz, time, animMask, leakAge);
        }
        float alpha = 0.0;
        float bloodQuality = clamp(gBlood1.w, 0.25, 1.0);
        float requestedStreams = clamp(round(gBlood1.x), 4.0, 32.0);
        float streamBudget = saturate(bloodQuality * 1.15);
        float streamCount = clamp(round(lerp(8.0, requestedStreams, streamBudget)), 6.0, 28.0);
        float floorStreamCount = wallLeakSurface > 0.5
            ? streamCount
            : clamp(round(streamCount * lerp(0.36, 0.74, bloodQuality)), 2.0, streamCount);
        float ceilingStreamCount = wallLeakSurface > 0.5
            ? streamCount
            : clamp(round(streamCount * lerp(0.32, 0.70, bloodQuality)), 2.0, streamCount);
        float streamWidthScale = max(0.10, gBlood1.z) * lerp(1.55, 1.0, bloodQuality);
        static const bool highBloodDetail = BRM_ENABLE_HIGH_BLOOD != 0;

)" R"(

        if (wallMask > 0.45)
        {
            float u = bloodUv.x;
            float y = bloodUv.y;
            float wallLeakRun = wallLeakSurface;
            float wallStreamWidthScale = streamWidthScale * 1.16;
            float streams = 0.0;
            float streamAccum = 0.0;
            float sdfWet = 0.0;
            float sdfCore = 0.0;
            float diffuseSoak = 0.0;
            float beads = 0.0;
            float topSource = 0.0;
            [loop]
            for (int i = 0; i < 32; ++i)
            {
                float fi = (float)i;
                if (fi >= streamCount) break;
                float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                if (highBloodDetail)
                {
                    densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                }
                float densityBand = densityBase;
                float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                float streamDelay = r0 * 4.6 + fi * (0.05 + r2 * 0.18) +
                    Hash21(float2(seed * 251.0 + fi, 157.0)) * 1.7;
                streamDelay *= lerp(1.0, 0.62, waterLiquid);
                float streamAge = max(0.0, leakAge - streamDelay);
                float speedPhase = streamAge * (0.62 + r2 * 0.54) + seed * 17.0 + fi * 2.13;
                float flowAge = max(0.0, streamAge * (0.90 + r1 * 0.18) +
                    sin(speedPhase) * (0.10 + r0 * 0.08) +
                    sin(speedPhase * 2.17 + r2 * 6.0) * 0.032);
                flowAge *= lerp(1.0, 1.34, waterLiquid);
                float sourceReady = smoothstep(0.10 + r2 * 0.55, 0.90 + r1 * 1.05, flowAge);
                float streamGrow = smoothstep(0.0, 1.0,
                    saturate((flowAge - (0.20 + r2 * 0.75) * lerp(1.0, 0.68, waterLiquid)) *
                    (0.052 + r1 * 0.055) * lerp(1.0, 1.24, waterLiquid)));
                float reachRoll = Hash21(float2(seed * 197.0 + fi, 109.0));
                float partialReach = 0.30 + Hash21(float2(seed * 211.0 + fi, 113.0)) * 0.52;
                float fullReach = 1.02 + Hash21(float2(seed * 223.0 + fi, 127.0)) * 0.18;
                float stopLen = lerp(partialReach, fullReach, step(0.48 + r1 * 0.22, reachRoll));
                float initialLen = 0.006 + r0 * 0.036;
                float len = saturate(lerp(initialLen, stopLen, streamGrow));
                float width = (0.0026 + r2 * 0.0054) * wallStreamWidthScale;
                float lean = (r1 - 0.5) * y * (0.0025 + r2 * 0.0035);
                float microWobble = (Hash21(float2(fi * 5.1 + seed * 23.0, y * 3.0)) - 0.5) * (0.0007 + r2 * 0.0010);
                if (highBloodDetail)
                {
                    microWobble = (Fbm3(float3(y * 2.2, fi * 1.7, seed * 23.0)) - 0.5) * (0.0011 + r2 * 0.0018);
                }
                float wander = lean + microWobble;
                float du = u - center - wander;
                float trailGate = 1.0 - smoothstep(len, len + 0.060, y);
                float permanentTrace = smoothstep(0.02, 0.28, streamGrow);
                float flowEnd = 16.0 + r0 * 24.0 + r2 * 18.0;
                float flowFade = 7.0 + r1 * 12.0;
                float activeFlow = 1.0 - smoothstep(flowEnd, flowEnd + flowFade, flowAge);
                float wetPulse = 0.91 + 0.09 * activeFlow * sin(y * (24.0 + r1 * 18.0) - time * (0.18 + r2 * 0.16) + seed * 37.0);
                float widthNoise = 0.86 + 0.24 * Hash21(float2(fi * 13.7 + seed * 29.0, floor(y * 15.0)));
                if (highBloodDetail)
                {
                    widthNoise = 0.76 + 0.38 * Fbm3(float3(fi * 2.2 + seed * 19.0, y * 8.0, seed * 41.0));
                }
                float gravityBulge = smoothstep(0.16, 0.92, y) * (0.08 + r1 * 0.18) +
                    smoothstep(0.78, 1.0, y) * (0.16 + r2 * 0.18);
                float widthTaper = width * (0.78 + gravityBulge) * widthNoise;
                float strandBreaks = lerp(0.70, 1.0, Hash21(float2(fi * 9.0 + seed * 43.0, floor(y * 22.0))));
                if (highBloodDetail)
                {
                    strandBreaks = smoothstep(0.18, 0.50, Fbm3(float3(u * 35.0 + fi, y * 10.0, seed * 43.0)) + 0.18);
                }
                float stream = exp(-(du * du) / max(0.000008, widthTaper * widthTaper)) * trailGate * permanentTrace * wetPulse * strandBreaks * streamStrength;
                float bleedHalo = exp(-(du * du) / max(0.000018, widthTaper * widthTaper * 18.0)) *
                    trailGate * permanentTrace * streamStrength * 0.46;
                float sdfWidth = widthTaper * (5.8 + bloodQuality * 3.2);
                float field = saturate(1.0 - abs(du) / max(0.0008, sdfWidth)) *
                    trailGate * permanentTrace * streamStrength;
                sdfWet += field;
                sdfCore += field * field;
                float soakDelay = (3.2 + r2 * 10.0 + y * (7.0 + r1 * 12.0)) * lerp(1.0, 0.74, waterLiquid);
                float soakAge = max(0.0, flowAge - soakDelay);
                float soakGrow = smoothstep(0.0, 1.0, saturate(soakAge * (0.070 + r0 * 0.058) * lerp(1.0, 1.18, waterLiquid)));
                float soakNoise = 0.72 + 0.28 * Hash21(float2(fi * 17.0 + seed * 61.0, floor(y * 16.0) + floor(u * 9.0)));
                if (highBloodDetail)
                {
                    soakNoise = 0.52 + 0.48 * Fbm3(float3(u * 12.0 + fi * 0.37, y * 8.0, seed * 67.0));
                }
                float soakWidth = widthTaper * (8.0 + bloodQuality * 13.0) * (0.62 + soakGrow * 1.22);
                float soakGate = 1.0 - smoothstep(len - 0.035, len + 0.20, y);
                float localSoak = saturate(1.0 - abs(du) / max(0.0012, soakWidth)) *
                    soakGate * soakGrow * streamStrength * soakNoise;
                diffuseSoak += localSoak;
                float headY = len - 0.018;
                float bead = 0.0;
                if (highBloodDetail)
                {
                    bead = exp(-(du * du) / max(0.000018, width * width * 3.0)) *
                        exp(-((y - headY) * (y - headY)) / max(0.00005, width * width * 22.0)) * streamStrength;
                }
                float sourceWidth = width * (2.3 + r1 * 1.8) + 0.0032 * wallStreamWidthScale;
                float sourceDepth = 0.018 + r2 * 0.040;
                float sourceY = 0.010 + Hash21(float2(seed * 269.0 + fi, 167.0)) * 0.020;
                float sourceNoise = 0.72 + 0.28 * Hash21(float2(floor(u * 18.0) + fi, seed * 37.0));
                if (highBloodDetail)
                {
                    sourceNoise = 0.45 + 0.55 * Fbm3(float3(u * 12.0, seed * 37.0 + fi, y * 5.0));
                }
                float sourceEdge = (Fbm3(float3(u * 18.0 + fi, y * 22.0, seed * 53.0)) - 0.5) *
                    (0.18 + sourceReady * 0.16);
                float2 sourceQ = float2((u - center) / max(0.0012, sourceWidth),
                    (y - sourceY) / max(0.0012, sourceDepth * (0.62 + r0 * 0.35)));
                float sourceBlob = 1.0 - smoothstep(0.56, 1.12, dot(sourceQ, sourceQ) + sourceEdge);
                float sourceFeeder = exp(-((u - center) * (u - center)) / max(0.000035, sourceWidth * sourceWidth * 0.42)) *
                    (1.0 - smoothstep(sourceDepth * (0.80 + r2 * 0.40),
                        sourceDepth * (1.55 + r1 * 0.70), y)) *
                    smoothstep(0.45, 1.0, sourceReady) * 0.38;
                float source = max(sourceBlob, sourceFeeder) * sourceNoise * sourceReady * streamStrength;
                streams = max(streams, stream);
                streamAccum += saturate(stream + bleedHalo);
                beads = max(beads, bead * smoothstep(0.08, 0.98, streamGrow) * (1.0 - smoothstep(0.96, 1.0, streamGrow) * 0.30));
                topSource = max(topSource, source);
            }
            float wallFullHeightGate = smoothstep(0.0, 0.024, y) * (1.0 - smoothstep(0.994, 1.0, y));
            float wallSheet = smoothstep(0.16, 0.86, streamAccum) *
                wallFullHeightGate *
                smoothstep(8.0, 18.0, leakAge) * 0.46;
            float mergedField = smoothstep(0.70, 1.68, sdfWet) * wallFullHeightGate;
            float mergedCore = smoothstep(0.36, 1.20, sdfCore) * wallFullHeightGate;
            float wallEdgeNoise = (Fbm3(float3(u * 4.1, seed * 9.0, y * 2.3)) - 0.5) * 0.075;
            float wallTopFade = smoothstep(-0.004 + wallEdgeNoise * 0.10, 0.036 + wallEdgeNoise * 0.10, y);
            float wallVerticalFade = wallTopFade * (1.0 - smoothstep(0.992, 1.0, y));
            float wallCardFade = smoothstep(0.018 + wallEdgeNoise, 0.115 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.885 - wallEdgeNoise, 0.982 - wallEdgeNoise, u)) *
                wallVerticalFade;
            float floodNoise = Fbm3(input.worldPos * float3(4.7, 2.8, 4.7) + seed * 11.0);
            float floodFine = Noise3(input.worldPos * float3(18.0, 8.0, 18.0) + seed * 29.0);
            float organicFlood = smoothstep(0.20, 0.72, floodNoise + (floodFine - 0.5) * 0.18);
            float streamFedSoak = smoothstep(0.34, 1.30, diffuseSoak) * wallCardFade *
                organicFlood * (0.40 + smoothstep(0.74, 1.80, diffuseSoak) * 0.22);
            float wallFlood = saturate(streamFedSoak);
            float ceilingContactFeed = smoothstep(0.18, 0.52, topSource + streams * 0.18 + sdfWet * 0.10);
            float ceilingContact = (1.0 - smoothstep(0.0, 0.038, y)) *
                smoothstep(1.8, 7.5, leakAge) *
                ceilingContactFeed *
                smoothstep(0.015 + wallEdgeNoise, 0.090 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.910 - wallEdgeNoise, 0.990 - wallEdgeNoise, u));
            wallFlood = saturate(wallFlood + ceilingContact * 0.22);
            float floorContact = smoothstep(0.935, 1.0, y) *
                smoothstep(5.0, 12.0, leakAge) *
                smoothstep(0.06, 0.42, streams + sdfWet * 0.20 + streamAccum * 0.035 + wallFlood * 0.24) *
                smoothstep(0.015 + wallEdgeNoise, 0.090 + wallEdgeNoise, u) *
                (1.0 - smoothstep(0.910 - wallEdgeNoise, 0.990 - wallEdgeNoise, u));
            wallFlood = saturate(wallFlood + floorContact * 0.36);
            wallFlood = min(wallFlood, 0.58);
            streams = saturate(streams + wallSheet * (0.48 + bloodQuality * 0.22) +
                mergedField * (0.66 + bloodQuality * 0.24) + wallFlood * (0.36 + bloodQuality * 0.18));
            float bottomGather = smoothstep(0.90, 1.0, y) * (streams * 0.48 + topSource * 0.10) * smoothstep(4.5, 9.5, leakAge);
            float brokenEdges = 0.88 + 0.12 * Hash21(float2(floor(u * 34.0) + seed * 59.0, floor(y * 24.0)));
            if (highBloodDetail)
            {
                brokenEdges = 0.78 + 0.22 * Fbm3(float3(u * 26.0, y * 18.0, seed * 59.0));
            }
            alpha = max(max(streams * brokenEdges, beads * 1.14), max(max(topSource * 0.58, bottomGather), max(wallFlood * 0.70, max(ceilingContact * 0.72, floorContact * 0.72))));
            alpha = smoothstep(0.008, 0.046, alpha);
            drips = saturate(streams + beads + bottomGather * 0.65 + wallFlood * 0.36 + floorContact * 0.28);
            thickness = saturate(streams * 0.60 + mergedCore * 0.78 + wallFlood * 0.68 + beads * 0.82 + bottomGather * 0.46 + topSource * 0.34 + ceilingContact * 0.28 + floorContact * 0.32);
            thickness *= alpha;
            drips *= alpha;
        }

)" R"(

        else if (floorMask > 0.45)
        {
            if (centerSeepSurface > 0.5)
            {
                float centerThickness = 0.0;
                float centerSpeed = lerp(0.026, 0.017, waterLiquid);
                float centerRadius = lerp(0.56, 0.72, waterLiquid);
                float source = CenterSeepPool(bloodUv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                alpha = source * floorMask * smoothstep(0.0, lerp(0.45, 0.82, waterLiquid), leakAge);
                alpha = smoothstep(lerp(0.020, 0.012, waterLiquid), lerp(0.118, 0.090, waterLiquid), alpha);
                thickness = centerThickness * alpha;
                drips = 0.0;
            }
            else
            {
                float u = bloodUv.x;
                float away = 1.0 - bloodUv.y;
                float pooled = 0.0;
                float pooledField = 0.0;
                float wetRim = 0.0;
                float earlySoakField = 0.0;
                float lobeThickness = 0.0;
                [loop]
                for (int i = 0; i < 32; ++i)
                {
                    float fi = (float)i;
                    if (fi >= floorStreamCount) break;
                    float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                    float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                    float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                    float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                    float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                    float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                    float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                    float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                    float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                    float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                    float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                    if (highBloodDetail)
                    {
                        densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                    }
                    float densityBand = densityBase;
                    float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                    float streamDelay = wallLeakSurface > 0.5
                        ? (r0 * 3.4 + fi * (0.05 + r2 * 0.16) + Hash21(float2(seed * 251.0 + fi, 157.0)) * 1.1)
                        : (r0 * 2.2 + fi * 0.16);
                    float streamAge = max(0.0, leakAge - streamDelay);
                    float flowAge = streamAge;
                    if (wallLeakSurface > 0.5)
                    {
                        float speedPhase = streamAge * (0.62 + r2 * 0.54) + seed * 17.0 + fi * 2.13;
                        flowAge = max(0.0, streamAge * (0.90 + r1 * 0.18) +
                            sin(speedPhase) * (0.10 + r0 * 0.08) +
                            sin(speedPhase * 2.17 + r2 * 6.0) * 0.032);
                    }
                    float streamGrowRate = wallLeakSurface > 0.5
                        ? (0.074 + r1 * 0.062)
                        : (0.088 + r1 * 0.066);
                    float streamGrow = smoothstep(0.0, 1.0, saturate(flowAge * streamGrowRate));
                    float len = saturate(0.160 + streamGrow * (0.92 + r1 * 0.24));
                    float reachFloor = smoothstep(0.83, 0.98, len) * streamStrength;
                    float contact = reachFloor;
                    if (wallLeakSurface > 0.5)
                    {
                        float waterFloorHit = smoothstep(0.88, 1.00, len) * smoothstep(5.8, 9.8, flowAge);
                        float bloodFloorHit = smoothstep(0.985, 1.075, len) * smoothstep(12.0, 17.0, flowAge);
                        contact = lerp(bloodFloorHit, waterFloorHit, waterLiquid) * streamStrength;
                    }
                    float poolDelay = wallLeakSurface > 0.5
                        ? lerp(11.8 + r2 * 4.8, 3.8 + r2 * 2.4, waterLiquid)
                        : 4.8;
                    float extraDelay = r2 * (wallLeakSurface > 0.5 ? lerp(1.3, 0.45, waterLiquid) : 1.9);
                    float poolAge = max(0.0, flowAge - poolDelay - extraDelay);
                    float poolGrowRate = wallLeakSurface > 0.5 ? (0.034 + r1 * 0.030) : (0.112 + r1 * 0.056);
                    poolGrowRate *= wallLeakSurface > 0.5 ? lerp(1.0, 1.42, waterLiquid) : lerp(1.0, 0.56, waterLiquid);
                    float poolGrow = smoothstep(0.0, 1.0, saturate(poolAge * poolGrowRate)) * contact;
                    float leanAtFloor = (r1 - 0.5) * (0.0025 + r2 * 0.0035);
                    float bottomWobble = 0.0;
                    if (wallLeakSurface > 0.5)
                    {
                        bottomWobble = (Hash21(float2(fi * 5.1 + seed * 23.0, 3.0)) - 0.5) * (0.0007 + r2 * 0.0010);
                        if (highBloodDetail)
                        {
                            bottomWobble = (Fbm3(float3(2.2, fi * 1.7, seed * 23.0)) - 0.5) * (0.0011 + r2 * 0.0018);
                        }
                    }
                    float sourceU = center + leanAtFloor + bottomWobble;
                    float edgeNoise = Hash21(float2(floor(u * 12.0) + fi, floor(away * 10.0) + seed * 23.0)) - 0.5;
                    if (highBloodDetail)
                    {
                        edgeNoise = Fbm3(float3(u * 7.5 + fi, away * 4.0, seed * 23.0)) - 0.5;
                    }
                    float spreadAway = wallLeakSurface > 0.5
                        ? (0.070 + poolGrow * (1.02 + r2 * 0.58) + edgeNoise * 0.046)
                        : (0.070 + poolGrow * (0.78 + r2 * 0.48) + edgeNoise * 0.040);
                    float spreadSide = (wallLeakSurface > 0.5
                        ? (0.032 + poolGrow * (0.155 + r1 * 0.210))
                        : (0.030 + poolGrow * (0.115 + r1 * 0.180))) * lerp(0.78, 1.0, streamWidthScale);
                    spreadAway *= lerp(1.0, 1.34, waterLiquid);
                    spreadSide *= lerp(1.0, 1.18, waterLiquid);
                    float sideWorld = (u - sourceU) * bloodUvMeters.x;
                    float awayWorld = away * bloodUvMeters.y;
                    float soakRag = Fbm3(float3(input.worldPos.xz * (7.0 + r2 * 7.0) + fi * 0.37, seed * 31.0 + fi));
                    if (wallLeakSurface > 0.5)
                    {
                        float soakPoolDelay = lerp(9.4 + r2 * 3.8, 3.1 + r2 * 2.0, waterLiquid);
                        float soakPoolAge = max(0.0, flowAge - soakPoolDelay);
                        float waterBottomReached = smoothstep(0.86, 0.99, len) * smoothstep(4.8, 8.8, flowAge);
                        float bloodBottomReached = smoothstep(0.965, 1.055, len) * smoothstep(10.5, 15.5, flowAge);
                        float bottomReached = lerp(bloodBottomReached, waterBottomReached, waterLiquid);
                        float soakGrowRate = (0.038 + r1 * 0.032) * lerp(1.0, 1.30, waterLiquid);
                        float soakGrow = smoothstep(0.0, 1.0, saturate(soakPoolAge * soakGrowRate)) *
                            bottomReached * streamStrength;
                        float soakSpreadAway = 0.055 + soakGrow * (0.72 + r2 * 0.42) + edgeNoise * 0.042;
                        float soakSpreadSide = (0.030 + soakGrow * (0.170 + r1 * 0.150)) * lerp(0.82, 1.0, streamWidthScale);
                        soakSpreadAway *= lerp(1.0, 1.42, waterLiquid);
                        soakSpreadSide *= lerp(1.0, 1.18, waterLiquid);
                        float soakSideWorld = max(0.016, soakSpreadSide * bloodUvMeters.x);
                        float soakAwayWorld = max(0.018, soakSpreadAway * bloodUvMeters.x);
                        float2 soakQ = float2(sideWorld / soakSideWorld, awayWorld / soakAwayWorld);
                        float soakBreakup = (soakRag - 0.5) * (0.28 + soakGrow * 0.34);
                        float soakLayer = 1.0 - smoothstep(0.72, 1.14, dot(soakQ, soakQ) + soakBreakup);
                        soakLayer *= smoothstep(0.0, 0.16, soakGrow);
                        earlySoakField = max(earlySoakField, soakLayer * (0.24 + soakGrow * 0.18));
                    }
                    float spreadSideWorld = max(0.010, spreadSide * bloodUvMeters.x);
                    float spreadAwayWorld = max(0.012, spreadAway * bloodUvMeters.x);
                    float2 q = float2(sideWorld / spreadSideWorld, awayWorld / spreadAwayWorld);
                    float edgeBreakup = (soakRag - 0.5) * (0.34 + poolGrow * 0.42);
                    float lobe = 1.0 - smoothstep(0.70, 1.05, dot(q, q) + edgeBreakup);
                    lobe *= smoothstep(0.0, 0.14, poolGrow);
                    float feeder = exp(-(sideWorld * sideWorld) / max(0.000035, spreadSideWorld * spreadSideWorld * 0.22)) *
                        (1.0 - smoothstep(0.0, (0.135 + poolGrow * 0.075) * bloodUvMeters.x, awayWorld)) * contact;
                    float capillary = smoothstep(0.62, 0.92,
                        Fbm3(float3(input.worldPos.xz * (18.0 + r1 * 12.0) + fi, seed * 57.0))) *
                        (1.0 - smoothstep(0.72, 1.35, dot(q, q))) * smoothstep(0.12, 0.85, poolGrow) * contact * 0.22;
                    lobe = max(lobe, capillary);
                    pooled = max(pooled, lobe);
                    pooledField += saturate(lobe) * (0.62 + poolGrow * 0.32) + feeder * 0.22;
                    wetRim = max(wetRim, feeder);
                    lobeThickness = max(lobeThickness, lobe * (0.64 + poolGrow * 0.54) + feeder * 0.52);
                }
                float sdfMerge = smoothstep(0.42, 1.18, pooledField + wetRim * 0.40);
                float merged = saturate(max(max(pooled, sdfMerge), earlySoakField * 0.58) + wetRim * 0.72);
                float lateralNoise = (Hash21(float2(floor(u * 11.0) + seed * 11.0, floor(away * 8.0))) - 0.5) * 0.024;
                if (highBloodDetail)
                {
                    lateralNoise = (Fbm3(float3(u * 6.1, away * 3.7, seed * 11.0)) - 0.5) * 0.030;
                }
                float lateral = smoothstep(0.030 + lateralNoise, 0.070 + lateralNoise, u) *
                    (1.0 - smoothstep(0.930 - lateralNoise, 0.970 - lateralNoise, u));
                float farEdgeNoise = (Fbm3(float3(u * 4.7 + seed * 23.0, away * 2.1, seed * 67.0)) - 0.5) * 0.16 +
                    (Noise3(float3(u * 13.0, away * 8.0, seed * 97.0)) - 0.5) * 0.045;
                float waterFarFade = 1.0 - smoothstep(1.56 + farEdgeNoise, 1.94 + farEdgeNoise, away);
                float waterNearFade = smoothstep(-0.030 + farEdgeNoise * 0.24, 0.045 + farEdgeNoise * 0.24, away);
                float floorCardFade = lerp(1.0, lateral * waterFarFade * waterNearFade, waterLiquid);
                float seam = 1.0 - smoothstep(0.0, 0.040, away);
                float floodNoise = 0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 2.7 + seed * 9.0, seed * 31.0));
                float floorFrontNoise = (Fbm3(float3(u * 3.1 + seed * 17.0, away * 1.9, seed * 53.0)) - 0.5) * 0.20 +
                    (Noise3(float3(u * 10.0, away * 5.5, seed * 79.0)) - 0.5) * 0.055;
                float lateFeatherStart = lerp(14.0, 10.0, waterLiquid);
                float lateFeatherEnd = lerp(34.0, 52.0, waterLiquid);
                float lateFeather = smoothstep(lateFeatherStart, lateFeatherEnd, leakAge) *
                    smoothstep(0.22, 0.86, pooledField + wetRim * 0.70) *
                    smoothstep(0.56, 0.91, floodNoise + floorFrontNoise * 0.55) *
                    (1.0 - smoothstep(lerp(0.74, 0.88, waterLiquid), lerp(1.16, 1.42, waterLiquid), away)) *
                    lateral * lerp(0.22, 0.38, waterLiquid);
                merged = saturate(merged + lateFeather);
                merged *= floorCardFade;
                wetRim *= floorCardFade;
                earlySoakField *= floorCardFade;
                lobeThickness *= floorCardFade;
                float soakNoise = 0.78 + 0.22 * Hash21(float2(floor(input.worldPos.x * 7.0) + seed * 41.0, floor(input.worldPos.z * 7.0)));
                float fibers = 0.82 + 0.18 * Hash21(float2(floor(input.worldPos.x * 9.0) + seed * 47.0, floor(input.worldPos.z * 9.0)));
                if (highBloodDetail)
                {
                    soakNoise = 0.65 + 0.35 * Fbm3(float3(input.worldPos.xz * 10.0, seed * 41.0));
                    fibers = 0.70 + 0.30 * Fbm3(float3(input.worldPos.xz * 11.0, seed * 41.0));
                }
                float soak = saturate(merged + earlySoakField * lerp(0.42, 0.68, waterLiquid)) *
                    lateral * lerp(0.22, 0.36, waterLiquid) * soakNoise;
                alpha = max(max(merged, soak * fibers), seam * lateral * wetRim * 0.75);
                alpha = smoothstep(lerp(0.024, 0.014, waterLiquid), lerp(0.112, 0.088, waterLiquid), alpha);
                thickness = saturate(lobeThickness * 0.86 + soak * lerp(0.28, 0.48, waterLiquid) +
                    earlySoakField * lerp(0.070, 0.16, waterLiquid) + seam * wetRim * 0.46);
                drips = saturate(merged + seam * wetRim * 0.45);
                thickness *= alpha;
                drips *= alpha;
            }
        }

)" R"(

        else
        {
            if (wallLeakSurface > 0.5)
            {
                float u = bloodUv.x;
                float away = bloodUv.y;
                float edgeJitterU = (Hash21(float2(floor(away * 10.0) + seed * 31.0, seed * 17.0)) - 0.5) * 0.055;
                float edgeJitterV = (Hash21(float2(floor(u * 10.0) + seed * 37.0, seed * 23.0)) - 0.5) * 0.050;
                if (highBloodDetail)
                {
                    edgeJitterU = (Fbm3(float3(away * 5.6, seed * 9.0, u * 1.7)) - 0.5) * 0.070;
                    edgeJitterV = (Fbm3(float3(u * 5.2, seed * 11.0, away * 1.9)) - 0.5) * 0.064;
                }
                float cardEdgeFade = smoothstep(0.012 + edgeJitterU, 0.115 + edgeJitterU, u) *
                    (1.0 - smoothstep(0.885 - edgeJitterU, 0.988 - edgeJitterU, u)) *
                    smoothstep(0.010 + edgeJitterV, 0.105 + edgeJitterV, away) *
                    (1.0 - smoothstep(0.850 - edgeJitterV, 0.985 - edgeJitterV, away));
                float topSource = 0.0;
                float topThickness = 0.0;
                [loop]
                for (int i = 0; i < 32; ++i)
                {
                    float fi = (float)i;
                    if (fi >= ceilingStreamCount) break;
                    float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
                    float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
                    float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
                    float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 3.0);
                    float clusterId = floor(fmod(fi + floor(seed * 31.0), clusterCount));
                    float uniformCenter = 0.055 + ((fi + 0.20 + r0 * 0.60) / max(1.0, streamCount)) * 0.89;
                    float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
                    float clusterSpread = 0.025 + Hash21(float2(seed * 97.0 + clusterId, 29.0)) * 0.080;
                    float clusteredCenter = clusterCenter + (Hash21(float2(seed * 131.0 + fi, 37.0)) - 0.5) * clusterSpread;
                    float center = clamp(lerp(uniformCenter, clusteredCenter, 0.58 + Hash21(float2(seed * 151.0 + fi, 43.0)) * 0.34), 0.045, 0.955);
                    float densityBase = Hash21(float2(center * 17.0 + seed * 11.0, clusterId * 3.1 + 5.0));
                    if (highBloodDetail)
                    {
                        densityBase = smoothstep(0.24, 0.78, Fbm3(float3(center * 4.2, seed * 18.0, clusterId * 2.3)));
                    }
                    float densityBand = densityBase;
                    float streamStrength = lerp(0.24, 1.18, densityBand) * lerp(0.58, 1.12, Hash21(float2(seed * 173.0 + fi, 47.0)));
                    float streamAge = max(0.0, leakAge - r0 * 2.2 - fi * 0.16);
                    float sourceGrow = smoothstep(0.0, 1.0, saturate(streamAge * (0.065 + r1 * 0.040) * lerp(1.0, 0.72, waterLiquid)));
                    float sourceReady = smoothstep(0.25, 1.15, streamAge);
                    float spreadAway = 0.030 + sourceGrow * (0.145 + r2 * 0.095);
                    float spreadSide = (0.018 + r2 * 0.022) * streamWidthScale * (1.0 + sourceGrow * 1.10);
                    spreadAway *= lerp(1.0, 1.22, waterLiquid);
                    spreadSide *= lerp(1.0, 1.14, waterLiquid);
                    float edgeNoise = (Hash21(float2(floor(u * 12.0) + fi, floor(away * 10.0) + seed * 23.0)) - 0.5) * 0.010 * sourceGrow;
                    if (highBloodDetail)
                    {
                        edgeNoise = (Fbm3(float3(u * 8.0 + fi, away * 7.0, seed * 23.0)) - 0.5) * 0.016 * sourceGrow;
                    }
                    float sideWorld = (u - center) * bloodUvMeters.x;
                    float awayWorld = away * bloodUvMeters.y;
                    float spreadSideWorld = max(0.006, spreadSide * bloodUvMeters.x);
                    float spreadAwayWorld = max(0.010, spreadAway * bloodUvMeters.x);
                    float edgeNoiseWorld = edgeNoise * bloodUvMeters.x;
                    float skew = (Hash21(float2(seed * 19.0 + fi, 91.0)) - 0.5) * sourceGrow * 0.42;
                    float2 q = float2((sideWorld + awayWorld * skew) / spreadSideWorld, (awayWorld + edgeNoiseWorld) / spreadAwayWorld);
                    float soakRag = Fbm3(float3(input.worldPos.xz * (11.0 + r2 * 9.0) + fi * 0.29, seed * 43.0 + fi));
                    float microBreak = Noise3(float3(input.worldPos.xz * (18.0 + r2 * 14.0) + fi * 0.17, seed * 71.0 + fi));
                    float lobe = 1.0 - smoothstep(0.54, 1.03,
                        dot(q, q) + (soakRag - 0.5) * (0.36 + sourceGrow * 0.42) + (microBreak - 0.5) * 0.10);
                    float capillary = smoothstep(0.60, 0.94,
                        Fbm3(float3(input.worldPos.xz * (20.0 + r1 * 10.0) + fi, seed * 61.0))) *
                        (1.0 - smoothstep(0.72, 1.28, dot(q, q))) * sourceGrow * sourceReady * 0.18;
                    float source = max(lobe, capillary) * sourceReady * streamStrength;
                    topSource = max(topSource, source);
                    topThickness = max(topThickness, source * (0.48 + sourceGrow * 0.42));
                }
                float raggedEdge = 0.88 + 0.12 * Hash21(float2(floor(u * 26.0) + seed * 67.0, floor(away * 18.0)));
                if (highBloodDetail)
                {
                    raggedEdge = 0.80 + 0.20 * Fbm3(float3(u * 20.0, away * 14.0, seed * 67.0));
                }
                float ceilingNoise = Fbm3(float3(input.worldPos.xz * 4.8 + seed * 7.0, seed * 37.0));
                float fineNoise = Noise3(float3(input.worldPos.xz * 18.0 + seed * 5.0, seed * 83.0));
                float organicMask = smoothstep(0.22, 0.70, ceilingNoise + (fineNoise - 0.5) * 0.20);
                float rimAge = smoothstep(lerp(5.5, 4.0, waterLiquid), lerp(11.5, 17.0, waterLiquid), leakAge);
                float rimNoise = Fbm3(float3(u * 5.4 + seed * 13.0, away * 3.2, seed * 47.0));
                float rimFine = Noise3(float3(u * 19.0 + seed * 7.0, away * 9.0, seed * 71.0));
                float rimWidth = 0.026 + rimNoise * 0.034;
                float rimSideFade = smoothstep(0.010 + edgeJitterU, 0.090 + edgeJitterU, u) *
                    (1.0 - smoothstep(0.910 - edgeJitterU, 0.990 - edgeJitterU, u));
                float rimBreakup = smoothstep(0.16, 0.72, rimNoise + (rimFine - 0.5) * 0.22);
                float delayedRim = (1.0 - smoothstep(rimWidth, rimWidth + 0.070, away)) *
                    rimSideFade * rimBreakup * rimAge;
                float soakFrontNoise = (Fbm3(float3(u * 3.4 + seed * 13.0, away * 1.7, seed * 41.0)) - 0.5) * 0.22 +
                    (Noise3(float3(u * 11.0, away * 5.0, seed * 73.0)) - 0.5) * 0.06;
                float rimFeed = smoothstep(0.04, 0.30, topSource);
                float soakReach = saturate((leakAge - lerp(7.5, 5.0, waterLiquid)) / lerp(34.0, 50.0, waterLiquid));
                float unevenCeilingReach = saturate(soakReach + rimFeed * 0.22 + soakFrontNoise);
                float ceilingFront = 1.0 - smoothstep(unevenCeilingReach,
                    unevenCeilingReach + 0.17 + abs(soakFrontNoise) * 0.10,
                    away);
                float ceilingSoak = smoothstep(lerp(8.0, 5.0, waterLiquid), lerp(31.0, 52.0, waterLiquid), leakAge) *
                    ceilingFront * smoothstep(0.006, 0.058, away) *
                    (1.0 - smoothstep(1.01 + edgeJitterV, 1.11 + edgeJitterV, away)) *
                    cardEdgeFade * organicMask * (0.62 + ceilingNoise * 0.38);
                topSource = saturate(topSource * cardEdgeFade + ceilingSoak * lerp(0.52, 0.74, waterLiquid) +
                    delayedRim * (lerp(0.52, 0.68, waterLiquid) + rimFeed * 0.40));
                alpha = topSource * raggedEdge * ceilingMask * smoothstep(0.0, 0.35, leakAge);
                alpha = smoothstep(lerp(0.014, 0.008, waterLiquid), lerp(0.092, 0.072, waterLiquid), alpha);
                thickness = saturate(topThickness * 0.86 + ceilingSoak * lerp(0.54, 0.78, waterLiquid) +
                    delayedRim * lerp(0.42, 0.58, waterLiquid)) * alpha;
            }
            else
            {
                float centerThickness = 0.0;
                float centerSpeed = lerp(0.024, 0.016, waterLiquid);
                float centerRadius = lerp(0.58, 0.74, waterLiquid);
                float source = CenterSeepPool(bloodUv, input.worldPos, seed, leakAge, centerSpeed, centerRadius, centerThickness);
                alpha = source * ceilingMask * smoothstep(0.0, lerp(0.65, 1.05, waterLiquid), leakAge);
                alpha = smoothstep(lerp(0.024, 0.012, waterLiquid), lerp(0.116, 0.088, waterLiquid), alpha);
                thickness = centerThickness * alpha;
            }
            drips = 0.0;
        }

        float wetAlpha = alpha;
        float wetThickness = thickness;
        float wetDrips = drips;
        alpha = wetAlpha * animMask;
        thickness = wetThickness * animMask;
        drips = wetDrips * animMask;
        if (alpha < lerp(0.045, 0.026, waterLiquid)) discard;

        float2 local = bloodUv * 2.0 - 1.0;
        float2 bulgeSlope = -local * thickness * (0.08 + wet * 0.08);
        bulgeSlope.y += wallMask * drips * (0.030 + wet * 0.028);
        bulgeSlope += (float2(
            Fbm3(float3(bloodUv * 18.0, seed * 47.0)),
            Fbm3(float3(bloodUv.yx * 18.0 + 3.7, seed * 53.0))) - 0.5) * thickness * 0.010;
        float3 B2 = normalize(cross(N, T));
        float3 worldN = normalize(N + T * bulgeSlope.x + B2 * bulgeSlope.y);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float3 reflectDir = reflect(-toLight, worldN);
        float facing = saturate(dot(reflectDir, V));
        float fresnel = pow(1.0 - saturate(dot(worldN, V)), 4.0);
        float specLight = saturate(flashlight + overhead * 0.34 + sparkLight * 0.72 + exitGlow * 0.48);
        float spec = (pow(facing, 168.0) * 1.85 + pow(facing, 34.0) * 0.36 + fresnel * 0.16) *
            specLight * (0.58 + wet * 1.85) * saturate(alpha + thickness * 0.35);
        float grime = Fbm3(input.worldPos * float3(8.0, 5.0, 8.0) + seed * 31.0);
        float filmAlpha = saturate(alpha * (0.58 + thickness * 0.38 + drips * 0.12 + wet * 0.035));
        if (filmAlpha < lerp(0.045, 0.026, waterLiquid)) discard;
        float lightEnergy = saturate(flashlight * 0.88 + overhead * 0.38 + sparkLight * 0.62 + exitGlow * 0.34 + gLighting0.z * 0.06);
        float3 thinBlood = float3(0.320, 0.0140, 0.0052);
        float3 pooledBlood = float3(0.092, 0.0024, 0.0012);
        float3 blood = lerp(thinBlood, pooledBlood, saturate(thickness * 0.88 + drips * 0.24));
        float3 color = blood * (0.24 + lightEnergy * 1.02) * (0.70 + grime * 0.22);
        color = lerp(color, color * float3(0.54, 0.28, 0.24), drips * 0.18);
        color += float3(0.72, 0.62, 0.48) * spec;
        color += exitGreen * (0.06 + wet * 0.16) * saturate(alpha + thickness * 0.18);
        if (waterLiquid > 0.5)
        {
            float waterCore = saturate(alpha * 0.74 + thickness * 0.26);
            float waterFresnel = pow(1.0 - saturate(dot(worldN, V)), 3.0);
            float waterSpec = spec * (0.54 + waterCore * 1.10) + waterFresnel * specLight * (0.08 + waterCore * 0.18);
            float3 thinTint = float3(0.045, 0.070, 0.076) * (0.42 + lightEnergy * 0.34);
            float3 deepTint = float3(0.025, 0.044, 0.052) * (0.36 + lightEnergy * 0.26);
            float3 clearFilm = lerp(thinTint, deepTint, saturate(thickness * 0.80 + drips * 0.20));
            float3 reflectedLamp = float3(0.42, 0.56, 0.58) * waterSpec;
            color = clearFilm * (0.48 + waterCore * 0.38) + reflectedLamp;
            color += exitGreen * (0.018 + waterCore * 0.052);
            filmAlpha = saturate(alpha * (0.20 + thickness * 0.11 + drips * 0.045));
        }
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.45;
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.25);
        color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
        filmAlpha *= 1.0 - fogBlock * 0.18;
        return float4(ApplyPost(color), filmAlpha);
    }

)" R"(

    if (input.material > 11.05 && input.material < 11.45)
    {
        float seed = frac(input.material * 23.71);
        float2 smokeUv = AspectSoftenedBloodUv(uv, input.worldPos);
        float2 plane = float2(smokeUv.x * 2.0 - 1.0, 1.0 - smokeUv.y * 2.0);
        float edgeDist = min(min(smokeUv.x, 1.0 - smokeUv.x), min(smokeUv.y, 1.0 - smokeUv.y));
        float cardFade = smoothstep(0.110, 0.330, edgeDist);
        float2 ovalP = plane * float2(1.10, 1.04);
        float radial = dot(ovalP, ovalP);
        float ovalFade = exp(-radial * 2.65) * (1.0 - smoothstep(0.62, 1.02, radial));
        float3 rdLocal = normalize(float3(dot(-V, T), dot(-V, B), dot(-V, N)));
        float3 p0 = float3(plane.x * 0.96, plane.y * 1.05, 0.0);
        float heightFade = smoothstep(0.02, 0.30, input.worldPos.y) * (1.0 - smoothstep(2.10, 2.70, input.worldPos.y));
        float transmittance = 1.0;
        float alpha = 0.0;
        [loop]
        for (int s = 0; s < 9; ++s)
        {
            float stepT = -0.95 + ((float)s + 0.5) * (1.90 / 9.0);
            float3 p = p0 + rdLocal * stepT;
            p.xz *= 0.90 + seed * 0.16;
            p.y += sin(time * (0.31 + seed * 0.10) + seed * 21.0) * 0.10;
            float density = BlackSmokeDensity(p, seed + (float)s * 0.017, time);
            float sampleAlpha = saturate(density * 0.210);
            alpha += transmittance * sampleAlpha;
            transmittance *= 1.0 - sampleAlpha;
        }
        float reform = 0.83 + 0.17 * sin(time * (0.68 + seed * 0.21) + seed * 17.0 + input.worldPos.y * 1.7);
        alpha = saturate(alpha * heightFade * cardFade * ovalFade * reform * 1.42);
        if (alpha < 0.012) discard;
        float3 color = float3(0.0, 0.0, 0.0);

        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.35);
        alpha *= pow(1.0 - fogBlock, 1.65);
        return float4(color, alpha);
    }

    if (input.material > 11.005 && input.material < 11.055)
    {
        float seed = frac(input.material * 29.73);
        float encodedSide = clamp(floor(rawUv.x + 0.0001), 0.0, 3.0);
        float rawModeEncoded = floor(rawUv.y + 0.0001);
        float floorNeighborMask = floor(rawModeEncoded / 8.0);
        float rawMode = rawModeEncoded - floorNeighborMask * 8.0;
        float warpA = Fbm3(float3(uv * (2.8 + seed * 2.2) + seed * 5.1, seed * 17.0));
        float warpB = Fbm3(float3(uv.yx * (3.4 + seed * 1.7) - seed * 4.3, seed * 29.0));
        float2 warpedUv = saturate(uv + (float2(warpA, warpB) - 0.5) * (0.075 + seed * 0.035));
        float2 d = warpedUv - 0.5;
        float border = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        float broad = Fbm3(float3(warpedUv * 5.7 + seed * 3.1, seed * 17.0));
        float fine = Fbm3(float3(warpedUv * 18.0 - seed * 2.0, seed * 31.0));
        float floorSurface = saturate(N.y * 2.0);
        float ceilingSurface = saturate(-N.y * 2.0);
        float horizontalSurface = saturate(abs(N.y) * 2.0);
        float vertical = 1.0 - horizontalSurface;
        float floorBridge = floorSurface * step(3.5, rawMode) * (1.0 - step(4.5, rawMode));
        float wallFromCeiling = vertical * step(2.5, rawMode) * (1.0 - step(3.5, rawMode));
        float wallFromFloor = vertical * step(3.5, rawMode);
        float ceilingCompact = ceilingSurface * (1.0 - vertical) * step(2.5, rawMode) * (1.0 - step(3.5, rawMode));
        float floorTouchdown = floorSurface * step(4.5, rawMode) * (1.0 - step(5.5, rawMode));
        float encodedMode = clamp(lerp(rawMode, 0.0, saturate(ceilingCompact + floorTouchdown)), 0.0, 2.0);
        float encodedEdge = (1.0 - vertical) * step(0.5, encodedMode);
        float edgeOnly = (1.0 - vertical) * step(1.5, encodedMode);
        float materialBand = saturate((input.material - 11.006) / 0.049);
        float edgeMode = max(smoothstep(0.32, 0.60, materialBand), encodedEdge);
        float floorMergeN = step(0.5, fmod(floor(floorNeighborMask / 1.0), 2.0));
        float floorMergeS = step(0.5, fmod(floor(floorNeighborMask / 2.0), 2.0));
        float floorMergeW = step(0.5, fmod(floor(floorNeighborMask / 4.0), 2.0));
        float floorMergeE = step(0.5, fmod(floor(floorNeighborMask / 8.0), 2.0));
        float floorMergeNW = step(0.5, fmod(floor(floorNeighborMask / 16.0), 2.0)) * saturate(floorMergeN + floorMergeW);
        float floorMergeNE = step(0.5, fmod(floor(floorNeighborMask / 32.0), 2.0)) * saturate(floorMergeN + floorMergeE);
        float floorMergeSW = step(0.5, fmod(floor(floorNeighborMask / 64.0), 2.0)) * saturate(floorMergeS + floorMergeW);
        float floorMergeSE = step(0.5, fmod(floor(floorNeighborMask / 128.0), 2.0)) * saturate(floorMergeS + floorMergeE);
        float2 mergedD = d;
        mergedD.x = d.x * lerp(1.0, 0.76, saturate(floorMergeW + floorMergeE)) +
            (floorMergeW - floorMergeE) * 0.08;
        mergedD.y = d.y * lerp(1.0, 0.76, saturate(floorMergeN + floorMergeS)) +
            (floorMergeS - floorMergeN) * 0.08;

        float puddleAngle = seed * 6.28318 + Hash21(float2(seed, 3.7)) * 2.4;
        float2 puddleRot = float2(cos(puddleAngle), sin(puddleAngle));
        float2 rd = float2(dot(mergedD, puddleRot), dot(mergedD, float2(-puddleRot.y, puddleRot.x)));
        float2 puddleAspect = float2(0.82 + Hash21(float2(seed, 5.1)) * 0.62,
                                     0.74 + Hash21(float2(seed, 7.3)) * 0.72);
        float2 floorPuddleAspect = float2(0.94 + Hash21(float2(seed, 5.1)) * 0.14,
                                          0.94 + Hash21(float2(seed, 7.3)) * 0.14);
        float floorShape = smoothstep(0.70, 0.16, length(rd * floorPuddleAspect) + (broad - 0.5) * 0.105);
        float floorLobes = 0.0;
        [loop]
        for (int f = 0; f < 4; ++f)
        {
            float ff = (float)f;
            float enabled = step(0.22, Hash21(float2(seed * 67.0 + ff, 31.0)));
            float2 fc = float2(Hash21(float2(seed * 71.0 + ff, 37.0)),
                               Hash21(float2(seed * 73.0, ff + 41.0))) - 0.5;
            fc *= 0.18 + Hash21(float2(seed * 79.0 + ff, 43.0)) * 0.34;
            float fa = seed * 3.9 + ff * 1.21 + Hash21(float2(seed, ff + 47.0)) * 1.3;
            float2 fr = float2(cos(fa), sin(fa));
            float2 fq = mergedD - fc;
            fq = float2(dot(fq, fr), dot(fq, float2(-fr.y, fr.x)));
            fq *= float2(1.10 + Hash21(float2(ff, seed * 83.0)) * 0.82,
                         0.76 + Hash21(float2(seed * 89.0, ff)) * 0.58);
            float oval = smoothstep(0.34 + Hash21(float2(seed * 97.0, ff)) * 0.17, 0.09, length(fq));
            floorLobes = max(floorLobes, oval * enabled * (0.48 + Hash21(float2(seed * 101.0 + ff, 53.0)) * 0.40));
        }
        floorShape = max(floorShape, floorLobes);
        float touchdownNoise = (Fbm3(float3(warpedUv * (10.0 + seed * 3.0) + seed * 11.0, seed * 59.0)) - 0.5) * 0.095 +
            (fine - 0.5) * 0.030;
        float touchdownRadial = length(rd * float2(0.94 + seed * 0.08, 0.88 + Hash21(float2(seed, 61.0)) * 0.12));
        float touchdownCore = 1.0 - smoothstep(0.30, 0.47, touchdownRadial + touchdownNoise * 0.55);
        float touchdownBody = 1.0 - smoothstep(0.48, 0.64, touchdownRadial + touchdownNoise);
        float touchdownRim = (1.0 - smoothstep(0.62, 0.75, touchdownRadial + touchdownNoise * 1.15)) *
            smoothstep(0.34, 0.58, touchdownRadial + touchdownNoise * 0.65);
        float touchdownShape = saturate(max(touchdownCore, touchdownBody * 0.88) + touchdownRim * 0.22);
        floorShape = lerp(floorShape, max(touchdownShape, floorShape * 0.32), floorTouchdown);
        float floorSeamNoise = (Fbm3(float3(uv * 8.0 + seed * 13.0, seed * 41.0)) - 0.5) * 0.075;
        float floorSeamBreak = smoothstep(0.18, 0.72, broad + fine * 0.13);
        float floorAlongX = smoothstep(0.030, 0.185, uv.x) * (1.0 - smoothstep(0.815, 0.970, uv.x));
        float floorAlongY = smoothstep(0.030, 0.185, uv.y) * (1.0 - smoothstep(0.815, 0.970, uv.y));
        float floorWetN = floorMergeN * (1.0 - smoothstep(0.27 + floorSeamNoise, 0.62 + floorSeamNoise, 1.0 - uv.y)) * floorAlongX;
        float floorWetS = floorMergeS * (1.0 - smoothstep(0.27 - floorSeamNoise, 0.62 - floorSeamNoise, uv.y)) * floorAlongX;
        float floorWetW = floorMergeW * (1.0 - smoothstep(0.27 - floorSeamNoise, 0.62 - floorSeamNoise, uv.x)) * floorAlongY;
        float floorWetE = floorMergeE * (1.0 - smoothstep(0.27 + floorSeamNoise, 0.62 + floorSeamNoise, 1.0 - uv.x)) * floorAlongY;
        float cornerNW = floorMergeNW * (1.0 - smoothstep(0.24, 0.62, length(float2(uv.x, 1.0 - uv.y) * float2(1.08, 1.08)) + floorSeamNoise));
        float cornerNE = floorMergeNE * (1.0 - smoothstep(0.24, 0.62, length(float2(1.0 - uv.x, 1.0 - uv.y) * float2(1.08, 1.08)) - floorSeamNoise));
        float cornerSW = floorMergeSW * (1.0 - smoothstep(0.24, 0.62, length(float2(uv.x, uv.y) * float2(1.08, 1.08)) - floorSeamNoise));
        float cornerSE = floorMergeSE * (1.0 - smoothstep(0.24, 0.62, length(float2(1.0 - uv.x, uv.y) * float2(1.08, 1.08)) + floorSeamNoise));
        float floorSeamShape = max(max(floorWetN, floorWetS), max(floorWetW, floorWetE));
        float floorCornerShape = max(max(cornerNW, cornerNE), max(cornerSW, cornerSE));
        floorShape = max(floorShape, max(floorSeamShape * (0.34 + floorSeamBreak * 0.18),
            floorCornerShape * (0.74 + floorSeamBreak * 0.26)));

)" R"(

        floorShape *= 1.0 - smoothstep(0.78, 0.96,
            Fbm3(float3(warpedUv * (8.0 + seed * 3.0) + seed * 9.0, seed * 37.0))) * 0.075;

        float ceilingShape = smoothstep(0.66, 0.15, length(rd * puddleAspect) + (broad - 0.5) * 0.25);
        float ceilingSdf = 10.0;
        float ceilingField = 0.0;
        [loop]
        for (int l = 0; l < 9; ++l)
        {
            float fl = (float)l;
            float2 lc = float2(Hash21(float2(seed * 17.0 + fl, 11.0)),
                               Hash21(float2(seed * 23.0, fl + 13.0))) - 0.5;
            lc *= 0.18 + Hash21(float2(seed * 31.0 + fl, 17.0)) * 0.72;
            float la = seed * 5.1 + fl * 1.37 + Hash21(float2(seed, fl + 19.0)) * 1.9;
            float2 lr = float2(cos(la), sin(la));
            float2 q = mergedD - lc;
            q = float2(dot(q, lr), dot(q, float2(-lr.y, lr.x)));
            q *= float2(1.05 + Hash21(float2(fl, seed * 37.0)) * 1.15,
                        0.76 + Hash21(float2(seed * 41.0, fl)) * 0.95);
            float radius = 0.16 + Hash21(float2(seed * 43.0, fl)) * 0.27;
            float edgeRough = (Fbm3(float3((q + lc) * (8.0 + fl * 0.7), seed * 47.0 + fl)) - 0.5) * 0.070;
            float sdf = length(q) - radius + edgeRough;
            ceilingSdf = min(ceilingSdf, sdf);
            float lobe = 1.0 - smoothstep(-0.035, 0.125 + Hash21(float2(seed * 53.0, fl)) * 0.08, sdf);
            ceilingField += saturate(lobe) * (0.45 + Hash21(float2(seed * 47.0 + fl, 23.0)) * 0.48);
        }
        float islands = smoothstep(0.66, 0.94, fine) * smoothstep(0.84, 0.22, length(mergedD * (1.12 + seed * 0.32)));
        float mergedCeiling = max(1.0 - smoothstep(-0.015, 0.110, ceilingSdf),
            smoothstep(0.62, 1.42, ceilingField));
        float dryBreak = smoothstep(0.72, 0.94,
            Fbm3(float3(warpedUv * (9.0 + seed * 5.0) + seed * 13.0, seed * 53.0))) *
            smoothstep(0.12, 0.72, max(ceilingShape, mergedCeiling));
        float ceilingNoiseEdge = (Fbm3(float3(warpedUv * 15.0 + seed * 19.0, seed * 61.0)) - 0.5) * 0.18;
        ceilingShape = max(max(ceilingShape * 0.52, mergedCeiling), islands * 0.74) * lerp(1.0, 0.22, edgeOnly);
        ceilingShape *= 1.0 - dryBreak * 0.26;
        ceilingShape = saturate(ceilingShape + ceilingNoiseEdge * smoothstep(0.18, 0.78, ceilingShape));
        float ceilingSeamNoise = (Fbm3(float3(uv * 6.5 + seed * 17.0, seed * 73.0)) - 0.5) * 0.105;
        float ceilingSeamBreak = smoothstep(0.16, 0.76, broad + fine * 0.16);
        float ceilingAlongX = smoothstep(0.020, 0.155, uv.x) * (1.0 - smoothstep(0.845, 0.985, uv.x));
        float ceilingAlongY = smoothstep(0.020, 0.155, uv.y) * (1.0 - smoothstep(0.845, 0.985, uv.y));
        float ceilingWetN = floorMergeN * (1.0 - smoothstep(0.20 + ceilingSeamNoise, 0.58 + ceilingSeamNoise, 1.0 - uv.y)) * ceilingAlongX;
        float ceilingWetS = floorMergeS * (1.0 - smoothstep(0.20 - ceilingSeamNoise, 0.58 - ceilingSeamNoise, uv.y)) * ceilingAlongX;
        float ceilingWetW = floorMergeW * (1.0 - smoothstep(0.20 - ceilingSeamNoise, 0.58 - ceilingSeamNoise, uv.x)) * ceilingAlongY;
        float ceilingWetE = floorMergeE * (1.0 - smoothstep(0.20 + ceilingSeamNoise, 0.58 + ceilingSeamNoise, 1.0 - uv.x)) * ceilingAlongY;
        float ceilingCornerNW = floorMergeNW * (1.0 - smoothstep(0.18, 0.58, length(float2(uv.x, 1.0 - uv.y) * float2(1.0, 1.0)) + ceilingSeamNoise));
        float ceilingCornerNE = floorMergeNE * (1.0 - smoothstep(0.18, 0.58, length(float2(1.0 - uv.x, 1.0 - uv.y) * float2(1.0, 1.0)) - ceilingSeamNoise));
        float ceilingCornerSW = floorMergeSW * (1.0 - smoothstep(0.18, 0.58, length(float2(uv.x, uv.y) * float2(1.0, 1.0)) - ceilingSeamNoise));
        float ceilingCornerSE = floorMergeSE * (1.0 - smoothstep(0.18, 0.58, length(float2(1.0 - uv.x, uv.y) * float2(1.0, 1.0)) + ceilingSeamNoise));
        float ceilingSeamShape = max(max(ceilingWetN, ceilingWetS), max(ceilingWetW, ceilingWetE));
        float ceilingCornerShape = max(max(ceilingCornerNW, ceilingCornerNE), max(ceilingCornerSW, ceilingCornerSE));
        float ceilingNeighborShape = max(ceilingSeamShape * (0.30 + ceilingSeamBreak * 0.16),
            ceilingCornerShape * (0.72 + ceilingSeamBreak * 0.26));
        ceilingNeighborShape *= 1.0 - smoothstep(0.82, 0.98,
            Fbm3(float3(uv * 13.0 + seed * 23.0, seed * 79.0))) * 0.18;
        ceilingShape = max(ceilingShape, ceilingNeighborShape);
        float2 compactOffset = float2(Hash21(float2(seed * 109.0, 71.0)),
                                      Hash21(float2(seed * 113.0, 73.0))) - 0.5;
        compactOffset *= 0.22;
        float2 compactQ = mergedD - compactOffset;
        float compactAngle = seed * 4.7 + Hash21(float2(seed * 127.0, 79.0)) * 2.2;
        float2 compactRot = float2(cos(compactAngle), sin(compactAngle));
        compactQ = float2(dot(compactQ, compactRot), dot(compactQ, float2(-compactRot.y, compactRot.x)));
        compactQ *= float2(1.04 + Hash21(float2(seed * 131.0, 83.0)) * 0.90,
                           0.82 + Hash21(float2(seed * 137.0, 89.0)) * 0.56);
        float compactRadius = 0.18 + Hash21(float2(seed * 139.0, 97.0)) * 0.17;
        float compactNoise = (Fbm3(float3((warpedUv + compactOffset) * 11.0 + seed * 7.0, seed * 101.0)) - 0.5) * 0.105;
        float compactMask = 1.0 - smoothstep(compactRadius, compactRadius + 0.145, length(compactQ) + compactNoise);
        float compactCore = 1.0 - smoothstep(compactRadius * 0.58, compactRadius + 0.090, length(compactQ) + compactNoise * 0.62);
        float compactBreak = 1.0 - smoothstep(0.76, 0.96,
            Fbm3(float3(warpedUv * (12.0 + seed * 3.0) + seed * 17.0, seed * 107.0))) * 0.16;
        ceilingShape = lerp(ceilingShape, max(ceilingShape * compactMask, compactCore * 0.68) * compactBreak, ceilingCompact);
        float sidePick = lerp(floor(seed * 4.0), encodedSide, encodedEdge);
        float edgeAway = uv.y;
        float edgeAlong = uv.x;
        if (sidePick >= 1.0 && sidePick < 2.0)
        {
            edgeAway = 1.0 - uv.y;
            edgeAlong = 1.0 - uv.x;
        }
        else if (sidePick >= 2.0 && sidePick < 3.0)
        {
            edgeAway = uv.x;
            edgeAlong = 1.0 - uv.y;
        }
        else if (sidePick >= 3.0)
        {
            edgeAway = 1.0 - uv.x;
            edgeAlong = uv.y;
        }
        float edgeNoise = Fbm3(float3(edgeAlong * 5.2 + seed * 4.3, edgeAway * 3.4 - seed * 2.1, seed * 43.0));
        float edgeFine = Fbm3(float3(edgeAlong * 19.0 - seed * 7.0, edgeAway * 14.0 + seed * 3.0, seed * 71.0));
        float edgeReach = 0.28 + seed * 0.30 + (edgeNoise - 0.5) * 0.18;
        float edgeFront = 1.0 - smoothstep(edgeReach, edgeReach + 0.13 + abs(edgeNoise - 0.5) * 0.10, edgeAway);
        float edgeTaper = smoothstep(0.02, 0.18, edgeAlong) * smoothstep(0.98, 0.74, edgeAlong);
        float edgeBreakup = smoothstep(0.18, 0.76, edgeNoise + (edgeFine - 0.5) * 0.24);
        float edgeThreads = smoothstep(0.80, 0.98, edgeFine) *
            (1.0 - smoothstep(edgeReach * 0.80, edgeReach + 0.26, edgeAway)) * edgeTaper;
        float edgeShape = max(edgeFront * edgeTaper * edgeBreakup, edgeThreads * 0.72);
        ceilingShape = max(ceilingShape, edgeShape * edgeMode);
        float horizontal = lerp(ceilingShape, floorShape, floorSurface);
        float wallFlowY = uv.y;
        float wallSource = wallFromCeiling;
        float wallWaterCore = 0.0;
        float wallWaterHalo = 0.0;
        float wallWaterSoak = 0.0;
        float wallBottomSoak = 0.0;
        float wallCardSideFade = smoothstep(0.014, 0.095, uv.x) * (1.0 - smoothstep(0.905, 0.990, uv.x));
        float wallEndFade = 1.0 - smoothstep(0.995, 1.012, wallFlowY);
        float wallWaterDebugActive = step(1.0, gTransition0.w);
        float wallWaterDebugPhase = frac(gTransition0.w);
        float wallWaterBaseAge = lerp(9.0, wallWaterDebugPhase * 8.5, wallWaterDebugActive);
        [loop]
        for (int wf = 0; wf < 11; ++wf)
        {
            float fi = (float)wf;
            float r0 = Hash21(float2(seed * 47.0 + fi, 3.0));
            float r1 = Hash21(float2(seed * 31.0, fi + 5.0));
            float r2 = Hash21(float2(fi + 9.0, seed * 71.0));
            float clusterCount = 2.0 + floor(Hash21(float2(seed * 83.0, 19.0)) * 2.0);
            float clusterId = floor(fmod(fi + floor(seed * 19.0), clusterCount));
            float uniformCenter = 0.070 + ((fi + 0.25 + r0 * 0.50) / 11.0) * 0.86;
            float clusterCenter = 0.08 + Hash21(float2(seed * 89.0 + clusterId * 5.7, 13.0)) * 0.84;
            float center = clamp(lerp(uniformCenter, clusterCenter + (r1 - 0.5) * 0.14, 0.46 + r2 * 0.22), 0.045, 0.955);
            float enabled = step(0.18, Hash21(float2(seed * 109.0 + fi, 41.0)));
            float len = 1.035 + r1 * 0.105;
            float flowDelay = r0 * 1.30 + fi * (0.045 + r2 * 0.030);
            float flowAge = max(0.0, wallWaterBaseAge - flowDelay);
            float speedPhase = flowAge * (0.58 + r2 * 0.74) + seed * 19.0 + fi * 1.73;
            float flowClock = max(0.0, flowAge * (1.08 + r1 * 0.28) +
                sin(speedPhase) * (0.10 + r0 * 0.09) +
                sin(speedPhase * 2.11 + r2 * 5.0) * 0.030);
            float flowGrow = smoothstep(0.0, 1.0, saturate(flowClock * (0.42 + r1 * 0.14)));
            float dynamicLen = lerp(0.075 + r2 * 0.055, len, flowGrow);
            float flowReady = smoothstep(0.02, 0.34 + r0 * 0.24, flowAge);
            float width = 0.0040 + r2 * 0.0075;
            float wander = (r1 - 0.5) * wallFlowY * (0.020 + r2 * 0.028) +
                (Fbm3(float3(wallFlowY * 5.2 + fi, seed * 11.0, 2.0)) - 0.5) * (0.007 + r0 * 0.010);
            float du = uv.x - center - wander;
            float trailGate = (1.0 - smoothstep(dynamicLen, dynamicLen + 0.045, wallFlowY)) *
                smoothstep(0.000, 0.035 + r0 * 0.025, wallFlowY);
            float breakNoise = smoothstep(0.16, 0.70,
                Fbm3(float3(uv.x * 28.0 + fi * 1.7, wallFlowY * 16.0, seed * 47.0)) + r2 * 0.18);
            float continuousFlow = lerp(breakNoise, max(breakNoise, 0.58 + r2 * 0.18),
                smoothstep(0.34, 0.96, wallFlowY) * flowGrow);
            float gravityWidth = width * (0.72 + smoothstep(0.22, 0.92, wallFlowY) * (0.36 + r1 * 0.44));
            float coreTrail = exp(-(du * du) / max(0.000012, gravityWidth * gravityWidth)) *
                trailGate * continuousFlow * enabled * wallFromCeiling * flowReady * (0.62 + r0 * 0.55);
            float haloTrail = exp(-(du * du) / max(0.000035, gravityWidth * gravityWidth * 18.0)) *
                trailGate * enabled * wallFromCeiling * flowReady * (0.35 + r1 * 0.30);
            float floorContact = exp(-(du * du) / max(0.000035, gravityWidth * gravityWidth * 22.0)) *
                smoothstep(0.82, 0.998, wallFlowY) * flowGrow * enabled * wallFromCeiling * flowReady;
            float sourceWidth = width * (3.8 + r1 * 2.5);
            float sourcePool = exp(-(du * du) / max(0.00005, sourceWidth * sourceWidth)) *
                (1.0 - smoothstep(0.060 + r2 * 0.045, 0.185 + r2 * 0.060, wallFlowY)) *
                enabled * wallFromCeiling * flowReady * (0.32 + r0 * 0.38);
            wallWaterCore = max(wallWaterCore, max(coreTrail, floorContact * 0.32));
            wallWaterHalo = max(wallWaterHalo, max(max(haloTrail * 0.72, sourcePool), floorContact * 0.62));
            wallWaterSoak += saturate(haloTrail * 0.36 + sourcePool * 0.24 + floorContact * 0.12);
        }
        wallWaterSoak = smoothstep(0.18, 1.05, wallWaterSoak) *
            smoothstep(0.10, 0.82, broad + wallWaterHalo * 0.38) *
            wallCardSideFade * wallEndFade;
        wallWaterHalo = saturate(max(wallWaterHalo * wallCardSideFade * wallEndFade, wallWaterSoak * 0.58));
        wallWaterCore *= wallCardSideFade * wallEndFade;
        float bottomDist = 1.0 - uv.y;
        float bottomNoise = (Fbm3(float3(uv.x * 9.0 + seed * 7.0, bottomDist * 5.0, seed * 91.0)) - 0.5) * 0.055;
        wallBottomSoak = wallFromFloor * wallCardSideFade *
            (1.0 - smoothstep(0.10 + bottomNoise, 0.34 + bottomNoise, bottomDist)) *
            smoothstep(0.18, 0.78, broad + fine * 0.16);
        float wallWetShape = saturate(max(max(wallWaterHalo * 0.72, wallWaterCore * 1.15) * wallSource,
            wallBottomSoak * 0.72));
        float shape = lerp(horizontal, wallWetShape, vertical);

)" R"(
        float bridgeBorder = min(min(uv.x, 1.0 - uv.x), min(uv.y, 1.0 - uv.y));
        float bridgeFilm = smoothstep(0.010, 0.075, bridgeBorder) *
            (0.86 + broad * 0.14);
        shape = max(shape, bridgeFilm * floorBridge);
        float edgeAwareBorder = border;
        if (edgeMode > 0.001)
        {
            if (sidePick < 1.0)
            {
                edgeAwareBorder = min(min(uv.x, 1.0 - uv.x), 1.0 - uv.y);
            }
            else if (sidePick < 2.0)
            {
                edgeAwareBorder = min(min(uv.x, 1.0 - uv.x), uv.y);
            }
            else if (sidePick < 3.0)
            {
                edgeAwareBorder = min(min(uv.y, 1.0 - uv.y), 1.0 - uv.x);
            }
            else
            {
                edgeAwareBorder = min(min(uv.y, 1.0 - uv.y), uv.x);
            }
        }
        float mergedFloorBorder = min(
            min(lerp(uv.x, 1.0, floorMergeW), lerp(1.0 - uv.x, 1.0, floorMergeE)),
            min(lerp(uv.y, 1.0, floorMergeS), lerp(1.0 - uv.y, 1.0, floorMergeN)));
        float floorMergeCoverage = saturate(max(floorSeamShape, floorCornerShape) * 1.65);
        float floorEdgeBorder = lerp(edgeAwareBorder, max(edgeAwareBorder, mergedFloorBorder), floorSurface * floorMergeCoverage);
        float borderNoise = (Fbm3(float3(uv * 11.0 + seed * 23.0, seed * 67.0)) - 0.5) * 0.032;
        float floorBorder = smoothstep(0.030 + borderNoise * 0.65, 0.155 + borderNoise * 1.15, floorEdgeBorder);
        float ceilingMergeCoverage = saturate(max(ceilingSeamShape, ceilingCornerShape) * 1.55);
        float ceilingEdgeAwareBorder = lerp(edgeAwareBorder, max(edgeAwareBorder, mergedFloorBorder), ceilingSurface * ceilingMergeCoverage);
        float ceilingSoftBorder = smoothstep(0.014 + borderNoise, 0.165 + borderNoise, ceilingEdgeAwareBorder);
        float ceilingEdgeBorder = smoothstep(0.006 + borderNoise * 0.45, 0.095 + borderNoise * 0.60, ceilingEdgeAwareBorder);
        float ceilingBorder = lerp(ceilingSoftBorder, ceilingEdgeBorder, edgeMode);
        float cardBorder = lerp(ceilingBorder, floorBorder, floorSurface);
        float verticalBorder = smoothstep(0.006, 0.14, border);
        float wallSideBorder = smoothstep(0.006, 0.14, min(uv.x, 1.0 - uv.x));
        verticalBorder = lerp(verticalBorder, wallSideBorder, saturate(wallFromCeiling + wallFromFloor));
        shape *= lerp(lerp(cardBorder, 1.0, floorBridge), verticalBorder, vertical);
        float debugLoopActive = step(1.0, gTransition0.w);
        float debugPhase = frac(gTransition0.w);
        float debugSpread = saturate((debugPhase - 0.05) / 0.70);
        float debugFade = 1.0 - smoothstep(0.88, 0.98, debugPhase);
        float2 debugCenterTile = floor(gMaze1.xy * 0.5) + 0.5;
        float2 debugCenterXZ = gMaze0.xy + debugCenterTile * gMaze0.zw;
        float2 debugTileDelta = (input.worldPos.xz - debugCenterXZ) / max(gMaze0.zw, float2(0.001, 0.001));
        float debugRadius = max(0.80, (max(gMaze1.x, gMaze1.y) - 2.0) * 0.58);
        float debugDist = lerp(length(debugTileDelta * float2(0.82, 0.82)), lerp(uv.y, 1.0 - uv.y, wallFromFloor), vertical);
        float debugEdgeNoise = (fine - 0.5) * 0.13 + (broad - 0.5) * 0.07;
        float debugReveal = (1.0 - smoothstep(debugSpread * debugRadius, debugSpread * debugRadius + 0.42, debugDist + debugEdgeNoise)) * debugFade;
        shape *= lerp(1.0, debugReveal, debugLoopActive);
        float touchdownGrow = smoothstep(0.46, 0.66, debugPhase);
        shape *= lerp(1.0, touchdownGrow, debugLoopActive * floorTouchdown);
        if (shape < 0.045) discard;

        float core = smoothstep(0.13, 0.82, shape);
        core = max(core, wallWaterCore * vertical);
        float rim = smoothstep(0.035, 0.18, shape) * (1.0 - smoothstep(0.38, 0.74, shape));
        float3 wetN = normalize(N + T * (fine - 0.5) * 0.035 + B * (broad - 0.5) * 0.026);
        float flashlight = FlashlightAmount(input.worldPos, wetN);
        float overhead = LocalLampLight(input.worldPos, wetN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, wetN);
        float3 exitGreen = ExitSignLight(input.worldPos, wetN);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, wetN), V));
        float fresnel = pow(1.0 - saturate(dot(wetN, V)), 5.0);
        float specLight = saturate(flashlight + overhead * 0.58 + sparkLight * 0.42 + exitGlow * 0.54);
        float spec = (pow(facing, 110.0) * 0.30 + pow(facing, 30.0) * 0.052 + fresnel * 0.014) * specLight * core;
        float filmAlpha = saturate(core * (0.40 + broad * 0.145) + rim * (0.105 + fine * 0.050) +
            vertical * (wallWaterCore * 0.16 + wallWaterSoak * 0.07 + wallBottomSoak * 0.10));
        filmAlpha *= lerp(1.12, 0.82, vertical);
        if (filmAlpha < 0.035) discard;
        float3 wetFilm = float3(0.0018, 0.0024, 0.0022) * (0.26 + specLight * 0.13);
        float3 color = wetFilm + float3(0.36, 0.42, 0.39) * spec;
        float wallFlow = vertical * saturate(wallWaterCore + wallWaterSoak * 0.42);
        float wallDamp = vertical * wallBottomSoak;
        color = lerp(color, float3(0.0007, 0.0012, 0.0010) * (0.50 + specLight * 0.24), wallFlow * 0.76);
        color = lerp(color, float3(0.0008, 0.0011, 0.0010) * (0.38 + specLight * 0.16), wallDamp * 0.42);
        color += float3(0.28, 0.36, 0.33) * spec * wallFlow * 0.72;
        color += exitGreen * (0.055 + core * 0.12 + spec * 0.55);
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        float fogBlock = saturate(fog * gFog0.z * 1.28);
        color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
        filmAlpha *= pow(1.0 - fogBlock, 1.35);
        return float4(ApplyPost(color), filmAlpha);
    }

)" R"(

    if (gHorror0.x > 0.01 && materialId < 11.5 && !(materialId > 3.5 && materialId < 4.5))
    {
        float flesh = saturate(gHorror0.x);
        float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.12));
        float2 fleshUv = rawUv * 0.72 + float2(Hash21(floor(input.worldPos.xz * 0.27)), Hash21(floor(input.worldPos.zx * 0.31))) * 0.23;
        float parallaxScale = gHorror0.w * (0.55 + flesh * 0.45);
        float layers = lerp(18.0, 9.0, saturate(viewTS.z));
        float2 stepUv = (viewTS.xy / max(viewTS.z, 0.12)) * (parallaxScale / layers);
        float2 pomUv = fleshUv;
        float layerDepth = 0.0;
        float currentDepth = 1.0 - gNormalHeight.Sample(gSampler, float3(pomUv, 15.0)).a;
        [loop]
        for (int p = 0; p < 18; ++p)
        {
            if ((float)p >= layers || layerDepth >= currentDepth) break;
            pomUv -= stepUv;
            layerDepth += 1.0 / layers;
            currentDepth = 1.0 - gNormalHeight.Sample(gSampler, float3(pomUv, 15.0)).a;
        }
        fleshUv = lerp(fleshUv, pomUv, saturate(parallaxScale * 18.0));
        float3 materialUv = float3(fleshUv, 15.0);
        float4 base = gAlbedo.Sample(gSampler, materialUv);
        float4 nh = gNormalHeight.Sample(gSampler, materialUv);
        float4 pbr = gMaterialProps.Sample(gSampler, materialUv);
        float aoMap = saturate(pbr.r);
        float sourceRoughness = saturate(pbr.g);
        float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
        nTex = normalize(float3(nTex.xy * (1.28 + flesh * 0.42), nTex.z));
        float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float3 reflectDir = reflect(-toLight, worldN);
        float wet = saturate(gHorror0.z);
        float ndv = saturate(dot(worldN, V));
        float fresnel = pow(1.0 - ndv, 4.0);
        float facing = saturate(dot(reflectDir, V));
        float effectiveRoughness = lerp(sourceRoughness, max(0.18, sourceRoughness * 0.42), wet);
        effectiveRoughness = saturate(effectiveRoughness - wet * 0.055);
        float gloss = 1.0 - effectiveRoughness;
        float specSharp = pow(facing, lerp(24.0, 150.0, gloss)) * lerp(0.42, 2.7, gloss);
        float specBroad = pow(facing, lerp(5.0, 22.0, gloss)) * lerp(0.48, 0.16, gloss);
        float poreSparkle = smoothstep(0.58, 0.92, Fbm3(float3(fleshUv * 42.0, 17.0))) * wet * gloss;
        float spec = (specSharp + specBroad + fresnel * (0.20 + wet * 0.36) + poreSparkle * 0.36) *
            flashlight * (0.75 + wet * 3.3 + flesh * 0.85);
        float cavity = saturate((0.58 - nh.a) * 1.9);
        float ridge = saturate((nh.a - 0.48) * 1.7);
        float3 fleshColor = base.rgb * (0.48 + flesh * 0.22) + float3(0.10, 0.005, 0.003) * flesh;
        float aoShadow = lerp(0.34, 1.0, aoMap);
        fleshColor = lerp(fleshColor, fleshColor * float3(0.28, 0.08, 0.07), saturate(cavity * 0.62 + (1.0 - aoMap) * 0.54));
        fleshColor += float3(0.085, 0.014, 0.008) * ridge * wet;
        float3 color = fleshColor * flashlight * 0.94 * aoShadow;
        color += float3(1.0, 0.21, 0.085) * spec * lerp(0.62, 0.98, aoMap);
        color *= 1.0 - CornerAO(input.worldPos, worldN) * 0.65;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if ((materialId > 2.5 && materialId < 3.5) || (materialId > 4.5 && materialId < 5.5))
    {
        float edge = max(smoothstep(0.055, 0.0, min(uv.x, 1.0 - uv.x)),
                         smoothstep(0.055, 0.0, min(uv.y, 1.0 - uv.y)));
        float lens = smoothstep(0.42, 0.0, abs(uv.y - 0.5)) * smoothstep(0.46, 0.0, abs(uv.x - 0.5));
        float3 lampBase = float3(0.72 + lens * 0.24 - edge * 0.18,
                                 0.76 + lens * 0.23 - edge * 0.16,
                                 0.70 + lens * 0.20 - edge * 0.12);
        float3 offBase = float3(0.018 + lens * 0.012,
                                0.018 + lens * 0.012,
                                0.016 + lens * 0.010);
        float3 base = materialId < 3.5 ? lampBase : offBase;
        float emit = materialId < 3.5 ? LampVisualPower(input.material, input.worldPos, time) * 2.6 * (1.0 - saturate(gTransition0.z)) : 0.0;
        float3 color = base * (gLighting0.z + emit);
        float fog = saturate((length(input.worldPos - cam) - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z);
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 11.5 && materialId < 12.5)
    {
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        float variant = frac(input.material);
        if (variant > 0.55)
        {
            float2 d = uv - 0.5;
            float radial = exp(-dot(d, d) * 5.4);
            float curl = Hash21(floor((uv + time * float2(0.05, -0.09)) * 9.0 + input.material * 23.0));
            float life = saturate((variant - 0.55) / 0.40);
            float alpha = radial * (0.18 + curl * 0.16) * life;
            if (alpha < 0.018) discard;
            float flashlight = FlashlightAmount(input.worldPos, N);
            float nonFleshLight = 1.0 - saturate(gTransition0.z);
            float3 color = float3(0.46, 0.50, 0.47) * (flashlight * 0.14 + (0.18 + gLighting0.z) * nonFleshLight);
            color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.75);
            return float4(ApplyPost(color), alpha);
        }

        float3 materialUv = MaterialUV(rawUv, input.material);
        float4 base = gAlbedo.Sample(gSampler, materialUv);
        if (base.a < 0.03) discard;
        fog = 1.0 - exp(-fog * fog * 1.4);
        float pulse = 0.96 + 0.04 * sin(time * 7.1 + input.material * 17.0);
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        if (nonFleshLight < 0.001) discard;
        float3 color = (float3(13.0, 0.35, 0.10) * base.a * pulse + base.rgb * 3.2) * nonFleshLight;
        float fogBlock = saturate(fog * gFog0.z * 1.55);
        float fogVisibility = pow(1.0 - fogBlock, 2.7);
        color *= fogVisibility;
        return float4(saturate(ApplyPost(color) + color * 0.35 * fogVisibility), saturate(base.a * 1.55 * fogVisibility));
    }

    if (materialId > 12.5 && materialId < 13.5)
    {
        float2 d = uv - 0.5;
        float r = length(d);
        float r2 = dot(d, d);
        float variant = frac(input.material);
        float core = exp(-r2 * 86.0);
        float halo = exp(-r2 * 28.0);
        float roundMask = smoothstep(0.47, 0.18, r);
        float alpha = saturate(core * 1.18 + halo * 0.44) * roundMask * (0.70 + variant * 0.62);
        if (alpha < 0.016) discard;
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        if (nonFleshLight < 0.001) discard;
        float3 color = (float3(8.4, 3.7, 0.72) * core + float3(2.8, 0.78, 0.12) * halo) * nonFleshLight;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 1.4);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.24);
        return float4(saturate(ApplyPost(color) + color * 0.18), saturate(alpha));
    }

    if (materialId > 14.5 && materialId < 15.5)
    {
        float variant = frac(input.material);
        float particleFade = smoothstep(0.055, 0.24, variant);
        float3 toLight = input.worldPos - gShadow0.xyz;
        float lightDist = length(toLight);
        float focus = max(0.45, gAir0.x);
        float blur = saturate(abs(lightDist - focus) / (0.62 + lightDist * 0.18)) * saturate(gAir0.y);
        float2 p = uv * 2.0 - 1.0;
        float angle = atan2(p.y, p.x);
        float3 stable = floor(input.worldPos * 2.7 + variant * 31.0);
        float h0 = Hash31(stable + 3.0);
        float h1 = Hash31(stable.yzx + 17.0);
        float h2 = Hash31(stable.zxy + 41.0);
        float2 q = p;
        float lobesA = sin(angle * (5.0 + floor(h0 * 4.0)) + variant * 38.0 + time * 0.035);
        float lobesB = sin(angle * (9.0 + floor(h1 * 5.0)) + variant * 71.0 - time * 0.026);
        float corner = sin(angle * (13.0 + floor(h2 * 4.0)) + h1 * 19.0);
        float sides = 5.0 + floor(h0 * 6.0);
        float sector = floor((angle + 3.14159265 + variant * 6.2831853) / (6.2831853 / sides));
        float faceted = (Hash21(float2(sector, variant * 43.0)) - 0.5) * 0.22;
        float edge = 0.58 + faceted + lobesA * 0.13 + lobesB * 0.08 + corner * 0.045;
        float r = length(q);
        float blob = smoothstep(edge + 0.13 + blur * 0.24, edge - 0.06 - blur * 0.10, r);
        float shell = smoothstep(edge + 0.24 + blur * 0.26, edge + 0.02, r) * (1.0 - blob);
        float holes = step(0.60 + blur * 0.18, Hash21(floor((q + variant) * (7.0 + h0 * 8.0))));
        blob *= lerp(1.0, 0.52, holes * (1.0 - blur * 0.45));
        float shape = max(blob, shell * 0.28);
        float flecks = lerp(0.78, 1.10, Hash21(floor(q * (10.0 + h1 * 9.0)) + variant * 23.0));
        shape *= flecks * smoothstep(1.22, 0.35, length(p));
        if (shape < 0.018) discard;

        float flashlight = FlashlightAmount(input.worldPos, N);
        float3 lightDir = normalize(gShadow1.xyz);
        float axisDist = max(0.0, dot(toLight, lightDir));
        float outerCos = clamp(gShadow2.z, 0.04, 0.98);
        float coneRadius = max(0.018, axisDist * sqrt(max(0.0, 1.0 - outerCos * outerCos)) / outerCos);
        float radialDist = length(toLight - lightDir * axisDist);
        float centerLine = 1.0 - smoothstep(0.08, 0.84, radialDist / coneRadius);
        float flashlightScale = sqrt(max(0.0, gLighting0.x));
        float centerBoost = (0.76 + centerLine * 0.70) * lerp(0.72, 1.24, saturate(flashlightScale * 0.78));
        float lightFade = smoothstep(0.35, 1.10, lightDist) * (1.0 - smoothstep(gShadow2.y * 0.46, gShadow2.y * 0.82, lightDist));
        float depthFade = 1.0 - smoothstep(gFog0.y * 0.72, gFog0.y, length(input.worldPos - cam));
        float focusAlpha = lerp(1.0, 0.46, blur);
        float alpha = shape * flashlight * centerBoost * lightFade * depthFade * focusAlpha * (0.20 + variant * 0.12) * particleFade * (1.0 - saturate(gTransition0.z));
        if (alpha < 0.010) discard;
        float3 color = float3(0.72, 0.78, 0.72) * (0.20 + flashlight * (1.08 + centerLine * 0.82));
        color += float3(0.42, 0.48, 0.44) * shell * (0.08 + blur * 0.08 + centerLine * 0.08) * flashlightScale;
        float fog = saturate((length(input.worldPos - cam) - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 2.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z * 0.65);
        return float4(ApplyPost(color), saturate(alpha));
    }

    if (input.material > 10.50 && input.material < 10.90)
    {
        float ndv = saturate(dot(N, V));
        float rim = pow(1.0 - ndv, 2.2);
        float hot = 0.82 + pow(ndv, 0.45) * 0.95 + rim * 0.34;
        float flutter = 0.96 + 0.04 * sin(time * 6.2 + input.material * 19.0);
        float3 color = float3(8.8, 0.28, 0.075) * hot * flutter;
        color += float3(3.8, 0.065, 0.018) * rim;
        float dist = length(input.worldPos - cam);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 1.8);
        float fogBlock = saturate(fog * gFog0.z * 1.55);
        float fogVisibility = pow(1.0 - fogBlock, 2.7);
        color *= fogVisibility;
        return float4(saturate(ApplyPost(color) + color * 0.32 * fogVisibility), fogVisibility);
    }

)" R"(
    if (materialId > 8.5 && materialId < 9.5 && frac(input.material) > 0.5)
    {
        float grain = Fbm3(input.worldPos * float3(12.0, 18.0, 12.0) + 2.3);
        float stain = Fbm3(input.worldPos * float3(4.0, 7.0, 4.0) + 18.0);
        float ridge = Fbm3(input.worldPos * float3(38.0, 24.0, 38.0) + 41.0);
        float3 worldN = normalize(N + T * (grain - 0.5) * 0.10 + B * (ridge - 0.5) * 0.055);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 bone = float3(0.62, 0.58, 0.48);
        bone += (grain - 0.5) * float3(0.10, 0.085, 0.055);
        bone -= smoothstep(0.54, 0.88, stain) * float3(0.13, 0.12, 0.090);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float facing = saturate(dot(reflect(-toLight, worldN), V));
        float spec = pow(facing, 34.0) * 0.22 * (flashlight + sparkLight * 0.45);
        float dist = length(input.worldPos - cam);
        float3 color = bone * (gLighting0.z * 0.34 + flashlight * 1.12 + overhead * 0.26 + sparkLight * 0.80);
        color += float3(0.78, 0.68, 0.48) * spec;
        color = lerp(color, float3(0.0, 0.0, 0.0), SceneFogBlock(dist, input.worldPos, 1.0));
        return float4(ApplyPost(color), 1.0);
    }

    if (materialId > 23.5 && materialId < 24.5)
    {
        float softness = saturate(frac(input.material));
        float2 p = uv * 2.0 - 1.0;
        float edgeNoise = (Fbm3(float3(input.worldPos.xz * (2.1 + softness * 1.4), softness * 17.0)) - 0.5) * (0.055 + softness * 0.070);
        float fineBreakup = Fbm3(float3(input.worldPos.xz * 8.5 + softness * 13.0, softness * 31.0));
        float r = length(p * float2(0.84, 1.16));
        float contact = 1.0 - smoothstep(0.16, 0.68 + softness * 0.24, r + edgeNoise * 0.28);
        float feather = 1.0 - smoothstep(0.26 + softness * 0.10, 1.16 + softness * 0.44, r + edgeNoise);
        float shape = saturate(max(contact * (0.26 - softness * 0.075), feather * 0.86));
        shape *= 0.82 + fineBreakup * 0.18;
        float localFixturePower = FixturePower(input.worldPos, time) * gLighting1.x;
        float flickerLinkedShadow = saturate(localFixturePower * lerp(0.42, 0.32, softness));
        float alpha = shape * flickerLinkedShadow * lerp(0.125, 0.070, softness) * (1.0 - saturate(gTransition0.z));
        if (alpha < 0.006) discard;
        return float4(0.0, 0.0, 0.0, alpha);
    }

    if ((materialId > 0.5 && materialId < 1.5) || (materialId > 1.5 && materialId < 2.5))
    {
        float floorMaterial = materialId < 1.5;
        float mipBias = floorMaterial > 0.5 ? 1.15 : 0.75;
        float3 materialUv = MaterialUV(rawUv, input.material);
        float4 base = gAlbedo.SampleBias(gSampler, materialUv, mipBias);
        base.rgb = BackroomsBaseColor(base.rgb, materialId);
        float4 pbr = gMaterialProps.SampleBias(gSampler, materialUv, mipBias);
        float4 nh = gNormalHeight.SampleBias(gSampler, materialUv, mipBias + 0.55);
        float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
        float normalStrength = floorMaterial > 0.5 ? 0.055 : 0.135;
        nTex = normalize(float3(nTex.xy * normalStrength, nTex.z));
        float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);
        float dist = length(input.worldPos - cam);
        float flashlight = FlashlightAmount(input.worldPos, worldN);
        float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;
        float sparkLight = SparkLight(input.worldPos, worldN);
        float3 exitGreen = ExitSignLight(input.worldPos, worldN);
        float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));
        float lift = gLighting0.z * (floorMaterial > 0.5 ? 0.018 : 0.040) * (1.0 - saturate(gTransition0.z));
        float aoMap = saturate(pbr.r);
        float roughness = saturate(pbr.g);
        float3 color = base.rgb * (gLighting0.z + overhead + flashlight + sparkLight + lift) * lerp(0.55, 1.0, aoMap);
        color += base.rgb * exitGreen * lerp(0.55, 1.0, aoMap);
        float3 toLight = normalize(gShadow0.xyz - input.worldPos);
        float specFacing = saturate(dot(reflect(-toLight, worldN), V));
        float gloss = 1.0 - roughness;
        float specSharpness = lerp(12.0, 92.0, gloss);
        float specLight = flashlight + sparkLight * 0.55 + exitGlow * 0.42 + overhead * (floorMaterial > 0.5 ? 0.18 : 0.62);
        float ceilingSheen = floorMaterial > 0.5 ? 0.16 : 0.54;
        float surfaceSpec = pow(specFacing, specSharpness) * gloss * gloss * ceilingSheen * specLight;
        color += float3(1.0, 0.92, 0.76) * surfaceSpec * lerp(0.72, 1.0, aoMap);
        color *= 1.0 - CornerAO(input.worldPos, N);
        float fog = saturate((dist - gFog0.x) / max(0.01, gFog0.y - gFog0.x));
        fog = 1.0 - exp(-fog * fog * 3.2);
        color = lerp(color, float3(0.0, 0.0, 0.0), fog * gFog0.z);
        return float4(ApplyPost(color), 1.0);
    }

    float3 viewTS = float3(dot(V, T), dot(V, B), max(dot(V, N), 0.18));
    float parallaxScale = 0.0;
    if (materialId < 0.5) parallaxScale = 0.018;
    else if (materialId < 1.5) parallaxScale = 0.0;
    else if (materialId < 2.5) parallaxScale = 0.003;
    float2 sampledRawUv = rawUv;
    if (materialId > 6.5 && materialId < 7.5)
    {
        sampledRawUv.y = 1.0 - sampledRawUv.y;
    }
    float3 firstUv = MaterialUV(sampledRawUv, input.material);
    float height = parallaxScale > 0.0 ? gNormalHeight.Sample(gSampler, firstUv).a : 0.48;
    sampledRawUv += (height - 0.48) * parallaxScale * viewTS.xy / viewTS.z;

    float3 materialUv = MaterialUV(sampledRawUv, input.material);
    float floorMipBias = (materialId > 0.5 && materialId < 1.5) ? 1.75 : 0.0;
    float4 base = gAlbedo.SampleBias(gSampler, materialUv, floorMipBias);
    base.rgb = BackroomsBaseColor(base.rgb, materialId);
    float4 pbr = gMaterialProps.SampleBias(gSampler, materialUv, floorMipBias);
    if (materialId > 3.5 && base.a < 0.08) discard;

    float4 nh = gNormalHeight.SampleBias(gSampler, materialUv, floorMipBias);
    float3 nTex = normalize(nh.xyz * 2.0 - 1.0);
    float normalStrength = 0.55;
    if (materialId > 0.5 && materialId < 1.5) normalStrength = 0.0;
    else if (materialId > 1.5 && materialId < 2.5) normalStrength = 0.35;
    nTex = normalize(float3(nTex.xy * normalStrength, nTex.z));
    float3 worldN = normalize(nTex.x * T + nTex.y * B + nTex.z * N);

    float dist = length(input.worldPos - cam);
    float flashlight = FlashlightAmount(input.worldPos, worldN);
    float sparkLight = SparkLight(input.worldPos, worldN);
    float3 exitGreen = ExitSignLight(input.worldPos, worldN);
    float exitGlow = max(exitGreen.r, max(exitGreen.g, exitGreen.b));

    float fixture = FixturePower(input.worldPos, time);
    float overhead = LocalLampLight(input.worldPos, worldN, time) * gLighting1.x;

    float ambient = gLighting0.z;
    float aoMap = saturate(pbr.r);
    float roughness = saturate(pbr.g);
    float3 color = base.rgb * (ambient + overhead + flashlight + sparkLight) * lerp(0.58, 1.0, aoMap);
    color += base.rgb * exitGreen * lerp(0.58, 1.0, aoMap);
    float3 toLight = normalize(gShadow0.xyz - input.worldPos);
    float specFacing = saturate(dot(reflect(-toLight, worldN), V));
    float gloss = 1.0 - roughness;
    float surfaceSpec = pow(specFacing, lerp(18.0, 95.0, gloss)) * gloss * 0.18 * (flashlight + sparkLight * 0.5 + exitGlow * 0.45);
    color += float3(1.0, 0.92, 0.78) * surfaceSpec;
    if (materialId > 1.5 && materialId < 2.5)
    {
        color += base.rgb * 0.035 * (1.0 - saturate(gTransition0.z));
    }
    if (materialId > 6.5 && materialId < 7.5)
    {
        float nonFleshLight = 1.0 - saturate(gTransition0.z);
        color = base.rgb * ((0.28 + flashlight * 0.2) * nonFleshLight + flashlight * 0.12) + base.rgb * 1.7 * nonFleshLight;
    }
    color *= 1.0 - CornerAO(input.worldPos, worldN);
    float eyeGlow = 0.0;
    if (materialId > 3.5 && materialId < 4.5)
    {
        eyeGlow = saturate((base.r - max(base.g, base.b)) * 3.5);
        color += base.rgb * eyeGlow * 2.4;
    }

    float fogBlock = SceneFogBlock(dist, input.worldPos, 1.0);
    color = lerp(color, float3(0.0, 0.0, 0.0), fogBlock);
    float3 posted = ApplyPost(color);
    if (materialId > 3.5 && materialId < 4.5)
    {
        float eyeFogVisibility = pow(1.0 - saturate(fogBlock * 1.55), 2.7);
        eyeGlow *= eyeFogVisibility;
        float death = saturate(gPost0.w);
        float eyeHold = smoothstep(0.48, 0.72, death) * (1.0 - smoothstep(0.92, 1.0, death));
        posted += float3(eyeGlow * 1.7, eyeGlow * 0.06, eyeGlow * 0.025);
        posted += float3(eyeGlow * 4.2, eyeGlow * 0.12, eyeGlow * 0.04) * eyeHold;
    }
    return float4(saturate(posted), base.a);
}
)";

        ComPtr<ID3DBlob> vs;
        ComPtr<ID3DBlob> hs;
        ComPtr<ID3DBlob> ds;
        ComPtr<ID3DBlob> ps;
        ComPtr<ID3DBlob> shadowPs;
        static const char* overlayShader = R"(
struct OverlayVSIn
{
    float2 pos : POSITION;
    float4 color : COLOR0;
};

struct OverlayVSOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR0;
};

OverlayVSOut OverlayVS(OverlayVSIn input)
{
    OverlayVSOut o;
    o.pos = float4(input.pos, 0.0, 1.0);
    o.color = input.color;
    return o;
}

float4 OverlayPS(OverlayVSOut input) : SV_TARGET
{
    return input.color;
}
)";
        static const char* postShader = R"(
cbuffer SceneConstants : register(b0)
{
    row_major float4x4 gViewProj;
    row_major float4x4 gLightViewProj;
    float4 gCameraPosTime;
    float4 gCameraDirAspect;
    float4 gLighting0;
    float4 gLighting1;
    float4 gFog0;
    float4 gAO0;
    float4 gPost0;
    float4 gPost1;
};

Texture2D gSceneColor : register(t0);
SamplerState gPostSampler : register(s0);

struct PostVSOut
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PostVSOut PostVS(uint vertexId : SV_VertexID)
{
    PostVSOut o;
    float2 p = float2((vertexId << 1) & 2, vertexId & 2);
    o.uv = p;
    o.pos = float4(p * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    return o;
}

float DirtHash(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}

float LensDirt(float2 uv)
{
    float2 centered = uv - 0.5;
    float vignette = smoothstep(0.18, 0.78, length(centered));
    float fine = DirtHash(floor(uv * 52.0)) * 0.55 + DirtHash(floor(uv * 137.0 + 19.0)) * 0.45;
    float smudgeA = smoothstep(0.36, 0.0, length((uv - float2(0.32, 0.42)) * float2(1.0, 1.8)));
    float smudgeB = smoothstep(0.42, 0.0, length((uv - float2(0.68, 0.58)) * float2(1.5, 0.9)));
    return saturate(vignette * 0.38 + pow(fine, 8.0) * 0.42 + smudgeA * 0.22 + smudgeB * 0.18);
}

float3 BrightPart(float3 c)
{
    float luma = dot(c, float3(0.299, 0.587, 0.114));
    float gate = smoothstep(0.58, 1.0, luma);
    return c * gate;
}

float4 PostPS(PostVSOut input) : SV_TARGET
{
    uint w;
    uint h;
    gSceneColor.GetDimensions(w, h);
    float2 texel = 1.0 / max(float2(w, h), float2(1.0, 1.0));
    float2 uv = input.uv;
    float danger = saturate(gPost0.z);
    float death = saturate(gPost0.w);
    float bloomAmount = saturate(gPost1.z);
    float dirtAmount = saturate(gPost1.w);
    float2 motion = clamp(gPost1.xy, float2(-0.045, -0.045), float2(0.045, 0.045));

    float3 color = gSceneColor.Sample(gPostSampler, uv).rgb;
    float3 blur = color * 0.36;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 0.35)).rgb * 0.22;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 0.72)).rgb * 0.17;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv - motion * 1.08)).rgb * 0.11;
    blur += gSceneColor.Sample(gPostSampler, saturate(uv + motion * 0.30)).rgb * 0.14;
    float motionWeight = saturate(length(motion) * 42.0);
    color = lerp(color, blur, motionWeight * (0.34 + danger * 0.22));

    float2 bloomStep = texel * (2.0 + danger * 2.0);
    float3 bloom = BrightPart(color) * 0.42;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 1.0,  0.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-1.0,  0.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 0.0,  1.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 0.0, -1.0)).rgb) * 0.12;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 1.0,  1.0)).rgb) * 0.08;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-1.0,  1.0)).rgb) * 0.08;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2( 1.0, -1.0)).rgb) * 0.08;
    bloom += BrightPart(gSceneColor.Sample(gPostSampler, uv + bloomStep * float2(-1.0, -1.0)).rgb) * 0.08;
    float dirt = LensDirt(uv) * dirtAmount;
    color += bloom * bloomAmount * (0.12 + dirt * 0.55);
    color += float3(1.0, 0.92, 0.72) * dirt * bloomAmount * 0.018;
    color *= 1.0 - smoothstep(0.58, 1.04, length((uv - 0.5) * float2(gCameraDirAspect.w, 1.0))) * (0.055 + dirtAmount * 0.035);
    color = lerp(color, float3(0.0, 0.0, 0.0), smoothstep(0.82, 1.0, death));
    return float4(saturate(color), 1.0);
}
)";
        ComPtr<ID3DBlob> overlayVs;
        ComPtr<ID3DBlob> overlayPs;
        ComPtr<ID3DBlob> postVs;
        ComPtr<ID3DBlob> postPs;
        if (!CompileShader(shader, "VSMain", "vs_4_0", vs)) return false;
        if (featureLevel_ >= D3D_FEATURE_LEVEL_11_0) {
            if (!CompileShader(shader, "HSMain", "hs_5_0", hs)) return false;
            if (!CompileShader(shader, "DSMain", "ds_5_0", ds)) return false;
        }
        const char* mainPsProfile = featureLevel_ >= D3D_FEATURE_LEVEL_11_0 ? "ps_5_0" : "ps_4_0";
        if (!CompileShader(shader, "PSMain", mainPsProfile, ps)) return false;
        if (!CompileShader(shader, "ShadowPS", "ps_4_0", shadowPs)) return false;
        if (!CompileShader(overlayShader, "OverlayVS", "vs_4_0", overlayVs)) return false;
        if (!CompileShader(overlayShader, "OverlayPS", "ps_4_0", overlayPs)) return false;
        if (!CompileShader(postShader, "PostVS", "vs_4_0", postVs)) return false;
        if (!CompileShader(postShader, "PostPS", "ps_4_0", postPs)) return false;
        HRESULT hr = device_->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, &vertexShader_);
        if (FAILED(hr)) return false;
        if (featureLevel_ >= D3D_FEATURE_LEVEL_11_0) {
            hr = device_->CreateHullShader(hs->GetBufferPointer(), hs->GetBufferSize(), nullptr, &hullShader_);
            if (FAILED(hr)) return false;
            hr = device_->CreateDomainShader(ds->GetBufferPointer(), ds->GetBufferSize(), nullptr, &domainShader_);
            if (FAILED(hr)) return false;
        }
        hr = device_->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, &pixelShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreatePixelShader(shadowPs->GetBufferPointer(), shadowPs->GetBufferSize(), nullptr, &shadowPixelShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreateVertexShader(overlayVs->GetBufferPointer(), overlayVs->GetBufferSize(), nullptr, &overlayVertexShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreatePixelShader(overlayPs->GetBufferPointer(), overlayPs->GetBufferSize(), nullptr, &overlayPixelShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreateVertexShader(postVs->GetBufferPointer(), postVs->GetBufferSize(), nullptr, &postVertexShader_);
        if (FAILED(hr)) return false;
        hr = device_->CreatePixelShader(postPs->GetBufferPointer(), postPs->GetBufferSize(), nullptr, &postPixelShader_);
        if (FAILED(hr)) return false;

        D3D11_INPUT_ELEMENT_DESC desc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32_FLOAT, 0, offsetof(Vertex, material), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = device_->CreateInputLayout(desc, ARRAYSIZE(desc), vs->GetBufferPointer(), vs->GetBufferSize(), &inputLayout_);
        if (FAILED(hr)) return false;
        D3D11_INPUT_ELEMENT_DESC overlayDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(OverlayVertex, pos), D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(OverlayVertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0}
        };
        hr = device_->CreateInputLayout(overlayDesc, ARRAYSIZE(overlayDesc), overlayVs->GetBufferPointer(), overlayVs->GetBufferSize(), &overlayInputLayout_);
        return SUCCEEDED(hr);
    }

    bool CreateStates() {
        D3D11_SAMPLER_DESC sd{};
        sd.Filter = D3D11_FILTER_ANISOTROPIC;
        sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sd.MaxAnisotropy = 8;
        sd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&sd, &sampler_))) return false;

        D3D11_SAMPLER_DESC shadowSd{};
        shadowSd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        shadowSd.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        shadowSd.BorderColor[0] = 1.0f;
        shadowSd.BorderColor[1] = 1.0f;
        shadowSd.BorderColor[2] = 1.0f;
        shadowSd.BorderColor[3] = 1.0f;
        shadowSd.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
        shadowSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&shadowSd, &shadowSampler_))) return false;

        D3D11_SAMPLER_DESC postSd{};
        postSd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        postSd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        postSd.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&postSd, &postSampler_))) return false;

        D3D11_RASTERIZER_DESC rd{};
        rd.FillMode = D3D11_FILL_SOLID;
        rd.CullMode = D3D11_CULL_NONE;
        rd.DepthClipEnable = TRUE;
        if (FAILED(device_->CreateRasterizerState(&rd, &rasterState_))) return false;

        D3D11_RASTERIZER_DESC shadowRd = rd;
        shadowRd.DepthBias = 32;
        shadowRd.SlopeScaledDepthBias = 1.6f;
        shadowRd.DepthBiasClamp = 0.0025f;
        if (FAILED(device_->CreateRasterizerState(&shadowRd, &shadowRasterState_))) return false;

        D3D11_DEPTH_STENCIL_DESC dsd{};
        dsd.DepthEnable = TRUE;
        dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        if (FAILED(device_->CreateDepthStencilState(&dsd, &depthState_))) return false;
        D3D11_DEPTH_STENCIL_DESC lessDsd = dsd;
        lessDsd.DepthFunc = D3D11_COMPARISON_LESS;
        if (FAILED(device_->CreateDepthStencilState(&lessDsd, &depthLessState_))) return false;
        D3D11_DEPTH_STENCIL_DESC readOnlyDsd = dsd;
        readOnlyDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        if (FAILED(device_->CreateDepthStencilState(&readOnlyDsd, &depthReadOnlyState_))) return false;
        D3D11_DEPTH_STENCIL_DESC overlayDsd{};
        overlayDsd.DepthEnable = FALSE;
        overlayDsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        overlayDsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
        if (FAILED(device_->CreateDepthStencilState(&overlayDsd, &depthDisabledState_))) return false;

        D3D11_BLEND_DESC bd{};
        bd.RenderTarget[0].BlendEnable = TRUE;
        bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        return SUCCEEDED(device_->CreateBlendState(&bd, &alphaBlend_));
    }

    bool CreateShadowResources() {
        shadowMapSize_ = static_cast<UINT>(std::clamp(settings_.flashlightShadowMapSize, 512, 4096));

        D3D11_TEXTURE2D_DESC td{};
        td.Width = shadowMapSize_;
        td.Height = shadowMapSize_;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R32_TYPELESS;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        if (FAILED(device_->CreateTexture2D(&td, nullptr, &shadowDepth_))) return false;

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        if (FAILED(device_->CreateDepthStencilView(shadowDepth_.Get(), &dsvDesc, &shadowDsv_))) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        return SUCCEEDED(device_->CreateShaderResourceView(shadowDepth_.Get(), &srvDesc, &shadowSrv_));
    }

    bool CreateConstantBuffer() {
        D3D11_BUFFER_DESC bd{};
        bd.ByteWidth = sizeof(SceneConstants);
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        return SUCCEEDED(device_->CreateBuffer(&bd, nullptr, &constantBuffer_));
    }

    bool CreateTextures() {
        StartupProfile profile(L"CreateTextures");
        const int width = kTextureSize;
        const int height = kTextureSize * kMaterialCount;
        std::vector<uint8_t> albedo(static_cast<size_t>(width * height * 4), 255);
        std::vector<uint8_t> normal(static_cast<size_t>(width * height * 4), 255);
        std::vector<uint8_t> props(static_cast<size_t>(width * height * 4), 255);
        profile.Mark(L"AllocateBaseArrays");

        auto makeSrv = [&](const std::vector<uint8_t>& pixels, ComPtr<ID3D11ShaderResourceView>& srv) {
            UINT mipLevels = 1;
            for (int s = kTextureSize; s > 1; s >>= 1) {
                ++mipLevels;
            }

            D3D11_TEXTURE2D_DESC td{};
            td.Width = width;
            td.Height = kTextureSize;
            td.MipLevels = mipLevels;
            td.ArraySize = kMaterialCount;
            td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            td.SampleDesc.Count = 1;
            td.Usage = D3D11_USAGE_DEFAULT;
            td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            td.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
            ComPtr<ID3D11Texture2D> tex;
            HRESULT hr = device_->CreateTexture2D(&td, nullptr, &tex);
            if (FAILED(hr)) return false;
            for (UINT slice = 0; slice < kMaterialCount; ++slice) {
                const uint8_t* src = pixels.data() + static_cast<size_t>(slice) * kTextureSize * width * 4;
                UINT subresource = D3D11CalcSubresource(0, slice, mipLevels);
                context_->UpdateSubresource(tex.Get(), subresource, nullptr, src, width * 4, 0);
            }

            D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
            sd.Format = td.Format;
            sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            sd.Texture2DArray.MostDetailedMip = 0;
            sd.Texture2DArray.MipLevels = mipLevels;
            sd.Texture2DArray.FirstArraySlice = 0;
            sd.Texture2DArray.ArraySize = kMaterialCount;
            hr = device_->CreateShaderResourceView(tex.Get(), &sd, &srv);
            if (FAILED(hr)) return false;
            context_->GenerateMips(srv.Get());
            return true;
        };

        uint64_t textureHash = TextureCacheHash();
        if (LoadTextureCache(textureHash, albedo, normal, props)) {
            profile.Mark(L"LoadTextureCache");
            ReportStartupActivity(L"Loading textures", L"Loaded cached material atlas. Creating GPU texture views.");
            if (!makeSrv(albedo, albedoSrv_)) return false;
            profile.Mark(L"CreateAlbedoSRV");
            if (!makeSrv(normal, normalSrv_)) return false;
            profile.Mark(L"CreateNormalSRV");
            if (!makeSrv(props, materialPropsSrv_)) return false;
            profile.Mark(L"CreateMaterialPropsSRV");
            return true;
        }
        profile.Mark(L"TextureCacheMiss");
        ReportStartupActivity(L"Loading textures", L"Texture cache miss. Generating material atlas.");

        std::vector<float> heights(static_cast<size_t>(width * height), 0.5f);
        std::vector<uint8_t> externalNormal(static_cast<size_t>(width * height * 4), 0);
        std::vector<uint8_t> hasExternalNormal(static_cast<size_t>(width * height), 0);
        for (int m = 0; m < kMaterialCount; ++m) {
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = m * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    props[i + 0] = 255; // AO
                    props[i + 1] = 178; // roughness
                    props[i + 2] = 0;
                    props[i + 3] = 255;
                }
            }
        }
        profile.Mark(L"AllocateWorkArrays");

        auto setPixel = [&](int material, int x, int y, float r, float g, float b, float a, float h) {
            int gy = material * kTextureSize + y;
            size_t i = static_cast<size_t>((gy * width + x) * 4);
            albedo[i + 0] = Byte(r);
            albedo[i + 1] = Byte(g);
            albedo[i + 2] = Byte(b);
            albedo[i + 3] = Byte(a);
            heights[static_cast<size_t>(gy * width + x)] = Clamp01(h);
        };

        auto applyAlbedo = [&](int material, const ImageRGBA& img) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    albedo[dst + 0] = img.pixels[src + 0];
                    albedo[dst + 1] = img.pixels[src + 1];
                    albedo[dst + 2] = img.pixels[src + 2];
                    albedo[dst + 3] = img.pixels[src + 3];
                }
            }
        };

        auto applyHeight = [&](int material, const ImageRGBA& img) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    float h = img.pixels[src] / 255.0f;
                    heights[static_cast<size_t>(gy * width + x)] = h;
                }
            }
        };

        auto applyNormal = [&](int material, const ImageRGBA& img) {
            if (!img.Valid()) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    externalNormal[dst + 0] = img.pixels[src + 0];
                    externalNormal[dst + 1] = img.pixels[src + 1];
                    externalNormal[dst + 2] = img.pixels[src + 2];
                    externalNormal[dst + 3] = 255;
                    hasExternalNormal[static_cast<size_t>(gy * width + x)] = 1;
                }
            }
        };

        auto applyScalarProp = [&](int material, const ImageRGBA& img, int channel) {
            if (!img.Valid() || channel < 0 || channel > 3) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    size_t src = static_cast<size_t>((y * img.width + x) * 4);
                    size_t dst = static_cast<size_t>((gy * width + x) * 4);
                    props[dst + channel] = img.pixels[src];
                }
            }
        };

        auto loadPbrMaterial = [&](int material, const wchar_t* stem) {
            ImageRGBA img;
            std::wstring base(stem);
            if (base.empty()) return;
            if (LoadImageWic(ResolveAsset(settings_, base + L"_color_4k.jpg"), kTextureSize, kTextureSize, img)) {
                applyAlbedo(material, img);
            }
            if (LoadImageWic(ResolveAsset(settings_, base + L"_height_4k.png"), kTextureSize, kTextureSize, img)) {
                applyHeight(material, img);
            }
            std::filesystem::path normalPath = ResolveAsset(settings_, base + L"_normal_directx_4k.png");
            std::error_code ec;
            uintmax_t normalSize = std::filesystem::exists(normalPath, ec) ? std::filesystem::file_size(normalPath, ec) : 0;
            if (settings_.useExternalNormals && normalSize > 0 &&
                (material == 15 || normalSize <= static_cast<uintmax_t>(settings_.maxNormalMapMB) * 1024ull * 1024ull) &&
                LoadImageWic(normalPath, kTextureSize, kTextureSize, img)) {
                applyNormal(material, img);
            }
            if (LoadImageWic(ResolveAsset(settings_, base + L"_ao_4k.jpg"), kTextureSize, kTextureSize, img)) {
                applyScalarProp(material, img, 0);
            }
            if (LoadImageWic(ResolveAsset(settings_, base + L"_roughness_4k.jpg"), kTextureSize, kTextureSize, img)) {
                applyScalarProp(material, img, 1);
            }
        };

        ReportStartupActivity(L"Loading textures", L"Generating procedural material atlas.");
        for (int y = 0; y < kTextureSize; ++y) {
            for (int x = 0; x < kTextureSize; ++x) {
                float u = static_cast<float>(x) / kTextureSize;
                float v = static_cast<float>(y) / kTextureSize;
                float n1 = FractalNoise(u * 8.0f, v * 8.0f, 13);
                float stains = FractalNoise(u * 2.0f + 31.0f, v * 3.0f, 33);
                float seam = std::min(std::abs(std::fmod(u * 4.0f, 1.0f) - 0.02f), std::abs(std::fmod(v * 2.0f, 1.0f) - 0.02f));
                float groove = SmoothStep(0.035f, 0.0f, seam);
                float grime = SmoothStep(0.4f, 1.0f, v) * 0.18f + SmoothStep(0.72f, 0.95f, stains) * 0.23f;
                float pattern = std::sin(u * 140.0f) * std::sin(v * 90.0f) * 0.015f;
                setPixel(0, x, y,
                    0.82f + pattern - grime * 0.82f + n1 * 0.055f,
                    0.68f + pattern - grime * 0.72f + n1 * 0.045f,
                    0.34f + pattern - grime * 0.42f + n1 * 0.018f,
                    1.0f,
                    0.54f - groove * 0.23f + n1 * 0.12f);

                float carpet = FractalNoise(u * 28.0f, v * 28.0f, 71);
                float tileGroove = std::max(SmoothStep(0.025f, 0.0f, std::abs(std::fmod(u * 4.0f, 1.0f) - 0.02f)),
                                            SmoothStep(0.025f, 0.0f, std::abs(std::fmod(v * 4.0f, 1.0f) - 0.02f)));
                float damp = SmoothStep(0.65f, 0.95f, FractalNoise(u * 3.0f + 11.0f, v * 3.0f + 19.0f, 92));
                setPixel(1, x, y,
                    0.60f + carpet * 0.11f - damp * 0.13f,
                    0.52f + carpet * 0.09f - damp * 0.10f,
                    0.30f + carpet * 0.055f - damp * 0.075f,
                    1.0f,
                    0.45f + carpet * 0.18f - tileGroove * 0.30f);

                float panelX = std::abs(std::fmod(u * 2.0f, 1.0f) - 0.01f);
                float panelY = std::abs(std::fmod(v * 2.0f, 1.0f) - 0.01f);
                float grid = std::max(SmoothStep(0.03f, 0.0f, panelX), SmoothStep(0.03f, 0.0f, panelY));
                float strip = SmoothStep(0.035f, 0.0f, std::abs(std::fmod(u * 2.0f, 1.0f) - 0.5f)) *
                              SmoothStep(0.35f, 0.05f, std::abs(std::fmod(v * 2.0f, 1.0f) - 0.5f));
                float speck = FractalNoise(u * 60.0f, v * 60.0f, 44);
                setPixel(2, x, y,
                    0.76f + strip * 0.20f - grid * 0.13f + speck * 0.030f,
                    0.64f + strip * 0.18f - grid * 0.12f + speck * 0.026f,
                    0.34f + strip * 0.10f - grid * 0.08f + speck * 0.018f,
                    1.0f,
                    0.48f - grid * 0.28f + speck * 0.08f);

                float edge = std::max(SmoothStep(0.055f, 0.0f, std::min(u, 1.0f - u)),
                                      SmoothStep(0.055f, 0.0f, std::min(v, 1.0f - v)));
                float lens = SmoothStep(0.42f, 0.0f, std::abs(v - 0.5f)) * SmoothStep(0.46f, 0.0f, std::abs(u - 0.5f));
                setPixel(3, x, y,
                    0.72f + lens * 0.24f - edge * 0.18f,
                    0.76f + lens * 0.23f - edge * 0.16f,
                    0.70f + lens * 0.20f - edge * 0.12f,
                    1.0f,
                    0.5f);
                setPixel(5, x, y,
                    0.018f + lens * 0.012f,
                    0.018f + lens * 0.012f,
                    0.016f + lens * 0.010f,
                    1.0f,
                    0.5f);

                auto lineMask = [&](float value, float center, float width) {
                    return SmoothStep(width, 0.0f, std::abs(value - center));
                };
                float torso = SmoothStep(1.0f, 0.64f, ((u - 0.50f) / 0.135f) * ((u - 0.50f) / 0.135f) + ((v - 0.57f) / 0.35f) * ((v - 0.57f) / 0.35f));
                float waist = SmoothStep(1.0f, 0.70f, ((u - 0.50f) / 0.085f) * ((u - 0.50f) / 0.085f) + ((v - 0.77f) / 0.25f) * ((v - 0.77f) / 0.25f));
                float head = SmoothStep(1.0f, 0.63f, ((u - 0.5f) / 0.135f) * ((u - 0.5f) / 0.135f) + ((v - 0.245f) / 0.145f) * ((v - 0.245f) / 0.145f));
                float neck = lineMask(u, 0.50f, 0.055f) * SmoothStep(0.31f, 0.48f, v) * (1.0f - SmoothStep(0.52f, 0.60f, v));
                float armL = lineMask(u, 0.34f + (v - 0.34f) * 0.18f, 0.035f) * SmoothStep(0.30f, 0.56f, v) * (1.0f - SmoothStep(0.83f, 0.93f, v));
                float armR = lineMask(u, 0.66f - (v - 0.34f) * 0.18f, 0.035f) * SmoothStep(0.30f, 0.56f, v) * (1.0f - SmoothStep(0.83f, 0.93f, v));
                float clawL = lineMask(u, 0.29f, 0.030f) * SmoothStep(0.76f, 0.88f, v) * (1.0f - SmoothStep(0.90f, 0.98f, v));
                float clawR = lineMask(u, 0.71f, 0.030f) * SmoothStep(0.76f, 0.88f, v) * (1.0f - SmoothStep(0.90f, 0.98f, v));
                float legL = lineMask(u, 0.44f - (v - 0.74f) * 0.09f, 0.040f) * SmoothStep(0.68f, 0.82f, v);
                float legR = lineMask(u, 0.56f + (v - 0.74f) * 0.09f, 0.040f) * SmoothStep(0.68f, 0.82f, v);
                float antlerL = lineMask(u, 0.43f - (0.22f - v) * 0.95f, 0.026f) * SmoothStep(0.06f, 0.16f, v) * (1.0f - SmoothStep(0.22f, 0.29f, v));
                float antlerR = lineMask(u, 0.57f + (0.22f - v) * 0.95f, 0.026f) * SmoothStep(0.06f, 0.16f, v) * (1.0f - SmoothStep(0.22f, 0.29f, v));
                float tineL = lineMask(u, 0.31f, 0.020f) * SmoothStep(0.07f, 0.14f, v) * (1.0f - SmoothStep(0.18f, 0.24f, v));
                float tineR = lineMask(u, 0.69f, 0.020f) * SmoothStep(0.07f, 0.14f, v) * (1.0f - SmoothStep(0.18f, 0.24f, v));
                float rib = std::max(std::max(lineMask(v, 0.50f, 0.012f), lineMask(v, 0.56f, 0.012f)), lineMask(v, 0.62f, 0.012f)) * lineMask(u, 0.50f, 0.13f);
                float skinNoise = FractalNoise(u * 18.0f, v * 24.0f, 213);
                float vein = SmoothStep(0.030f, 0.0f, std::abs(std::fmod(u * 9.0f + v * 2.7f, 1.0f) - 0.06f)) *
                    SmoothStep(0.46f, 0.92f, FractalNoise(u * 5.0f + 9.0f, v * 8.0f, 214));
                float scar = SmoothStep(0.018f, 0.0f, std::abs(std::fmod(u * 13.0f - v * 4.0f, 1.0f) - 0.04f));
                setPixel(4, x, y,
                    0.018f + skinNoise * 0.016f + vein * 0.018f + scar * 0.012f,
                    0.017f + skinNoise * 0.014f + vein * 0.014f + scar * 0.010f,
                    0.016f + skinNoise * 0.012f + vein * 0.010f + scar * 0.009f,
                    1.0f,
                    0.43f + skinNoise * 0.18f + scar * 0.12f);

                float doorPanel = std::max(
                    SmoothStep(0.025f, 0.0f, std::abs(u - 0.12f)),
                    SmoothStep(0.025f, 0.0f, std::abs(u - 0.88f)));
                doorPanel = std::max(doorPanel, std::max(
                    SmoothStep(0.025f, 0.0f, std::abs(v - 0.16f)),
                    SmoothStep(0.025f, 0.0f, std::abs(v - 0.84f))));
                float doorGrime = SmoothStep(0.62f, 0.96f, FractalNoise(u * 5.0f + 17.0f, v * 8.0f + 4.0f, 19));
                setPixel(6, x, y,
                    0.23f - doorPanel * 0.04f - doorGrime * 0.05f,
                    0.18f - doorPanel * 0.035f - doorGrime * 0.04f,
                    0.12f - doorPanel * 0.02f,
                    1.0f,
                    0.50f - doorPanel * 0.08f);

                setPixel(7, x, y,
                    0.02f,
                    0.42f,
                    0.12f,
                    1.0f,
                    0.5f);

                float plasticGrain = FractalNoise(u * 14.0f, v * 13.0f, 91);
                setPixel(8, x, y,
                    0.66f + plasticGrain * 0.065f,
                    0.61f + plasticGrain * 0.055f,
                    0.47f + plasticGrain * 0.040f,
                    1.0f,
                    0.50f + plasticGrain * 0.05f);

                float paperEdge = std::max(SmoothStep(0.035f, 0.0f, std::min(u, 1.0f - u)),
                                           SmoothStep(0.035f, 0.0f, std::min(v, 1.0f - v)));
                float paperStain = SmoothStep(0.62f, 0.95f, FractalNoise(u * 4.0f + 4.0f, v * 6.0f + 15.0f, 21));
                setPixel(9, x, y,
                    0.82f - paperEdge * 0.12f - paperStain * 0.22f,
                    0.80f - paperEdge * 0.11f - paperStain * 0.18f,
                    0.70f - paperEdge * 0.08f - paperStain * 0.13f,
                    1.0f,
                    0.50f);

                float scratch = std::max(SmoothStep(0.018f, 0.0f, std::abs(std::fmod(u * 19.0f + v * 3.0f, 1.0f) - 0.04f)),
                                         SmoothStep(0.014f, 0.0f, std::abs(std::fmod(v * 23.0f - u * 2.0f, 1.0f) - 0.06f)));
                float rust = SmoothStep(0.72f, 0.96f, FractalNoise(u * 5.0f + 19.0f, v * 6.0f - 7.0f, 118));
                setPixel(10, x, y,
                    0.055f + n1 * 0.025f + scratch * 0.040f + rust * 0.055f,
                    0.058f + n1 * 0.023f + scratch * 0.034f + rust * 0.025f,
                    0.062f + n1 * 0.020f + scratch * 0.030f - rust * 0.008f,
                    1.0f,
                    0.46f + n1 * 0.07f + scratch * 0.10f - rust * 0.08f);

                float wet = SmoothStep(0.35f, 0.0f, std::abs(u - 0.5f)) * SmoothStep(0.50f, 0.02f, std::abs(v - 0.5f));
                wet = std::max(wet, SmoothStep(0.78f, 1.0f, FractalNoise(u * 6.0f, v * 6.0f, 137)));
                setPixel(11, x, y,
                    0.020f + wet * 0.030f,
                    0.026f + wet * 0.036f,
                    0.024f + wet * 0.032f,
                    1.0f,
                    0.60f + wet * 0.15f);

                float eyeDx = (u - 0.5f) / 0.34f;
                float eyeDy = (v - 0.5f) / 0.26f;
                float eyeGlow = std::exp(-(eyeDx * eyeDx + eyeDy * eyeDy) * 3.3f);
                float hot = std::exp(-(eyeDx * eyeDx + eyeDy * eyeDy) * 28.0f);
                float ragged = 0.78f + FractalNoise(u * 11.0f, v * 9.0f, 222) * 0.22f;
                float eyeAlpha = Clamp01((eyeGlow * 0.98f + hot * 1.18f) * ragged);
                setPixel(12, x, y,
                    1.0f,
                    0.060f + hot * 0.26f,
                    0.020f,
                    eyeAlpha,
                    0.58f);
            }
        }
        profile.Mark(L"ProceduralMaterials");

        auto fillRuntimeMaterial = [&](int material, float r, float g, float b, float roughness) {
            if (material < 0 || material >= kMaterialCount) return;
            for (int y = 0; y < kTextureSize; ++y) {
                int gy = material * kTextureSize + y;
                for (int x = 0; x < kTextureSize; ++x) {
                    setPixel(material, x, y, r, g, b, 1.0f, 0.50f);
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    props[i + 1] = Byte(roughness);
                }
            }
        };
        fillRuntimeMaterial(16, 0.62f, 0.58f, 0.50f, 0.62f);
        fillRuntimeMaterial(17, 0.13f, 0.145f, 0.15f, 0.72f);
        fillRuntimeMaterial(18, 0.012f, 0.012f, 0.012f, 0.58f);
        fillRuntimeMaterial(19, 0.70f, 0.68f, 0.62f, 0.50f);
        fillRuntimeMaterial(20, 0.29f, 0.50f, 0.64f, 0.82f);
        fillRuntimeMaterial(21, 0.78f, 0.78f, 0.74f, 0.42f);
        fillRuntimeMaterial(22, 0.38f, 0.36f, 0.34f, 0.62f);
        fillRuntimeMaterial(23, 0.035f, 0.034f, 0.031f, 0.70f);
        fillRuntimeMaterial(24, 0.78f, 0.74f, 0.62f, 0.58f);

        auto loadRuntimeAlbedo = [&](int material, const wchar_t* relativePath) {
            if (material < 0 || material >= kMaterialCount) return;
            ImageRGBA img;
            if (LoadImageWic(ResolveConfiguredAssetPath(relativePath), kTextureSize, kTextureSize, img)) {
                applyAlbedo(material, img);
            }
        };

        {
            ReportStartupActivity(L"Loading textures", L"Loading external PBR textures.");
            ScopedCom com;
            if (com.Ok()) {
                loadPbrMaterial(0, settings_.wallStem.c_str());
                loadPbrMaterial(1, settings_.floorStem.c_str());
                loadPbrMaterial(2, settings_.ceilingStem.c_str());
                loadPbrMaterial(15, settings_.fleshStem.c_str());
                loadRuntimeAlbedo(7, L"assets\\models\\runtime\\textures\\emergency_exit_sign_diffuse.jpeg");
                loadRuntimeAlbedo(16, L"assets\\models\\runtime\\textures\\office_chair_modern_diffuse.jpg");
                loadRuntimeAlbedo(19, L"assets\\models\\runtime\\textures\\office_chair_classic_2209.jpg");
                loadRuntimeAlbedo(20, L"assets\\models\\runtime\\textures\\office_chair_classic_textiles.png");
                loadRuntimeAlbedo(22, L"assets\\models\\runtime\\textures\\office_chair_task_diffuse.png");
            }
        }
        profile.Mark(L"LoadExternalPBRs");

        ReportStartupActivity(L"Loading textures", L"Building normal and material property maps.");
        for (int m = 0; m < kMaterialCount; ++m) {
            for (int y = 0; y < kTextureSize; ++y) {
                for (int x = 0; x < kTextureSize; ++x) {
                    auto hAt = [&](int sx, int sy) {
                        sx = (sx + kTextureSize) % kTextureSize;
                        sy = (sy + kTextureSize) % kTextureSize;
                        return heights[static_cast<size_t>((m * kTextureSize + sy) * width + sx)];
                    };
                    float hl = hAt(x - 1, y);
                    float hr = hAt(x + 1, y);
                    float hu = hAt(x, y - 1);
                    float hd = hAt(x, y + 1);
                    XMVECTOR n = XMVector3Normalize(XMVectorSet((hl - hr) * 3.1f, (hu - hd) * 3.1f, 1.0f, 0.0f));
                    XMFLOAT3 nf;
                    XMStoreFloat3(&nf, n);
                    int gy = m * kTextureSize + y;
                    size_t i = static_cast<size_t>((gy * width + x) * 4);
                    if (hasExternalNormal[static_cast<size_t>(gy * width + x)]) {
                        normal[i + 0] = externalNormal[i + 0];
                        normal[i + 1] = externalNormal[i + 1];
                        normal[i + 2] = externalNormal[i + 2];
                    } else {
                        normal[i + 0] = Byte(nf.x * 0.5f + 0.5f);
                        normal[i + 1] = Byte(nf.y * 0.5f + 0.5f);
                        normal[i + 2] = Byte(nf.z * 0.5f + 0.5f);
                    }
                    normal[i + 3] = Byte(hAt(x, y));
                }
            }
        }
        profile.Mark(L"BuildNormalHeightArray");
        SaveTextureCache(textureHash, albedo, normal, props);
        profile.Mark(L"SaveTextureCache");

        ReportStartupActivity(L"Loading textures", L"Creating GPU texture views.");
        if (!makeSrv(albedo, albedoSrv_)) return false;
        profile.Mark(L"CreateAlbedoSRV");
        if (!makeSrv(normal, normalSrv_)) return false;
        profile.Mark(L"CreateNormalSRV");
        if (!makeSrv(props, materialPropsSrv_)) return false;
        profile.Mark(L"CreateMaterialPropsSRV");
        return true;
    }

    bool CreateFlashlightPatternTexture() {
        if (!device_) return false;
        constexpr int kPatternSize = 512;
        ImageRGBA img;
        bool loaded = false;
        {
            ScopedCom com;
            if (com.Ok()) {
                loaded = LoadImageWic(ResolveConfiguredAssetPath(L"assets\\PBRs\\downloads\\t_flashlightpattern.png"),
                    kPatternSize, kPatternSize, img);
            }
        }
        if (!loaded) {
            img.width = kPatternSize;
            img.height = kPatternSize;
            img.pixels.assign(static_cast<size_t>(kPatternSize * kPatternSize * 4), 255);
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(img.width);
        td.Height = static_cast<UINT>(img.height);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = img.pixels.data();
        init.SysMemPitch = static_cast<UINT>(img.width * 4);

        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = device_->CreateTexture2D(&td, &init, &tex);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = device_->CreateShaderResourceView(tex.Get(), &sd, &flashlightPatternSrv_);
        return SUCCEEDED(hr);
    }

    bool CreateMazeMaskTexture() {
        if (!device_ || maze_.w <= 0 || maze_.h <= 0 || maze_.open.empty()) return false;
        mazeSrv_.Reset();

        std::vector<uint8_t> mask(static_cast<size_t>(maze_.w * maze_.h), 0);
        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                mask[static_cast<size_t>(y * maze_.w + x)] = maze_.IsOpen(x, y) ? 255 : 0;
            }
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(maze_.w);
        td.Height = static_cast<UINT>(maze_.h);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_IMMUTABLE;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = mask.data();
        init.SysMemPitch = static_cast<UINT>(maze_.w);

        ComPtr<ID3D11Texture2D> tex;
        HRESULT hr = device_->CreateTexture2D(&td, &init, &tex);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = device_->CreateShaderResourceView(tex.Get(), &sd, &mazeSrv_);
        return SUCCEEDED(hr);
    }

    bool CreateLampDamageTexture() {
        if (!device_ || maze_.w <= 0 || maze_.h <= 0) return false;
        lampDamageTexture_.Reset();
        lampDamageSrv_.Reset();

        const size_t pixelCount = static_cast<size_t>(maze_.w) * static_cast<size_t>(maze_.h);
        if (lampDamagePixels_.size() != pixelCount) {
            lampDamagePixels_.assign(pixelCount, 0);
        }

        D3D11_TEXTURE2D_DESC td{};
        td.Width = static_cast<UINT>(maze_.w);
        td.Height = static_cast<UINT>(maze_.h);
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA init{};
        init.pSysMem = lampDamagePixels_.data();
        init.SysMemPitch = static_cast<UINT>(maze_.w);

        HRESULT hr = device_->CreateTexture2D(&td, &init, &lampDamageTexture_);
        if (FAILED(hr)) return false;

        D3D11_SHADER_RESOURCE_VIEW_DESC sd{};
        sd.Format = td.Format;
        sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        sd.Texture2D.MostDetailedMip = 0;
        sd.Texture2D.MipLevels = 1;
        hr = device_->CreateShaderResourceView(lampDamageTexture_.Get(), &sd, &lampDamageSrv_);
        if (FAILED(hr)) return false;
        lampDamageDirty_ = false;
        return true;
    }

    void AddQuad(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                 XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                 XMFLOAT3 n, XMFLOAT3 t, float material, float uScale, float vScale) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, {0.0f, 0.0f}, material});
        v.push_back({b, n, t, {uScale, 0.0f}, material});
        v.push_back({c, n, t, {uScale, vScale}, material});
        v.push_back({d, n, t, {0.0f, vScale}, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    void AddQuadUV(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                   XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                   XMFLOAT3 n, XMFLOAT3 t, XMFLOAT2 auv, XMFLOAT2 buv, XMFLOAT2 cuv, XMFLOAT2 duv,
                   float material) {
        uint32_t base = static_cast<uint32_t>(v.size());
        v.push_back({a, n, t, auv, material});
        v.push_back({b, n, t, buv, material});
        v.push_back({c, n, t, cuv, material});
        v.push_back({d, n, t, duv, material});
        i.insert(i.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    }

    static XMFLOAT3 Add3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x + b.x, a.y + b.y, a.z + b.z};
    }

    static XMFLOAT3 Scale3(XMFLOAT3 a, float s) {
        return {a.x * s, a.y * s, a.z * s};
    }

    static XMFLOAT3 Sub3(XMFLOAT3 a, XMFLOAT3 b) {
        return {a.x - b.x, a.y - b.y, a.z - b.z};
    }

    static float Dot3(XMFLOAT3 a, XMFLOAT3 b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static XMFLOAT3 Cross3(XMFLOAT3 a, XMFLOAT3 b) {
        return {
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        };
    }

    static float Length3(XMFLOAT3 a) {
        return std::sqrt(std::max(0.0f, Dot3(a, a)));
    }

    static XMFLOAT3 Normalize3(XMFLOAT3 a, XMFLOAT3 fallback = {0.0f, 1.0f, 0.0f}) {
        float len = Length3(a);
        if (len <= 0.0001f) return fallback;
        return Scale3(a, 1.0f / len);
    }

    static XMFLOAT3 Lerp3(XMFLOAT3 a, XMFLOAT3 b, float t) {
        return {Lerp(a.x, b.x, t), Lerp(a.y, b.y, t), Lerp(a.z, b.z, t)};
    }

    static XMFLOAT3 RotateYVec(XMFLOAT3 a, float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        return {a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
    }

    static XMFLOAT3 OrientedOffset(XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward, float x, float y, float z) {
        return Add3(Add3(Scale3(right, x), Scale3(up, y)), Scale3(forward, z));
    }

    void AddOrientedBox(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        auto p = [&](float x, float y, float z) {
            return Add3(center, OrientedOffset(right, up, forward, x * half.x, y * half.y, z * half.z));
        };
        auto face = [&](XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c0, XMFLOAT3 d, XMFLOAT3 n, XMFLOAT3 t) {
            AddQuadUV(v, i, a, b, c0, d, n, t, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
        };
        face(p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right);
        face(p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f));
        face(p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f));
        face(p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward);
        face(p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right);
        face(p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right);
    }

    void AddFloorCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                      XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, 1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }

    void AddCeilingCard(std::vector<Vertex>& v, std::vector<uint32_t>& i,
                        XMFLOAT3 center, float width, float depth, float yaw, float y, float material) {
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 forward{s, 0.0f, c};
        XMFLOAT3 a = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 b = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward, -depth * 0.5f)));
        XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  width * 0.5f), Scale3(forward,  depth * 0.5f)));
        XMFLOAT3 d = Add3(center, Add3(Scale3(right, -width * 0.5f), Scale3(forward,  depth * 0.5f)));
        a.y = b.y = c0.y = d.y = y;
        AddQuadUV(v, i, a, b, c0, d, {0, -1, 0}, right, {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
    }

    bool AppendStaticPropMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                              const StaticPropMesh& mesh, XMFLOAT3 origin, float yaw,
                              float scaleX, float scaleY, float scaleZ,
                              float pitch = 0.0f, float materialOverride = -1.0f,
                              std::vector<uint32_t>* shadowIndices = nullptr) const {
        if (mesh.vertices.empty()) return false;
        if (vertices.size() + mesh.vertices.size() > static_cast<size_t>(std::numeric_limits<uint32_t>::max())) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);
        const float invX = 1.0f / std::max(0.001f, std::abs(scaleX));
        const float invY = 1.0f / std::max(0.001f, std::abs(scaleY));
        const float invZ = 1.0f / std::max(0.001f, std::abs(scaleZ));
        XMFLOAT3 meshSpan{
            std::max(0.001f, mesh.max.x - mesh.min.x),
            std::max(0.001f, mesh.max.y - mesh.min.y),
            std::max(0.001f, mesh.max.z - mesh.min.z)
        };
        auto generatedUv = [&](const Vertex& src) {
            float nx = (src.pos.x - mesh.min.x) / meshSpan.x;
            float ny = (src.pos.y - mesh.min.y) / meshSpan.y;
            float nz = (src.pos.z - mesh.min.z) / meshSpan.z;
            XMFLOAT3 nAbs{std::abs(src.normal.x), std::abs(src.normal.y), std::abs(src.normal.z)};
            XMFLOAT2 uv = nAbs.y >= nAbs.x && nAbs.y >= nAbs.z
                ? XMFLOAT2{nx, nz}
                : (nAbs.x >= nAbs.z ? XMFLOAT2{nz, 1.0f - ny} : XMFLOAT2{nx, 1.0f - ny});
            int materialId = std::clamp(static_cast<int>(std::floor(src.material)), 0, kMaterialCount - 1);
            if (materialId == 22) {
                uv = {0.20f + uv.x * 0.60f, 0.28f + uv.y * 0.36f};
            }
            return uv;
        };

        uint32_t base = static_cast<uint32_t>(vertices.size());
        vertices.reserve(vertices.size() + mesh.vertices.size());
        indices.reserve(indices.size() + mesh.vertices.size());
        for (const Vertex& src : mesh.vertices) {
            XMFLOAT3 pos = Add3(origin, Add3(Scale3(right, src.pos.x * scaleX),
                Add3(Scale3(up, src.pos.y * scaleY), Scale3(forward, src.pos.z * scaleZ))));
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, src.normal.x * invX),
                Add3(Scale3(up, src.normal.y * invY), Scale3(forward, src.normal.z * invZ))), up);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, src.tangent.x * scaleX),
                Add3(Scale3(up, src.tangent.y * scaleY), Scale3(forward, src.tangent.z * scaleZ))), right);
            XMFLOAT2 uv = mesh.generatedUvFallback ? generatedUv(src) : src.uv;
            vertices.push_back({pos, normal, tangent, uv, materialOverride >= 0.0f ? materialOverride : src.material});
        }
        for (uint32_t n = 0; n < static_cast<uint32_t>(mesh.vertices.size()); ++n) {
            uint32_t idx = base + n;
            indices.push_back(idx);
            if (shadowIndices) shadowIndices->push_back(idx);
        }
        return true;
    }

    bool AppendStaticPropMeshGrounded(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                                      const StaticPropMesh& mesh, XMFLOAT3 floorCenter, float yaw,
                                      float scaleX, float scaleY, float scaleZ,
                                      float pitch = 0.0f, float materialOverride = -1.0f,
                                      std::vector<uint32_t>* shadowIndices = nullptr) const {
        if (mesh.vertices.empty()) return false;

        float c = std::cos(yaw);
        float s = std::sin(yaw);
        XMFLOAT3 right{c, 0.0f, -s};
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{s, 0.0f, c};
        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        XMFLOAT3 up = Normalize3(Add3(Scale3(worldUp, cp), Scale3(forward, sp)), worldUp);
        forward = Normalize3(Add3(Scale3(forward, cp), Scale3(worldUp, -sp)), forward);

        float minY = std::numeric_limits<float>::max();
        for (const Vertex& src : mesh.vertices) {
            float y = right.y * src.pos.x * scaleX + up.y * src.pos.y * scaleY + forward.y * src.pos.z * scaleZ;
            minY = std::min(minY, y);
        }
        XMFLOAT3 origin{floorCenter.x, floorCenter.y - minY, floorCenter.z};
        return AppendStaticPropMesh(vertices, indices, mesh, origin, yaw, scaleX, scaleY, scaleZ, pitch, materialOverride, shadowIndices);
    }

    XMFLOAT2 FloorUv(float x, float z) const {
        return {x / settings_.floorTextureMeters, z / settings_.floorTextureMeters};
    }

    XMFLOAT2 CeilingUv(float x, float z) const {
        float scaleX = settings_.ceilingTextureMeters > 0.001f
            ? std::max(0.2f, settings_.ceilingTextureMeters)
            : std::max(0.2f, maze_.tileW);
        float scaleZ = settings_.ceilingTextureMeters > 0.001f
            ? std::max(0.2f, settings_.ceilingTextureMeters)
            : std::max(0.2f, maze_.tileD);
        float originX = -static_cast<float>(maze_.w) * maze_.tileW * 0.5f;
        float originZ = -static_cast<float>(maze_.h) * maze_.tileD * 0.5f;
        return {(x - originX) / scaleX, (z - originZ) / scaleZ};
    }

    XMFLOAT2 WallUvX(float x, float y) const {
        return {x / settings_.wallTextureMeters, y / settings_.wallTextureMeters};
    }

    XMFLOAT2 WallUvZ(float z, float y) const {
        return {z / settings_.wallTextureMeters, y / settings_.wallTextureMeters};
    }

    void CreateMazeMesh() {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<uint32_t> waterIndices;
        std::vector<uint32_t> transparentIndices;
        std::vector<uint32_t> propShadowIndices;
        vertices.reserve(maze_.w * maze_.h * 64);
        indices.reserve(maze_.w * maze_.h * 96);
        waterIndices.reserve(maze_.w * maze_.h * 6);
        transparentIndices.reserve(maze_.w * maze_.h * 12);
        propShadowIndices.reserve(maze_.w * maze_.h * 24);
        propLookPoints_.clear();
        sparkEmitters_.clear();
        runtimeLamps_.clear();
        lampDamagePixels_.assign(static_cast<size_t>(std::max(0, maze_.w * maze_.h)), 0);
        lampDamageDirty_ = true;
        steamEmitters_.clear();
        bloodScarePoints_.clear();
        exitSignLightPos_ = {};
        exitSignLightStrength_ = 0.0f;

        const float tileW = maze_.tileW;
        const float tileD = maze_.tileD;
        const float tileAvg = maze_.TileAverage();
        const float tileMin = maze_.TileMinimum();
        const float wallH = settings_.wallHeightMeters;
        float ox = -static_cast<float>(maze_.w) * tileW * 0.5f;
        float oz = -static_cast<float>(maze_.h) * tileD * 0.5f;

        struct ExitPortal {
            Tile tile{};
            int dx = 0;
            int dy = 1;
            float yaw = kPi;
            XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
            XMFLOAT3 right{-1.0f, 0.0f, 0.0f};
            XMFLOAT3 wallCenter{};
            float halfSpan = kTile * 0.5f;
            bool valid = false;
        };

        auto makeExitPortal = [&]() {
            ExitPortal portal{};
            portal.tile = maze_.exit;
            struct DoorSide { int dx; int dy; };
            std::vector<DoorSide> sides;
            Tile e = maze_.exit;
            if (e.y == maze_.h - 2) sides.push_back({0, 1});
            if (e.x == maze_.w - 2) sides.push_back({1, 0});
            if (e.y == 1) sides.push_back({0, -1});
            if (e.x == 1) sides.push_back({-1, 0});
            const DoorSide fallbackSides[] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
            sides.insert(sides.end(), std::begin(fallbackSides), std::end(fallbackSides));

            for (const DoorSide& side : sides) {
                if (maze_.IsOpen(e.x + side.dx, e.y + side.dy)) continue;
                XMFLOAT3 c = maze_.WorldCenter(e, 0.0f);
                portal.dx = side.dx;
                portal.dy = side.dy;
                portal.inward = {-static_cast<float>(side.dx), 0.0f, -static_cast<float>(side.dy)};
                portal.yaw = std::atan2(portal.inward.x, portal.inward.z);
                XMVECTOR rightVec = XMVector3Normalize(XMVector3Cross(XMVectorSet(0, 1, 0, 0), XMLoadFloat3(&portal.inward)));
                XMStoreFloat3(&portal.right, rightVec);
                portal.wallCenter = {c.x + side.dx * tileW * 0.5f, 0.0f, c.z + side.dy * tileD * 0.5f};
                portal.halfSpan = (side.dy != 0 ? tileW : tileD) * 0.5f;
                portal.valid = true;
                break;
            }
            return portal;
        };

        ExitPortal exitPortal = makeExitPortal();

        auto addFloorCeilingRun = [&](int y, int x0, int x1) {
            float z0 = oz + y * tileD;
            float z1 = z0 + tileD;
            for (int x = x0; x < x1; ++x) {
                float l = ox + x * tileW;
                float r = l + tileW;
                AddQuadUV(vertices, indices,
                    {l, 0, z1}, {r, 0, z1}, {r, 0, z0}, {l, 0, z0},
                    {0, 1, 0}, {1, 0, 0},
                    FloorUv(l, z1), FloorUv(r, z1), FloorUv(r, z0), FloorUv(l, z0), 1.0f);
                AddQuadUV(vertices, indices,
                    {l, wallH, z0}, {r, wallH, z0}, {r, wallH, z1}, {l, wallH, z1},
                    {0, -1, 0}, {1, 0, 0},
                    CeilingUv(l, z0), CeilingUv(r, z0), CeilingUv(r, z1), CeilingUv(l, z1), 2.0f);
            }
        };

        auto addNorthWallRun = [&](int y, int x0, int x1) {
            float z = oz + y * tileD;
            for (int x = x0; x < x1; ++x) {
                float l = ox + x * tileW;
                float r = l + tileW;
                AddQuadUV(vertices, indices,
                    {r, 0, z}, {l, 0, z}, {l, wallH, z}, {r, wallH, z},
                    {0, 0, 1}, {-1, 0, 0},
                    WallUvX(r, 0), WallUvX(l, 0), WallUvX(l, wallH), WallUvX(r, wallH), 0.0f);
            }
        };

        auto addSouthWallRun = [&](int y, int x0, int x1) {
            float z = oz + (y + 1) * tileD;
            for (int x = x0; x < x1; ++x) {
                float l = ox + x * tileW;
                float r = l + tileW;
                AddQuadUV(vertices, indices,
                    {l, 0, z}, {r, 0, z}, {r, wallH, z}, {l, wallH, z},
                    {0, 0, -1}, {1, 0, 0},
                    WallUvX(l, 0), WallUvX(r, 0), WallUvX(r, wallH), WallUvX(l, wallH), 0.0f);
            }
        };

        auto addWestWallRun = [&](int x, int y0, int y1) {
            float l = ox + x * tileW;
            for (int y = y0; y < y1; ++y) {
                float z0 = oz + y * tileD;
                float z1 = z0 + tileD;
                AddQuadUV(vertices, indices,
                    {l, 0, z0}, {l, 0, z1}, {l, wallH, z1}, {l, wallH, z0},
                    {1, 0, 0}, {0, 0, 1},
                    WallUvZ(z0, 0), WallUvZ(z1, 0), WallUvZ(z1, wallH), WallUvZ(z0, wallH), 0.0f);
            }
        };

        auto addEastWallRun = [&](int x, int y0, int y1) {
            float r = ox + (x + 1) * tileW;
            for (int y = y0; y < y1; ++y) {
                float z0 = oz + y * tileD;
                float z1 = z0 + tileD;
                AddQuadUV(vertices, indices,
                    {r, 0, z1}, {r, 0, z0}, {r, wallH, z0}, {r, wallH, z1},
                    {-1, 0, 0}, {0, 0, -1},
                    WallUvZ(z1, 0), WallUvZ(z0, 0), WallUvZ(z0, wallH), WallUvZ(z1, wallH), 0.0f);
            }
        };

        auto addExitDoorwayWall = [&]() {
            if (!exitPortal.valid) return;
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            float openingHalf = std::min(exitPortal.halfSpan * 0.68f, 0.76f);
            float doorwayTop = std::min(wallH - 0.18f, 2.38f);
            float vestibuleLength = std::min(
                std::max(tileAvg * 7.5f, settings_.fogEndMeters + tileAvg * 3.0f),
                tileAvg * 14.0f);
            float vestibuleHalf = std::min(exitPortal.halfSpan * 0.72f, 0.82f);

            auto wallPoint = [&](float along, float y, float push = 0.0f) {
                return Add3(exitPortal.wallCenter,
                    Add3(Scale3(exitPortal.right, along), Add3({0.0f, y, 0.0f}, Scale3(exitPortal.inward, push))));
            };

            auto addWallPatch = [&](float a, float b, float y0, float y1) {
                if (b <= a || y1 <= y0) return;
                AddQuadUV(vertices, indices,
                    wallPoint(a, y0), wallPoint(b, y0), wallPoint(b, y1), wallPoint(a, y1),
                    exitPortal.inward, exitPortal.right,
                    {a / settings_.wallTextureMeters, y0 / settings_.wallTextureMeters},
                    {b / settings_.wallTextureMeters, y0 / settings_.wallTextureMeters},
                    {b / settings_.wallTextureMeters, y1 / settings_.wallTextureMeters},
                    {a / settings_.wallTextureMeters, y1 / settings_.wallTextureMeters},
                    0.0f);
            };

            addWallPatch(-exitPortal.halfSpan, -openingHalf, 0.0f, wallH);
            addWallPatch(openingHalf, exitPortal.halfSpan, 0.0f, wallH);
            addWallPatch(-openingHalf, openingHalf, doorwayTop, wallH);

            XMFLOAT3 outward = Scale3(exitPortal.inward, -1.0f);
            XMFLOAT3 nearCenter = Add3(exitPortal.wallCenter, Scale3(outward, 0.05f));
            XMFLOAT3 farCenter = Add3(exitPortal.wallCenter, Scale3(outward, vestibuleLength));
            auto p = [&](float along, float y, float depth) {
                return Add3(nearCenter, Add3(Scale3(exitPortal.right, along), Add3(Scale3(up, y), Scale3(outward, depth))));
            };

            AddQuadUV(vertices, indices,
                p(-vestibuleHalf, 0.0f, 0.0f), p(vestibuleHalf, 0.0f, 0.0f), p(vestibuleHalf, 0.0f, vestibuleLength), p(-vestibuleHalf, 0.0f, vestibuleLength),
                {0, 1, 0}, exitPortal.right,
                FloorUv(nearCenter.x, nearCenter.z), FloorUv(Add3(nearCenter, Scale3(exitPortal.right, vestibuleHalf * 2.0f)).x, nearCenter.z),
                FloorUv(Add3(farCenter, Scale3(exitPortal.right, vestibuleHalf)).x, Add3(farCenter, Scale3(exitPortal.right, vestibuleHalf)).z),
                FloorUv(Add3(farCenter, Scale3(exitPortal.right, -vestibuleHalf)).x, Add3(farCenter, Scale3(exitPortal.right, -vestibuleHalf)).z),
                1.0f);
            XMFLOAT3 vc0 = p(-vestibuleHalf, wallH, vestibuleLength);
            XMFLOAT3 vc1 = p(vestibuleHalf, wallH, vestibuleLength);
            XMFLOAT3 vc2 = p(vestibuleHalf, wallH, 0.0f);
            XMFLOAT3 vc3 = p(-vestibuleHalf, wallH, 0.0f);
            AddQuadUV(vertices, indices,
                vc0, vc1, vc2, vc3,
                {0, -1, 0}, exitPortal.right,
                CeilingUv(vc0.x, vc0.z), CeilingUv(vc1.x, vc1.z),
                CeilingUv(vc2.x, vc2.z), CeilingUv(vc3.x, vc3.z),
                2.0f);
            auto addVestibuleSide = [&](float side) {
                XMFLOAT3 normal = Scale3(exitPortal.right, -side);
                AddQuadUV(vertices, indices,
                    p(side * vestibuleHalf, 0.0f, vestibuleLength), p(side * vestibuleHalf, 0.0f, 0.0f),
                    p(side * vestibuleHalf, wallH, 0.0f), p(side * vestibuleHalf, wallH, vestibuleLength),
                    normal, Scale3(outward, -1.0f),
                    {0, 0}, {vestibuleLength / settings_.wallTextureMeters, 0},
                    {vestibuleLength / settings_.wallTextureMeters, wallH / settings_.wallTextureMeters}, {0, wallH / settings_.wallTextureMeters},
                    0.0f);
            };
            addVestibuleSide(-1.0f);
            addVestibuleSide(1.0f);
            AddQuadUV(vertices, indices,
                p(vestibuleHalf, 0.0f, vestibuleLength), p(-vestibuleHalf, 0.0f, vestibuleLength),
                p(-vestibuleHalf, wallH, vestibuleLength), p(vestibuleHalf, wallH, vestibuleLength),
                Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
                {0, 0}, {1, 0}, {1, 1}, {0, 1}, 10.0f);
        };

        auto addNorthWallRunWithPortal = [&](int y, int x0, int x1) {
            if (exitPortal.valid && exitPortal.dy == -1 && exitPortal.tile.y == y && exitPortal.tile.x >= x0 && exitPortal.tile.x < x1) {
                if (x0 < exitPortal.tile.x) addNorthWallRun(y, x0, exitPortal.tile.x);
                addExitDoorwayWall();
                if (exitPortal.tile.x + 1 < x1) addNorthWallRun(y, exitPortal.tile.x + 1, x1);
            } else {
                addNorthWallRun(y, x0, x1);
            }
        };

        auto addSouthWallRunWithPortal = [&](int y, int x0, int x1) {
            if (exitPortal.valid && exitPortal.dy == 1 && exitPortal.tile.y == y && exitPortal.tile.x >= x0 && exitPortal.tile.x < x1) {
                if (x0 < exitPortal.tile.x) addSouthWallRun(y, x0, exitPortal.tile.x);
                addExitDoorwayWall();
                if (exitPortal.tile.x + 1 < x1) addSouthWallRun(y, exitPortal.tile.x + 1, x1);
            } else {
                addSouthWallRun(y, x0, x1);
            }
        };

        auto addWestWallRunWithPortal = [&](int x, int y0, int y1) {
            if (exitPortal.valid && exitPortal.dx == -1 && exitPortal.tile.x == x && exitPortal.tile.y >= y0 && exitPortal.tile.y < y1) {
                if (y0 < exitPortal.tile.y) addWestWallRun(x, y0, exitPortal.tile.y);
                addExitDoorwayWall();
                if (exitPortal.tile.y + 1 < y1) addWestWallRun(x, exitPortal.tile.y + 1, y1);
            } else {
                addWestWallRun(x, y0, y1);
            }
        };

        auto addEastWallRunWithPortal = [&](int x, int y0, int y1) {
            if (exitPortal.valid && exitPortal.dx == 1 && exitPortal.tile.x == x && exitPortal.tile.y >= y0 && exitPortal.tile.y < y1) {
                if (y0 < exitPortal.tile.y) addEastWallRun(x, y0, exitPortal.tile.y);
                addExitDoorwayWall();
                if (exitPortal.tile.y + 1 < y1) addEastWallRun(x, exitPortal.tile.y + 1, y1);
            } else {
                addEastWallRun(x, y0, y1);
            }
        };

        for (int y = 0; y < maze_.h; ++y) {
            int x = 0;
            while (x < maze_.w) {
                while (x < maze_.w && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x, y - 1))) ++x;
                int start = x;
                while (x < maze_.w && maze_.IsOpen(x, y) && !maze_.IsOpen(x, y - 1)) ++x;
                if (start < x) addNorthWallRunWithPortal(y, start, x);
            }

            x = 0;
            while (x < maze_.w) {
                while (x < maze_.w && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x, y + 1))) ++x;
                int start = x;
                while (x < maze_.w && maze_.IsOpen(x, y) && !maze_.IsOpen(x, y + 1)) ++x;
                if (start < x) addSouthWallRunWithPortal(y, start, x);
            }
        }

        for (int x = 0; x < maze_.w; ++x) {
            int y = 0;
            while (y < maze_.h) {
                while (y < maze_.h && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x - 1, y))) ++y;
                int start = y;
                while (y < maze_.h && maze_.IsOpen(x, y) && !maze_.IsOpen(x - 1, y)) ++y;
                if (start < y) addWestWallRunWithPortal(x, start, y);
            }

            y = 0;
            while (y < maze_.h) {
                while (y < maze_.h && !(maze_.IsOpen(x, y) && !maze_.IsOpen(x + 1, y))) ++y;
                int start = y;
                while (y < maze_.h && maze_.IsOpen(x, y) && !maze_.IsOpen(x + 1, y)) ++y;
                if (start < y) addEastWallRunWithPortal(x, start, y);
            }
        }

        auto tileHash = [&](int x, int y, float salt) {
            return LampHash(static_cast<float>(x) + salt * 3.17f, static_cast<float>(y) - salt * 5.31f);
        };

        struct FloorFootprint {
            float x = 0.0f;
            float z = 0.0f;
            float hx = 0.0f;
            float hz = 0.0f;
            float c = 1.0f;
            float s = 0.0f;
        };
        std::vector<FloorFootprint> floorReservations;
        floorReservations.reserve(512);

        auto makeFootprint = [&](float px, float pz, float width, float depth, float yaw, float pad) {
            FloorFootprint fp{};
            fp.x = px;
            fp.z = pz;
            fp.hx = width * 0.5f + pad;
            fp.hz = depth * 0.5f + pad;
            fp.c = std::cos(yaw);
            fp.s = std::sin(yaw);
            return fp;
        };
        auto axisDot = [](float ax, float az, float bx, float bz) {
            return ax * bx + az * bz;
        };
        auto footprintOverlap = [&](const FloorFootprint& a, const FloorFootprint& b) {
            auto separatedOn = [&](float ax, float az) {
                float dx = b.x - a.x;
                float dz = b.z - a.z;
                float center = std::abs(axisDot(dx, dz, ax, az));
                float ar = a.hx * std::abs(axisDot(a.c, -a.s, ax, az)) +
                    a.hz * std::abs(axisDot(a.s, a.c, ax, az));
                float br = b.hx * std::abs(axisDot(b.c, -b.s, ax, az)) +
                    b.hz * std::abs(axisDot(b.s, b.c, ax, az));
                return center > ar + br;
            };
            if (separatedOn(a.c, -a.s)) return false;
            if (separatedOn(a.s, a.c)) return false;
            if (separatedOn(b.c, -b.s)) return false;
            if (separatedOn(b.s, b.c)) return false;
            return true;
        };
        auto footprintFitsMaze = [&](float px, float pz, float width, float depth, float yaw, float wallPad) {
            float c = std::cos(yaw);
            float s = std::sin(yaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 forward{s, 0.0f, c};
            float hx = width * 0.5f + wallPad;
            float hz = depth * 0.5f + wallPad;
            int sxCount = std::clamp(static_cast<int>(std::ceil((width + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
            int szCount = std::clamp(static_cast<int>(std::ceil((depth + wallPad * 2.0f) / (tileMin * 0.18f))) + 2, 3, 13);
            for (int sy = 0; sy < szCount; ++sy) {
                float fy = szCount == 1 ? 0.0f : static_cast<float>(sy) / static_cast<float>(szCount - 1);
                float ly = Lerp(-hz, hz, fy);
                for (int sx = 0; sx < sxCount; ++sx) {
                    float fx = sxCount == 1 ? 0.0f : static_cast<float>(sx) / static_cast<float>(sxCount - 1);
                    float lx = Lerp(-hx, hx, fx);
                    XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, lx, 0.0f, ly));
                    Tile tile = maze_.TileFromWorld(p.x, p.z);
                    if (!maze_.IsOpen(tile.x, tile.y)) return false;
                }
            }
            return true;
        };
        auto floorFootprintClear = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.055f) {
            if (!footprintFitsMaze(px, pz, width, depth, yaw, pad)) return false;
            FloorFootprint candidate = makeFootprint(px, pz, width, depth, yaw, pad);
            for (const FloorFootprint& reserved : floorReservations) {
                if (footprintOverlap(candidate, reserved)) return false;
            }
            return true;
        };
        auto reserveFloorFootprint = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.075f) {
            if (!floorFootprintClear(px, pz, width, depth, yaw, pad)) return false;
            floorReservations.push_back(makeFootprint(px, pz, width, depth, yaw, pad));
            return true;
        };
        auto longFloorFootprintClear = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.055f) {
            if (!floorFootprintClear(px, pz, width, depth, yaw, pad)) return false;
            float c = std::cos(yaw);
            float s = std::sin(yaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 forward{s, 0.0f, c};
            int steps = std::clamp(static_cast<int>(width / (tileMin * 0.30f)) + 2, 4, 22);
            for (int i = 0; i <= steps; ++i) {
                float along = (static_cast<float>(i) / static_cast<float>(steps) - 0.5f) * width;
                const float laterals[] = {-0.46f, 0.0f, 0.46f};
                for (float lateral : laterals) {
                    XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0, 1, 0}, forward, along, 0.0f, lateral * depth));
                    Tile tile = maze_.TileFromWorld(p.x, p.z);
                    if (!maze_.IsOpen(tile.x, tile.y)) return false;
                }
            }
            return true;
        };
        auto constrainedHallwayTile = [&](Tile t) {
            if (!maze_.IsOpen(t.x, t.y)) return false;
            return maze_.OpenNeighborCount(t) <= 2 && !IsRoomLike(t);
        };
        auto reserveRealisticFloorFootprint = [&](float& px, float& pz, float width, float depth,
                                                  float& yaw, float pad, float seed) {
            Tile t = maze_.TileFromWorld(px, pz);
            if (!constrainedHallwayTile(t)) {
                return reserveFloorFootprint(px, pz, width, depth, yaw, pad);
            }

            struct CandidatePlacement {
                float x = 0.0f;
                float z = 0.0f;
                float yaw = 0.0f;
                float score = -1.0e9f;
            };
            CandidatePlacement best{};
            XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
            const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            int openCount = 0;
            for (Tile d : dirs) {
                if (!maze_.IsOpen(t.x + d.x, t.y + d.y)) continue;
                ++openCount;
                float dirYaw = std::atan2(static_cast<float>(d.x), static_cast<float>(d.y));
                float jitter = (LampHash(center.x + seed * 11.7f + d.x * 3.1f, center.z - seed * 9.3f + d.y * 4.7f) - 0.5f) * 0.22f;
                const float shifts[] = {0.20f, 0.08f, -0.02f};
                const float flips[] = {0.0f, kPi};
                for (float flip : flips) {
                    for (float shift : shifts) {
                        float cx = center.x + static_cast<float>(d.x) * tileW * shift;
                        float cz = center.z + static_cast<float>(d.y) * tileD * shift;
                        float candidateYaw = dirYaw + flip + jitter;
                        if (!floorFootprintClear(cx, cz, width, depth, candidateYaw, pad)) continue;
                        float sideClear = static_cast<float>(maze_.LocalOpenCount(maze_.TileFromWorld(cx, cz), 1));
                        float shiftScore = shift * 3.2f;
                        float longAxisScore = depth >= width ? 1.0f : 0.35f;
                        float score = sideClear * 0.22f + shiftScore + longAxisScore - std::abs(jitter) * 0.15f;
                        if (score > best.score) {
                            best = {cx, cz, candidateYaw, score};
                        }
                    }
                }
            }

            if (openCount <= 0 || best.score <= -1.0e8f) return false;
            px = best.x;
            pz = best.z;
            yaw = best.yaw;
            return reserveFloorFootprint(px, pz, width, depth, yaw, pad);
        };

        auto adjustLongHallwayPlacement = [&](float& px, float& pz, float& yaw, float width, float depth, float seed) {
            Tile t = maze_.TileFromWorld(px, pz);
            if (!constrainedHallwayTile(t)) return true;
            XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
            const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
            float bestScore = -1.0e9f;
            float bestX = px;
            float bestZ = pz;
            float bestYaw = yaw;
            for (Tile d : dirs) {
                if (!maze_.IsOpen(t.x + d.x, t.y + d.y)) continue;
                float dirYaw = std::atan2(static_cast<float>(d.x), static_cast<float>(d.y));
                float jitter = (LampHash(center.x + seed * 4.9f + d.x, center.z - seed * 6.3f + d.y) - 0.5f) * 0.18f;
                const float shifts[] = {0.18f, 0.06f};
                const float flips[] = {0.0f, kPi};
                for (float shift : shifts) {
                    for (float flip : flips) {
                        float cx = center.x + static_cast<float>(d.x) * tileW * shift;
                        float cz = center.z + static_cast<float>(d.y) * tileD * shift;
                        float candidateYaw = dirYaw + flip + jitter;
                        if (!longFloorFootprintClear(cx, cz, width, depth, candidateYaw, 0.075f)) continue;
                        float score = shift * 3.0f + static_cast<float>(maze_.LocalOpenCount(maze_.TileFromWorld(cx, cz), 1)) * 0.20f;
                        if (score > bestScore) {
                            bestScore = score;
                            bestX = cx;
                            bestZ = cz;
                            bestYaw = candidateYaw;
                        }
                    }
                }
            }
            if (bestScore <= -1.0e8f) return false;
            px = bestX;
            pz = bestZ;
            yaw = bestYaw;
            return true;
        };

        auto propSpan = [](const StaticPropMesh& mesh, int axis) {
            if (axis == 0) return std::max(0.001f, mesh.max.x - mesh.min.x);
            if (axis == 1) return std::max(0.001f, mesh.max.y - mesh.min.y);
            return std::max(0.001f, mesh.max.z - mesh.min.z);
        };
        auto cabinetSize = [](bool tall) {
            return tall
                ? XMFLOAT3{0.66f, 1.34f, 0.40f}
                : XMFLOAT3{0.78f, 0.92f, 0.42f};
        };
        auto pickChairMesh = [&](float seed, bool waitingChair) -> const StaticPropMesh* {
            int order[3] = {0, 1, 2};
            if (waitingChair) {
                order[0] = 1;
                order[1] = 0;
                order[2] = 2;
            } else {
                order[0] = 2;
                order[1] = 0;
                order[2] = 1;
            }
            std::array<const StaticPropMesh*, 3> candidates{};
            int count = 0;
            for (int idx : order) {
                if (!chairPropMeshes_[static_cast<size_t>(idx)].vertices.empty()) {
                    candidates[static_cast<size_t>(count++)] = &chairPropMeshes_[static_cast<size_t>(idx)];
                }
            }
            if (count <= 0) return nullptr;
            int pick = std::clamp(static_cast<int>(seed * static_cast<float>(count)), 0, count - 1);
            return candidates[static_cast<size_t>(pick)];
        };

        auto nearestLampXZ = [&](float px, float pz) {
            float strideTiles = std::max(1.0f, std::floor(settings_.lampSpacing / std::max(0.001f, tileAvg) + 0.5f));
            float strideX = tileW * strideTiles;
            float strideZ = tileD * strideTiles;
            float originX = ox + tileW * 1.5f;
            float originZ = oz + tileD * 1.5f;
            float cellX = std::floor((px - originX) / std::max(0.001f, strideX) + 0.5f);
            float cellZ = std::floor((pz - originZ) / std::max(0.001f, strideZ) + 0.5f);
            return XMFLOAT2{originX + cellX * strideX, originZ + cellZ * strideZ};
        };

        auto addBakedPropShadow = [&](float px, float pz, float width, float depth, float height, float yaw, float seed) {
            if (gEffectDebugViewer && gDebugSliceEffect != DebugSliceEffect::CeilingLamps) return;
            if (width <= 0.03f || depth <= 0.03f || height <= 0.04f) return;
            XMFLOAT2 lamp = nearestLampXZ(px, pz);
            float dx = px - lamp.x;
            float dz = pz - lamp.y;
            float distXZ = std::sqrt(dx * dx + dz * dz);
            float distFade = std::clamp(distXZ / std::max(0.001f, tileAvg * 3.8f), 0.0f, 1.0f);
            float dirLen = std::max(0.001f, distXZ);
            float dirX = dx / dirLen;
            float dirZ = dz / dirLen;
            float verticalGap = std::max(0.35f, wallH - std::min(height, wallH * 0.86f));
            float offset = std::min(tileAvg * 0.38f, distXZ * height / verticalGap * 0.24f);
            float shadowX = px + dirX * offset;
            float shadowZ = pz + dirZ * offset;
            float broad = std::max(width, depth);
            float cross = broad * (0.76f + distFade * 0.36f) + height * (0.05f + distFade * 0.12f);
            float along = std::max(depth, broad * 0.72f) * (0.94f + distFade * 0.72f) + height * (0.14f + distFade * 0.52f);
            float shadowYaw = distXZ > 0.08f ? std::atan2(dirX, dirZ) : yaw;
            float softness = std::clamp(0.10f + distFade * 0.82f + seed * 0.035f, 0.06f, 0.95f);
            float lift = 0.012f + seed * 0.003f;
            AddFloorCard(vertices, transparentIndices, {shadowX, 0.0f, shadowZ},
                std::max(0.10f, cross), std::max(0.12f, along), shadowYaw, lift, 24.0f + softness);
        };

        auto addChair = [&](XMFLOAT3 c, float yaw, bool waitingChair) {
            const StaticPropMesh* chairMesh = pickChairMesh(LampHash(c.x + yaw * 0.37f, c.z + (waitingChair ? 9.1f : 17.4f)), waitingChair);
            if (!chairMesh) return false;
            float px = c.x;
            float pz = c.z;
            float placeYaw = yaw;
            if (!reserveRealisticFloorFootprint(px, pz, waitingChair ? 1.02f : 1.05f, waitingChair ? 0.96f : 1.02f,
                    placeYaw, 0.075f, LampHash(c.x + 2.3f, c.z - 1.7f))) return false;
            float scale = waitingChair ? 1.04f : 1.10f;
            AppendStaticPropMeshGrounded(vertices, indices, *chairMesh, {px, 0.0f, pz}, placeYaw, scale, scale, scale,
                0.0f, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, waitingChair ? 0.88f : 0.94f, waitingChair ? 0.82f : 0.90f,
                waitingChair ? 0.96f : 1.05f, placeYaw, LampHash(px + 5.1f, pz - 2.8f));
            propLookPoints_.push_back({px, 0.72f, pz});
            return true;
            float seatY = waitingChair ? 0.43f : 0.48f;
            float bodyMat = 8.0f;
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, {c.x, seatY - 0.033f, c.z}, {0.42f, 0.018f, 0.37f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, {c.x, seatY + 0.010f, c.z}, {0.36f, 0.048f, 0.31f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, seatY + 0.015f, 0.33f), {0.32f, 0.014f, 0.030f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, seatY + 0.015f, -0.33f), {0.32f, 0.014f, 0.030f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.37f, seatY + 0.015f, 0.0f), {0.020f, 0.014f, 0.29f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.37f, seatY + 0.015f, 0.0f), {0.020f, 0.014f, 0.29f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.83f, -0.315f),
                {0.39f, 0.30f, 0.036f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, 0.84f, -0.356f), {0.42f, 0.015f, 0.018f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 1.125f, -0.347f), {0.37f, 0.018f, 0.020f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.31f, 0.71f, -0.30f), {0.026f, 0.34f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.31f, 0.71f, -0.30f), {0.026f, 0.34f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.12f, 0.88f, -0.347f), {0.015f, 0.24f, 0.014f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.12f, 0.88f, -0.347f), {0.015f, 0.24f, 0.014f}, yaw, 10.0f);
            const float legX[2] = {-0.25f, 0.25f};
            const float legZ[2] = {-0.22f, 0.22f};
            for (float lx : legX) {
                for (float lz : legZ) {
                    XMFLOAT3 leg = off(lx, 0.21f, lz);
                    AddOrientedBox(vertices, indices, leg, {0.025f, 0.21f, 0.025f}, yaw, 10.0f);
                }
            }
            AddOrientedBox(vertices, indices, off(0.0f, 0.25f, 0.22f), {0.25f, 0.014f, 0.014f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.25f, -0.22f), {0.25f, 0.014f, 0.014f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.25f, 0.25f, 0.0f), {0.014f, 0.014f, 0.22f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.25f, 0.25f, 0.0f), {0.014f, 0.014f, 0.22f}, yaw, 10.0f);
            if (waitingChair) {
                AddOrientedBox(vertices, indices, off(-0.43f, 0.63f, 0.02f), {0.035f, 0.035f, 0.30f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.43f, 0.63f, 0.02f), {0.035f, 0.035f, 0.30f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(-0.43f, 0.48f, -0.18f), {0.020f, 0.18f, 0.020f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.43f, 0.48f, -0.18f), {0.020f, 0.18f, 0.020f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.0f, 0.55f, 0.30f), {0.41f, 0.016f, 0.018f}, yaw, 10.0f);
            }
            if (!waitingChair) {
                AddOrientedBox(vertices, indices, {c.x, 0.24f, c.z}, {0.055f, 0.24f, 0.055f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, {c.x, 0.50f, c.z}, {0.075f, 0.040f, 0.075f}, yaw, 10.0f);
                for (int k = 0; k < 4; ++k) {
                    float armYaw = yaw + k * kPi * 0.5f;
                    XMFLOAT3 foot = Add3({c.x, 0.08f, c.z}, {std::sin(armYaw) * 0.26f, 0.0f, std::cos(armYaw) * 0.26f});
                    AddOrientedBox(vertices, indices, foot, {0.16f, 0.025f, 0.025f}, armYaw, 10.0f);
                    XMFLOAT3 caster = Add3(foot, {std::sin(armYaw) * 0.13f, -0.035f, std::cos(armYaw) * 0.13f});
                    AddOrientedBox(vertices, indices, caster, {0.042f, 0.022f, 0.018f}, armYaw, 10.0f);
                    AddOrientedBox(vertices, indices, Add3(caster, {0.0f, 0.0f, 0.018f}), {0.030f, 0.018f, 0.010f}, armYaw, 5.0f);
                }
                AddOrientedBox(vertices, indices, off(-0.40f, 0.64f, -0.02f), {0.035f, 0.035f, 0.28f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.40f, 0.64f, -0.02f), {0.035f, 0.035f, 0.28f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(-0.40f, 0.74f, -0.16f), {0.026f, 0.026f, 0.13f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(0.40f, 0.74f, -0.16f), {0.026f, 0.026f, 0.13f}, yaw, 10.0f);
            }
            propLookPoints_.push_back({c.x, 0.72f, c.z});
            return true;
        };

        auto addStandingTable = [&](float px, float pz, float width, float depth, float yaw, float seed) {
            if (deskPropMesh_.vertices.empty()) return false;
            if (!reserveFloorFootprint(px, pz, width + 0.22f, depth + 0.22f, yaw, 0.085f)) return false;
            float topY = 0.70f + seed * 0.05f;
            float scaleX = depth / propSpan(deskPropMesh_, 0);
            float scaleY = topY + 0.035f;
            float scaleZ = width / propSpan(deskPropMesh_, 2);
            AppendStaticPropMesh(vertices, indices, deskPropMesh_, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
                0.0f, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, width, depth, topY + 0.08f, yaw, seed);
            propLookPoints_.push_back({px, topY, pz});
            return true;
            XMFLOAT3 c{px, 0.0f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            float topMat = seed < 0.42f ? 8.0f : 10.0f;
            AddOrientedBox(vertices, indices, off(0.0f, topY, 0.0f), {width * 0.5f, 0.042f, depth * 0.5f}, yaw, topMat);
            AddOrientedBox(vertices, indices, off(0.0f, topY - 0.055f, depth * 0.5f - 0.035f), {width * 0.48f, 0.030f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, topY - 0.055f, -depth * 0.5f + 0.035f), {width * 0.48f, 0.030f, 0.026f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(width * 0.5f - 0.035f, topY - 0.055f, 0.0f), {0.026f, 0.030f, depth * 0.44f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-width * 0.5f + 0.035f, topY - 0.055f, 0.0f), {0.026f, 0.030f, depth * 0.44f}, yaw, 10.0f);
            const float lx[2] = {-0.42f, 0.42f};
            const float lz[2] = {-0.42f, 0.42f};
            for (float sx : lx) {
                for (float sz : lz) {
                    AddOrientedBox(vertices, indices, off(sx * width, topY * 0.50f, sz * depth), {0.035f, topY * 0.46f, 0.035f}, yaw, 10.0f);
                }
            }
            AddOrientedBox(vertices, indices, off(0.0f, 0.36f, depth * 0.38f), {width * 0.37f, 0.018f, 0.018f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.36f, -depth * 0.38f), {width * 0.37f, 0.018f, 0.018f}, yaw, 10.0f);
            if (seed > 0.52f) {
                AddOrientedBox(vertices, indices, off((seed - 0.5f) * width * 0.45f, topY + 0.060f, (seed - 0.5f) * depth * -0.25f),
                    {0.20f, 0.018f, 0.145f}, yaw + 0.18f, 9.0f);
                AddOrientedBox(vertices, indices, off(width * 0.20f, topY + 0.075f, depth * -0.18f),
                    {0.045f, 0.055f, 0.045f}, yaw, 5.0f);
            }
            propLookPoints_.push_back({px, topY, pz});
            return true;
        };

        auto addSideTable = [&](float px, float pz, float width, float depth, float yaw, float seed) {
            if (deskPropMesh_.vertices.empty()) return false;
            if (!reserveFloorFootprint(px, pz, width + 0.24f, depth * 0.72f + 0.36f, yaw, 0.085f)) return false;
            float height = 0.68f + seed * 0.10f;
            float scaleX = depth / propSpan(deskPropMesh_, 0);
            float scaleY = height + 0.030f;
            float scaleZ = width / propSpan(deskPropMesh_, 2);
            AppendStaticPropMesh(vertices, indices, deskPropMesh_, {px, 0.0f, pz}, yaw + kPi * 0.5f, scaleX, scaleY, scaleZ,
                0.0f, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, width, depth, height + 0.06f, yaw, seed);
            propLookPoints_.push_back({px, height * 0.72f, pz});
            return true;
            XMFLOAT3 c{px, 0.0f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            float bodyMat = seed < 0.46f ? 8.0f : 10.0f;
            float trimMat = 10.0f;
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.50f, 0.0f), {width * 0.50f, height * 0.50f, depth * 0.50f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, height + 0.030f, 0.0f), {width * 0.54f, 0.035f, depth * 0.54f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, 0.045f, 0.0f), {width * 0.53f, 0.040f, depth * 0.52f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(-width * 0.52f, height * 0.52f, 0.0f), {0.026f, height * 0.46f, depth * 0.50f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(width * 0.52f, height * 0.52f, 0.0f), {0.026f, height * 0.46f, depth * 0.50f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.52f, -depth * 0.535f), {width * 0.47f, height * 0.39f, 0.014f}, yaw, 5.0f);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.52f, -depth * 0.566f), {0.010f, height * 0.35f, 0.016f}, yaw, trimMat);
            for (int door = -1; door <= 1; door += 2) {
                float dx = door * width * 0.245f;
                AddOrientedBox(vertices, indices, off(dx, height * 0.52f, -depth * 0.585f), {width * 0.215f, height * 0.34f, 0.012f}, yaw, bodyMat);
                AddOrientedBox(vertices, indices, off(dx - door * width * 0.060f, height * 0.52f, -depth * 0.612f), {0.012f, height * 0.115f, 0.014f}, yaw, trimMat);
                if (seed > 0.62f && door == (seed > 0.80f ? 1 : -1)) {
                    float doorYaw = yaw + door * (0.42f + seed * 0.22f);
                    XMFLOAT3 openCenter = off(dx + door * width * 0.075f, height * 0.50f, -depth * 0.66f);
                    AddOrientedBox(vertices, indices, openCenter, {width * 0.19f, height * 0.31f, 0.012f}, doorYaw, bodyMat);
                }
            }
            for (int legX = -1; legX <= 1; legX += 2) {
                for (int legZ = -1; legZ <= 1; legZ += 2) {
                    AddOrientedBox(vertices, indices, off(legX * width * 0.39f, 0.085f, legZ * depth * 0.36f), {0.026f, 0.085f, 0.026f}, yaw, trimMat);
                }
            }
            if (seed > 0.35f) {
                AddOrientedBox(vertices, indices, off(-width * 0.18f, height + 0.080f, -depth * 0.16f), {width * 0.22f, 0.018f, 0.125f}, yaw + 0.25f, 9.0f);
                AddOrientedBox(vertices, indices, off(width * 0.22f, height + 0.090f, depth * 0.10f), {0.045f, 0.055f, 0.045f}, yaw, 5.0f);
            }
            propLookPoints_.push_back({px, height * 0.72f, pz});
            return true;
        };

        auto addTippedChair = [&](float px, float pz, float yaw, bool waitingChair, float seed) {
            const StaticPropMesh* chairMesh = pickChairMesh(seed, waitingChair);
            if (!chairMesh) return false;
            if (!reserveRealisticFloorFootprint(px, pz, waitingChair ? 1.10f : 1.18f, waitingChair ? 1.26f : 1.34f,
                    yaw, 0.075f, seed)) return false;
            float scale = waitingChair ? 1.02f : 1.08f;
            float pitch = seed < 0.5f ? kPi * 0.5f : -kPi * 0.5f;
            AppendStaticPropMeshGrounded(vertices, indices, *chairMesh, {px, 0.0f, pz}, yaw, scale, scale, scale, pitch,
                -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, waitingChair ? 1.10f : 1.18f, waitingChair ? 1.26f : 1.34f,
                0.46f, yaw, seed);
            propLookPoints_.push_back({px, 0.22f, pz});
            return true;
            XMFLOAT3 c{px, 0.0f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, off(0.0f, 0.12f, 0.02f), {0.36f, 0.030f, 0.31f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, off(0.0f, 0.16f, -0.44f), {0.38f, 0.030f, 0.25f}, yaw + 0.08f, 8.0f);
            AddOrientedBox(vertices, indices, off(-0.31f, 0.16f, -0.23f), {0.028f, 0.028f, 0.34f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.31f, 0.16f, -0.23f), {0.028f, 0.028f, 0.34f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(-0.22f, 0.11f, 0.28f), {0.025f, 0.11f, 0.025f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off(0.24f, 0.10f, 0.25f), {0.025f, 0.10f, 0.025f}, yaw, 10.0f);
            AddOrientedBox(vertices, indices, off((seed - 0.5f) * 0.36f, 0.055f, 0.0f), {0.18f, 0.020f, 0.020f}, yaw + 0.45f, 10.0f);
            if (!waitingChair) {
                AddOrientedBox(vertices, indices, off(0.02f, 0.15f, 0.02f), {0.050f, 0.15f, 0.050f}, yaw, 10.0f);
                AddOrientedBox(vertices, indices, off(-0.38f, 0.065f, 0.03f), {0.12f, 0.020f, 0.018f}, yaw + 0.20f, 10.0f);
                AddOrientedBox(vertices, indices, off(0.38f, 0.065f, -0.05f), {0.12f, 0.020f, 0.018f}, yaw - 0.18f, 10.0f);
            }
            propLookPoints_.push_back({px, 0.24f, pz});
            return true;
        };

        auto addTrashBin = [&](float px, float pz, float yaw, bool tipped, float seed) {
            if (trashBinPropMesh_.vertices.empty()) return false;
            seed = std::clamp(seed, 0.0f, 1.0f);
            float targetHeight = 0.40f + seed * 0.14f;
            float scale = targetHeight / propSpan(trashBinPropMesh_, 1);
            float diameter = std::max(propSpan(trashBinPropMesh_, 0), propSpan(trashBinPropMesh_, 2)) * scale;
            float footprintW = diameter + 0.08f;
            float footprintD = tipped ? targetHeight + 0.16f : footprintW;
            if (!reserveRealisticFloorFootprint(px, pz, footprintW, footprintD, yaw, tipped ? 0.065f : 0.055f, seed)) return false;
            float pitch = tipped ? (seed < 0.5f ? kPi * 0.5f : -kPi * 0.5f) : 0.0f;
            AppendStaticPropMeshGrounded(vertices, indices, trashBinPropMesh_, {px, 0.0f, pz}, yaw,
                scale, scale, scale, pitch, -1.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, footprintW, footprintD, tipped ? diameter * 0.62f : targetHeight, yaw, seed);
            propLookPoints_.push_back({px, tipped ? diameter * 0.45f : targetHeight * 0.55f, pz});
            return true;
        };

        auto addDeskLampOnSurface = [&](float px, float pz, float surfaceY, float tableYaw,
                                         float tableWidth, float tableDepth, float seed) {
            if (deskLampPropMesh_.vertices.empty()) return false;
            float c = std::cos(tableYaw);
            float s = std::sin(tableYaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 forward{s, 0.0f, c};
            float ox = (seed - 0.5f) * tableWidth * 0.34f;
            float oz = (LampHash(px + seed * 7.1f, pz - seed * 5.3f) - 0.5f) * tableDepth * 0.30f;
            XMFLOAT3 p = Add3({px, 0.0f, pz}, Add3(Scale3(right, ox), Scale3(forward, oz)));
            float targetHeight = 0.38f + LampHash(px - seed * 3.1f, pz + seed * 4.7f) * 0.09f;
            float scale = targetHeight / propSpan(deskLampPropMesh_, 1);
            AppendStaticPropMeshGrounded(vertices, indices, deskLampPropMesh_, {p.x, surfaceY + 0.012f, p.z},
                tableYaw + (seed - 0.5f) * 0.85f, scale, scale, scale, 0.0f, -1.0f, &propShadowIndices);
            propLookPoints_.push_back({p.x, surfaceY + targetHeight * 0.62f, p.z});
            return true;
        };

        auto addCassetteAt = [&](float px, float pz, float yaw, float floorY, float seed) {
            if (cassettePropMesh_.vertices.empty()) return false;
            float width = 0.100f;
            float depth = 0.064f;
            if (!floorFootprintClear(px, pz, width, depth, yaw, 0.024f)) return false;
            float scale = width / propSpan(cassettePropMesh_, 0);
            AppendStaticPropMeshGrounded(vertices, indices, cassettePropMesh_, {px, floorY + 0.002f, pz},
                yaw + (seed - 0.5f) * 0.38f, scale, scale, scale, 0.0f, -1.0f, &propShadowIndices);
            return true;
        };

        auto addDebugPropInspectionModel = [&]() {
            if (!gEffectDebugViewer || gDebugSliceEffect != DebugSliceEffect::Props) return;
            int propIndex = WrapDebugPropIndex(gDebugPropIndex);
            const StaticPropMesh* mesh = nullptr;
            switch (propIndex) {
            case 0: mesh = &chairPropMeshes_[0]; break;
            case 1: mesh = &chairPropMeshes_[1]; break;
            case 2:
            case 3: mesh = &chairPropMeshes_[2]; break;
            case 4: mesh = &cabinetPropMesh_; break;
            case 5: mesh = &deskPropMesh_; break;
            case 6:
            case 7: mesh = &trashBinPropMesh_; break;
            case 8: mesh = &deskLampPropMesh_; break;
            case 9: mesh = &cassettePropMesh_; break;
            case 10: mesh = &airVentPropMesh_; break;
            case 11: mesh = &exitSignPropMesh_; break;
            case 12: mesh = &ceilingLampPropMeshes_[0]; break;
            case 13: mesh = &ceilingLampPropMeshes_[1]; break;
            case 14: mesh = &ceilingLampPropMeshes_[2]; break;
            case 15: mesh = &ceilingLampPropMeshes_[3]; break;
            default: break;
            }

            int tiles = std::clamp(gDebugSliceTiles, 1, 5);
            float centerX = ox + (1.0f + static_cast<float>(tiles) * 0.5f) * tileW;
            float centerZ = oz + (1.0f + static_cast<float>(tiles) * 0.5f) * tileD;
            float targetMax = 1.22f;
            float yaw = kPi;
            float pitch = 0.0f;
            switch (propIndex) {
            case 3:
                pitch = kPi * 0.5f;
                targetMax = 1.12f;
                break;
            case 6:
                targetMax = 0.58f;
                break;
            case 7:
                pitch = kPi * 0.5f;
                targetMax = 0.62f;
                break;
            case 4:
                targetMax = 1.44f;
                break;
            case 5:
                targetMax = 1.62f;
                yaw = kPi * 0.5f;
                break;
            case 8:
                targetMax = 0.56f;
                break;
            case 9:
                targetMax = 0.58f;
                break;
            case 10:
                targetMax = 0.86f;
                break;
            case 11:
                targetMax = 1.18f;
                break;
            case 12:
            case 13:
            case 14:
            case 15:
                targetMax = 1.36f;
                yaw = kPi * 0.5f;
                break;
            default:
                break;
            }

            if (mesh && !mesh->vertices.empty()) {
                float spanX = propSpan(*mesh, 0);
                float spanY = propSpan(*mesh, 1);
                float spanZ = propSpan(*mesh, 2);
                float spanMax = std::max(spanX, std::max(spanY, spanZ));
                float scale = std::clamp(targetMax / std::max(0.001f, spanMax), 0.035f, 12.0f);
                bool wallMounted = propIndex == 10 || propIndex == 11;
                bool suspendedLamp = propIndex >= 12 && propIndex <= 15;
                if (wallMounted || suspendedLamp) {
                    float c = std::cos(yaw);
                    float s = std::sin(yaw);
                    XMFLOAT3 right{c, 0.0f, -s};
                    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                    XMFLOAT3 forward{s, 0.0f, c};
                    XMFLOAT3 meshCenter{
                        (mesh->min.x + mesh->max.x) * 0.5f,
                        (mesh->min.y + mesh->max.y) * 0.5f,
                        (mesh->min.z + mesh->max.z) * 0.5f
                    };
                    XMFLOAT3 desiredCenter{
                        centerX,
                        wallMounted ? settings_.wallHeightMeters * 0.54f : 1.08f,
                        wallMounted ? centerZ - tileD * 0.34f : centerZ
                    };
                    XMFLOAT3 centeredOffset = Add3(Scale3(right, meshCenter.x * scale),
                        Add3(Scale3(up, meshCenter.y * scale), Scale3(forward, meshCenter.z * scale)));
                    XMFLOAT3 origin = Add3(desiredCenter, Scale3(centeredOffset, -1.0f));
                    AppendStaticPropMesh(vertices, indices, *mesh, origin, yaw, scale, scale, scale,
                        0.0f, -1.0f, &propShadowIndices);
                    propLookPoints_.push_back(desiredCenter);
                    return;
                }
                AppendStaticPropMeshGrounded(vertices, indices, *mesh, {centerX, 0.0f, centerZ},
                    yaw, scale, scale, scale, pitch, -1.0f, &propShadowIndices);
                float lookY = std::clamp((mesh->max.y - mesh->min.y) * scale * 0.55f, 0.16f, 1.15f);
                propLookPoints_.push_back({centerX, lookY, centerZ});
                return;
            }

            if (propIndex == 10) {
                float panelY = settings_.wallHeightMeters * 0.54f;
                float panelZ = centerZ - tileD * 0.34f;
                AddOrientedBox(vertices, indices, {centerX, panelY, panelZ}, {0.52f, 0.18f, 0.018f}, kPi, 10.0f);
                AddOrientedBox(vertices, indices, {centerX, panelY, panelZ - 0.026f}, {0.42f, 0.11f, 0.010f}, kPi, 5.0f);
                for (int slot = -3; slot <= 3; ++slot) {
                    AddOrientedBox(vertices, indices,
                        {centerX, panelY + static_cast<float>(slot) * 0.030f, panelZ - 0.044f},
                        {0.36f, 0.0048f, 0.006f}, kPi, 8.0f);
                }
                propLookPoints_.push_back({centerX, panelY, panelZ});
                return;
            }

            AddOrientedBox(vertices, indices, {centerX, 0.18f, centerZ}, {0.42f, 0.18f, 0.42f}, 0.0f, 5.0f);
            propLookPoints_.push_back({centerX, 0.18f, centerZ});
        };

        addDebugPropInspectionModel();

        auto addRoomClutterGroup = [&](Tile t, int groupIndex, uint32_t scatterSeed) {
            if (!IsRoomLike(t)) return false;
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float yaw = Rand01(groupIndex, 311, scatterSeed) * kPi * 2.0f;
            float kind = Rand01(groupIndex, 313, scatterSeed);
            float px = c.x + (Rand01(groupIndex, 317, scatterSeed) - 0.5f) * tileW * 0.44f;
            float pz = c.z + (Rand01(groupIndex, 331, scatterSeed) - 0.5f) * tileD * 0.44f;
            if (kind < 0.38f) {
                float w = 1.10f + Rand01(groupIndex, 337, scatterSeed) * 0.42f;
                float d = 0.68f + Rand01(groupIndex, 347, scatterSeed) * 0.28f;
                float chairRing = std::max(w, d) * 0.5f + 0.74f;
                if (!longFloorFootprintClear(px, pz, chairRing * 2.0f + 1.08f, chairRing * 2.0f + 1.02f, yaw, 0.070f)) return false;
                float tableSeed = Rand01(groupIndex, 349, scatterSeed);
                if (!addStandingTable(px, pz, w, d, yaw, tableSeed)) return false;
                if (Rand01(groupIndex, 351, scatterSeed) < 0.64f) {
                    addDeskLampOnSurface(px, pz, 0.745f + tableSeed * 0.05f, yaw, w, d, Rand01(groupIndex, 352, scatterSeed));
                }
                int chairs = 2 + static_cast<int>(Rand01(groupIndex, 353, scatterSeed) * 3.0f);
                int placedChairs = 0;
                for (int i = 0; i < chairs; ++i) {
                    float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(chairs) + RandRange(-0.22f, 0.22f);
                    float radius = chairRing + Rand01(groupIndex * 7 + i, 359, scatterSeed) * 0.18f;
                    XMFLOAT3 chairPos{px + std::sin(a) * radius, 0.0f, pz + std::cos(a) * radius};
                    float chairYaw = a + kPi + RandRange(-0.32f, 0.32f);
                    bool placed = false;
                    if (Rand01(groupIndex * 7 + i, 367, scatterSeed) < 0.22f) {
                        placed = addTippedChair(chairPos.x, chairPos.z, chairYaw, false, Rand01(groupIndex * 7 + i, 371, scatterSeed));
                    } else {
                        placed = addChair(chairPos, chairYaw, Rand01(groupIndex * 7 + i, 373, scatterSeed) < 0.42f);
                    }
                    if (placed) ++placedChairs;
                }
                if (Rand01(groupIndex, 375, scatterSeed) < 0.72f) {
                    float binA = yaw + Rand01(groupIndex, 377, scatterSeed) * kPi * 2.0f;
                    float binR = chairRing * (0.42f + Rand01(groupIndex, 381, scatterSeed) * 0.28f);
                    addTrashBin(px + std::sin(binA) * binR, pz + std::cos(binA) * binR,
                        binA + kPi * 0.5f, Rand01(groupIndex, 385, scatterSeed) < 0.38f,
                        Rand01(groupIndex, 387, scatterSeed));
                }
                return true;
            }
            if (kind < 0.62f) {
                float sideW = 1.12f + Rand01(groupIndex, 379, scatterSeed) * 0.44f;
                float sideD = 0.66f + Rand01(groupIndex, 383, scatterSeed) * 0.30f;
                float chairRadius = std::max(sideW, sideD) * 0.5f + 0.76f;
                if (!longFloorFootprintClear(px, pz, chairRadius * 2.0f + 0.80f, chairRadius * 1.55f, yaw, 0.070f)) return false;
                float sideSeed = Rand01(groupIndex, 389, scatterSeed);
                if (!addSideTable(px, pz, sideW, sideD, yaw, sideSeed)) return false;
                if (Rand01(groupIndex, 391, scatterSeed) < 0.58f) {
                    addDeskLampOnSurface(px, pz, 0.720f + sideSeed * 0.10f, yaw, sideW, sideD, Rand01(groupIndex, 393, scatterSeed));
                }
                addTippedChair(px + std::sin(yaw + 0.8f) * chairRadius, pz + std::cos(yaw + 0.8f) * chairRadius,
                    yaw + kPi * 0.72f, Rand01(groupIndex, 397, scatterSeed) < 0.5f, Rand01(groupIndex, 401, scatterSeed));
                float binA = yaw - 0.95f + (Rand01(groupIndex, 403, scatterSeed) - 0.5f) * 0.58f;
                addTrashBin(px + std::sin(binA) * chairRadius * 0.72f, pz + std::cos(binA) * chairRadius * 0.72f,
                    binA + kPi * 0.5f, Rand01(groupIndex, 405, scatterSeed) < 0.52f,
                    Rand01(groupIndex, 407, scatterSeed));
                return true;
            }
            if (kind < 0.82f && !trashBinPropMesh_.vertices.empty()) {
                float clusterRadius = 0.70f + Rand01(groupIndex, 409, scatterSeed) * 0.36f;
                if (!longFloorFootprintClear(px, pz, clusterRadius * 2.0f + 1.10f, clusterRadius * 2.0f + 1.00f, yaw, 0.070f)) return false;
                float furnitureYaw = yaw + (Rand01(groupIndex, 411, scatterSeed) - 0.5f) * 0.85f;
                float furnitureA = yaw + kPi * (0.40f + Rand01(groupIndex, 413, scatterSeed) * 0.40f);
                if (Rand01(groupIndex, 415, scatterSeed) < 0.45f) {
                    float tableW = 0.82f + Rand01(groupIndex, 417, scatterSeed) * 0.26f;
                    float tableD = 0.52f + Rand01(groupIndex, 419, scatterSeed) * 0.20f;
                    float smallTableSeed = Rand01(groupIndex, 421, scatterSeed);
                    float tableX = px + std::sin(furnitureA) * 0.38f;
                    float tableZ = pz + std::cos(furnitureA) * 0.38f;
                    if (addSideTable(tableX, tableZ, tableW, tableD, furnitureYaw, smallTableSeed) &&
                        Rand01(groupIndex, 422, scatterSeed) < 0.46f) {
                        addDeskLampOnSurface(tableX, tableZ, 0.720f + smallTableSeed * 0.10f,
                            furnitureYaw, tableW, tableD, Rand01(groupIndex, 424, scatterSeed));
                    }
                } else if (Rand01(groupIndex, 423, scatterSeed) < 0.55f) {
                    addChair({px + std::sin(furnitureA) * 0.48f, 0.0f, pz + std::cos(furnitureA) * 0.48f},
                        furnitureYaw + kPi, Rand01(groupIndex, 425, scatterSeed) < 0.5f);
                }
                int bins = 2 + static_cast<int>(Rand01(groupIndex, 427, scatterSeed) * 3.0f);
                int placedBins = 0;
                for (int i = 0; i < bins; ++i) {
                    float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(bins)
                        + (Rand01(groupIndex * 17 + i, 429, scatterSeed) - 0.5f) * 0.72f;
                    float radius = clusterRadius * (0.46f + Rand01(groupIndex * 17 + i, 431, scatterSeed) * 0.52f);
                    bool tipped = i == 0
                        ? Rand01(groupIndex * 17 + i, 433, scatterSeed) < 0.70f
                        : Rand01(groupIndex * 17 + i, 433, scatterSeed) < 0.46f;
                    if (addTrashBin(px + std::sin(a) * radius, pz + std::cos(a) * radius,
                            a + (Rand01(groupIndex * 17 + i, 435, scatterSeed) - 0.5f) * 0.80f,
                            tipped, Rand01(groupIndex * 17 + i, 437, scatterSeed))) {
                        ++placedBins;
                    }
                }
                if (placedBins == 0 && addTrashBin(px, pz, yaw, Rand01(groupIndex, 439, scatterSeed) < 0.55f,
                        Rand01(groupIndex, 441, scatterSeed))) {
                    ++placedBins;
                }
                return placedBins > 0;
            }
            int chairs = 3 + static_cast<int>(Rand01(groupIndex, 409, scatterSeed) * 4.0f);
            float ring = 0.88f + Rand01(groupIndex, 407, scatterSeed) * 0.28f;
            if (!longFloorFootprintClear(px, pz, ring * 2.0f + 1.02f, ring * 2.0f + 1.02f, yaw, 0.070f)) return false;
            int placedChairs = 0;
            for (int i = 0; i < chairs; ++i) {
                float a = yaw + static_cast<float>(i) * kPi * 2.0f / static_cast<float>(chairs);
                float radius = ring + Rand01(groupIndex * 11 + i, 419, scatterSeed) * 0.20f;
                float cx = px + std::sin(a) * radius;
                float cz = pz + std::cos(a) * radius;
                float chairYaw = a + RandRange(-0.50f, 0.50f);
                bool placed = false;
                if (Rand01(groupIndex * 11 + i, 421, scatterSeed) < 0.34f) {
                    placed = addTippedChair(cx, cz, chairYaw, true, Rand01(groupIndex * 11 + i, 431, scatterSeed));
                } else {
                    placed = addChair({cx, 0.0f, cz}, chairYaw, true);
                }
                if (placed) ++placedChairs;
            }
            if (Rand01(groupIndex, 443, scatterSeed) < 0.58f) {
                addTrashBin(px, pz, yaw + Rand01(groupIndex, 445, scatterSeed) * kPi,
                    Rand01(groupIndex, 447, scatterSeed) < 0.42f, Rand01(groupIndex, 449, scatterSeed));
            }
            return placedChairs > 0;
        };

        constexpr float kA4PaperShortMeters = 0.210f;
        constexpr float kA4PaperLongMeters = 0.297f;
        auto addPaperAt = [&](float px, float pz, float yaw, float lift) {
            if (!floorFootprintClear(px, pz, kA4PaperShortMeters, kA4PaperLongMeters, yaw)) return false;
            AddFloorCard(vertices, indices, {px, 0.0f, pz}, kA4PaperShortMeters, kA4PaperLongMeters, yaw, lift, 9.0f);
            return true;
        };

        auto addLoosePapers = [&](Tile t, int count, bool hallwaySpill) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float spreadX = hallwaySpill ? tileW * 1.55f : tileW * 0.82f;
            float spreadZ = hallwaySpill ? tileD * 1.55f : tileD * 0.82f;
            for (int p = 0; p < count; ++p) {
                float px = c.x + (tileHash(t.x, t.y, 5.0f + p * 1.71f) - 0.5f) * spreadX;
                float pz = c.z + (tileHash(t.x, t.y, 7.0f + p * 1.93f) - 0.5f) * spreadZ;
                float yaw = tileHash(t.x, t.y, 9.0f + p * 2.11f) * kPi * 2.0f;
                float lift = 0.040f + p * 0.0016f + tileHash(t.x, t.y, 17.0f + p) * 0.006f;
                if (addPaperAt(px, pz, yaw, lift) &&
                    tileHash(t.x, t.y, 31.0f + p * 2.37f) < 0.010f) {
                    addCassetteAt(px, pz, yaw, lift + 0.003f, tileHash(t.x, t.y, 37.0f + p));
                }
            }
            propLookPoints_.push_back({c.x, 0.18f, c.z});
        };

        auto addWallVent = [&](Tile t, int side, float seed) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
            if (wallSpan < 1.38f || wallH < 1.12f) return;
            float lateralLimit = std::max(0.0f, wallSpan * 0.5f - 0.68f);
            float lateral = (tileHash(t.x, t.y, 41.0f + seed) - 0.5f) * lateralLimit * 2.0f;
            bool lowVent = tileHash(t.x, t.y, 42.2f + seed) < 0.34f;
            float ventW = lowVent ? 0.68f : 0.92f;
            float ventH = lowVent ? 0.24f : 0.34f;
            constexpr float kLowVentFloorGap = 0.085f;
            constexpr float kHighVentCeilingGap = 0.095f;
            constexpr float kVentVerticalClearance = 0.055f;
            float minVentY = ventH * 0.5f + kVentVerticalClearance;
            float maxVentY = std::max(minVentY, wallH - ventH * 0.5f - kVentVerticalClearance);
            float yCenter = lowVent
                ? kLowVentFloorGap + ventH * 0.5f
                : wallH - kHighVentCeilingGap - ventH * 0.5f;
            yCenter = std::clamp(yCenter, minVentY, maxVentY);
            float yaw = 0.0f;
            XMFLOAT3 center{c.x, yCenter, c.z};
            if (side == 0) {
                yaw = 0.0f;
                center = {c.x + lateral, yCenter, c.z - tileD * 0.5f + 0.018f};
            } else if (side == 1) {
                yaw = kPi;
                center = {c.x + lateral, yCenter, c.z + tileD * 0.5f - 0.018f};
            } else if (side == 2) {
                yaw = kPi * 0.5f;
                center = {c.x - tileW * 0.5f + 0.018f, yCenter, c.z + lateral};
            } else {
                yaw = -kPi * 0.5f;
                center = {c.x + tileW * 0.5f - 0.018f, yCenter, c.z + lateral};
            }
            if (!airVentPropMesh_.vertices.empty()) {
                float spanX = std::max(0.001f, propSpan(airVentPropMesh_, 0));
                float spanY = std::max(0.001f, propSpan(airVentPropMesh_, 1));
                float scale = std::min(ventW / spanX, ventH / spanY);
                scale = std::clamp(scale, 0.05f, 8.0f);
                if (AppendStaticPropMesh(vertices, indices, airVentPropMesh_, center, yaw, scale, scale, scale)) {
                    XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                    XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                    XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
                    XMFLOAT3 emitterPos = Add3(center, OrientedOffset(right, up, forward, 0.0f, lowVent ? 0.02f : -0.02f, 0.090f));
                    steamEmitters_.push_back({emitterPos, forward, tileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
                    propLookPoints_.push_back(center);
                    return;
                }
            }
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto voff = [&](float x, float y, float z) {
                return Add3(center, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.010f), {ventW * 0.50f, ventH * 0.50f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(0.0f, 0.0f, 0.022f), {ventW * 0.38f, ventH * 0.31f, 0.006f}, yaw, 5.0f);
            AddOrientedBox(vertices, indices, voff(-ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(ventW * 0.47f, 0.0f, 0.027f), {0.014f, ventH * 0.43f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(0.0f, -ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
            AddOrientedBox(vertices, indices, voff(0.0f, ventH * 0.43f, 0.027f), {ventW * 0.45f, 0.012f, 0.010f}, yaw, 8.0f);
            for (int s = -3; s <= 3; ++s) {
                float yOff = static_cast<float>(s) * ventH * 0.115f;
                float stagger = (s & 1) ? 0.006f : -0.004f;
                AddOrientedBox(vertices, indices, voff(stagger, yOff, 0.034f + static_cast<float>(s + 3) * 0.0008f),
                    {ventW * 0.36f, 0.006f, 0.007f}, yaw, 8.0f);
            }
            const XMFLOAT2 screws[] = {{-0.43f, -0.37f}, {0.43f, -0.37f}, {-0.43f, 0.37f}, {0.43f, 0.37f}};
            for (const XMFLOAT2& screw : screws) {
                AddOrientedBox(vertices, indices, voff(screw.x * ventW, screw.y * ventH, 0.039f), {0.014f, 0.014f, 0.005f}, yaw, 10.0f);
            }
            steamEmitters_.push_back({voff(0.0f, lowVent ? 0.02f : -0.02f, 0.082f), forward, tileHash(t.x, t.y, 45.0f + seed) * 5.0f, false});
            propLookPoints_.push_back(center);
        };

        auto addMetalCabinet = [&](float px, float pz, float yaw, float seed, bool tall) {
            XMFLOAT3 cab = cabinetSize(tall);
            float width = cab.x;
            float height = cab.y;
            float depth = cab.z;
            if (cabinetPropMesh_.vertices.empty()) return false;
            if (!reserveFloorFootprint(px, pz, width, depth, yaw, 0.090f)) return false;
            float scaleX = width / propSpan(cabinetPropMesh_, 0);
            float scaleY = height / propSpan(cabinetPropMesh_, 1);
            float scaleZ = depth / propSpan(cabinetPropMesh_, 2);
            AppendStaticPropMesh(vertices, indices, cabinetPropMesh_, {px, 0.0f, pz}, yaw, scaleX, scaleY, scaleZ,
                0.0f, 10.0f, &propShadowIndices);
            addBakedPropShadow(px, pz, width, depth, height, yaw, seed);
            propLookPoints_.push_back({px, height * 0.74f, pz});
            return true;
            XMFLOAT3 c{px, height * 0.5f, pz};
            XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
            auto off = [&](float x, float y, float z) {
                return Add3(c, OrientedOffset(right, up, forward, x, y, z));
            };

            float bodyMat = 8.0f;
            float trimMat = 10.0f;
            AddOrientedBox(vertices, indices, c, {width * 0.48f, height * 0.49f, depth * 0.48f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.505f, -depth * 0.025f), {width * 0.52f, 0.026f, depth * 0.52f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, -height * 0.505f, 0.0f), {width * 0.51f, 0.024f, depth * 0.50f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(-width * 0.505f, 0.0f, -depth * 0.02f), {0.018f, height * 0.48f, depth * 0.49f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(width * 0.505f, 0.0f, -depth * 0.02f), {0.018f, height * 0.48f, depth * 0.49f}, yaw, trimMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.03f, -depth * 0.515f), {width * 0.44f, height * 0.42f, 0.010f}, yaw, bodyMat);
            AddOrientedBox(vertices, indices, off(0.0f, height * 0.03f, -depth * 0.542f), {0.010f, height * 0.40f, 0.012f}, yaw, trimMat);

            int rows = tall ? 4 : 2;
            float usableH = height * 0.78f;
            float drawerH = usableH / static_cast<float>(rows) * 0.76f;
            float startY = -usableH * 0.5f + drawerH * 0.5f;
            for (int r = 0; r < rows; ++r) {
                float ry = startY + static_cast<float>(r) * usableH / static_cast<float>(rows);
                float variant = LampHash(seed * 17.0f + static_cast<float>(r) * 3.3f, px - pz);
                float drawerW = width * (tall ? 0.78f : 0.82f);
                if (variant < 0.18f) {
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.555f), {drawerW * 0.46f, drawerH * 0.40f, 0.012f}, yaw, trimMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.575f), {drawerW * 0.32f, 0.010f, 0.010f}, yaw, trimMat);
                } else if (variant < 0.46f) {
                    float pull = 0.13f + variant * 0.30f;
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.56f - pull * 0.48f), {drawerW * 0.44f, drawerH * 0.38f, pull * 0.5f}, yaw, bodyMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.555f), {drawerW * 0.48f, drawerH * 0.42f, 0.010f}, yaw, trimMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry + drawerH * 0.02f, -depth * 0.59f - pull), {drawerW * 0.30f, 0.010f, 0.010f}, yaw, trimMat);
                } else {
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.560f), {drawerW * 0.46f, drawerH * 0.40f, 0.012f}, yaw, bodyMat);
                    AddOrientedBox(vertices, indices, off(0.0f, ry + drawerH * 0.31f, -depth * 0.580f), {drawerW * 0.40f, 0.006f, 0.006f}, yaw, trimMat);
                }
                if (variant >= 0.18f) {
                    AddOrientedBox(vertices, indices, off(0.0f, ry, -depth * 0.595f), {drawerW * 0.20f, 0.008f, 0.010f}, yaw, trimMat);
                }
            }
            int ventSlots = tall ? 4 : 3;
            for (int s = 0; s < ventSlots; ++s) {
                float sy = height * 0.30f - static_cast<float>(s) * 0.055f;
                AddOrientedBox(vertices, indices, off(width * 0.515f, sy, depth * 0.10f), {0.006f, 0.007f, depth * 0.22f}, yaw, trimMat);
            }
            for (int sx = -1; sx <= 1; sx += 2) {
                for (int sz = -1; sz <= 1; sz += 2) {
                    AddOrientedBox(vertices, indices, off(sx * width * 0.36f, -height * 0.51f - 0.030f, sz * depth * 0.31f), {0.030f, 0.030f, 0.030f}, yaw, trimMat);
                }
            }

            if (!tall && seed > 0.56f) {
                float doorYaw = yaw + (seed > 0.76f ? -0.48f : 0.48f);
                float side = seed > 0.76f ? 1.0f : -1.0f;
                AddOrientedBox(vertices, indices,
                    off(side * width * 0.30f, height * 0.02f, -depth * 0.65f),
                    {width * 0.20f, height * 0.32f, 0.012f}, doorYaw, bodyMat);
                AddOrientedBox(vertices, indices,
                    off(side * width * 0.12f, height * 0.02f, -depth * 0.70f),
                    {0.010f, height * 0.12f, 0.010f}, doorYaw, trimMat);
            }
            propLookPoints_.push_back({px, height * 0.74f, pz});
            return true;
        };

        auto addMetalCabinetAgainstWall = [&](Tile t, int side, float seed) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            bool tall = seed < 0.62f;
            XMFLOAT3 cab = cabinetSize(tall);
            float width = cab.x;
            float depth = cab.z;
            float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
            float lateralLimit = std::max(0.0f, wallSpan * 0.5f - width * 0.5f - 0.10f);
            float lateral = (LampHash(c.x + seed * 9.1f, c.z - seed * 3.7f) - 0.5f) * lateralLimit * 2.0f;
            float px = c.x;
            float pz = c.z;
            float yaw = 0.0f;
            if (side == 0) {
                yaw = kPi;
                px = c.x + lateral;
                pz = c.z - tileD * 0.5f + depth * 0.5f + 0.075f;
            } else if (side == 1) {
                yaw = 0.0f;
                px = c.x + lateral;
                pz = c.z + tileD * 0.5f - depth * 0.5f - 0.075f;
            } else if (side == 2) {
                yaw = -kPi * 0.5f;
                px = c.x - tileW * 0.5f + depth * 0.5f + 0.075f;
                pz = c.z + lateral;
            } else {
                yaw = kPi * 0.5f;
                px = c.x + tileW * 0.5f - depth * 0.5f - 0.075f;
                pz = c.z + lateral;
            }
            return addMetalCabinet(px, pz, yaw, seed, tall);
        };

        auto addExitDoor = [&]() {
            if (!exitPortal.valid) return;
            XMFLOAT3 c = maze_.WorldCenter(exitPortal.tile, 0.0f);
            float bx = exitPortal.wallCenter.x;
            float bz = exitPortal.wallCenter.z;
            XMFLOAT3 inward = exitPortal.inward;
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 right = exitPortal.right;
            XMFLOAT3 doorCenter{bx + inward.x * 0.075f, 1.10f, bz + inward.z * 0.075f};
            XMFLOAT3 forward = inward;
            exitDoorCenter_ = doorCenter;
            exitDoorNormal_ = inward;
            exitDoorRight_ = right;
            exitDoorHinge_ = Add3(doorCenter, OrientedOffset(right, up, forward, -0.55f, 0.0f, 0.0f));
            AddOrientedBox(vertices, indices, Add3(doorCenter, OrientedOffset(right, up, forward, -0.62f, 0.0f, 0.0f)), {0.035f, 1.18f, 0.055f}, exitPortal.yaw, 10.0f);
            AddOrientedBox(vertices, indices, Add3(doorCenter, OrientedOffset(right, up, forward, 0.62f, 0.0f, 0.0f)), {0.035f, 1.18f, 0.055f}, exitPortal.yaw, 10.0f);
            AddOrientedBox(vertices, indices, Add3(doorCenter, OrientedOffset(right, up, forward, 0.0f, 1.17f, 0.0f)), {0.68f, 0.055f, 0.055f}, exitPortal.yaw, 10.0f);
            XMFLOAT3 sign = Add3(doorCenter, OrientedOffset(right, up, forward, 0.0f,
                std::clamp(wallH - doorCenter.y - 0.19f, 0.78f, 1.18f), 0.073f));
            exitSignLightPos_ = Add3(sign, OrientedOffset(right, up, forward, 0.0f, -0.02f, 0.22f));
            exitSignLightStrength_ = 1.0f;
            if (!exitSignPropMesh_.vertices.empty()) {
                float spanX = std::max(0.001f, propSpan(exitSignPropMesh_, 0));
                float spanY = std::max(0.001f, propSpan(exitSignPropMesh_, 1));
                float targetW = std::min(1.24f, exitPortal.halfSpan * 1.48f);
                float targetH = std::clamp(wallH * 0.16f, 0.30f, 0.42f);
                float scale = std::clamp(std::min(targetW / spanX, targetH / spanY), 0.05f, 8.0f);
                XMFLOAT3 localCenter{
                    (exitSignPropMesh_.min.x + exitSignPropMesh_.max.x) * 0.5f,
                    (exitSignPropMesh_.min.y + exitSignPropMesh_.max.y) * 0.5f,
                    (exitSignPropMesh_.min.z + exitSignPropMesh_.max.z) * 0.5f
                };
                auto appendExitSignModel = [&](XMFLOAT3 signCenter, XMFLOAT3 signRight, XMFLOAT3 signForward, float yaw) {
                    XMFLOAT3 origin = Add3(signCenter, Add3(Scale3(signRight, -localCenter.x * scale),
                        Add3(Scale3(up, -localCenter.y * scale), Scale3(signForward, -localCenter.z * scale))));
                    return AppendStaticPropMesh(vertices, indices, exitSignPropMesh_, origin, yaw, scale, scale, scale, 0.0f, 7.0f);
                };
                bool appended = appendExitSignModel(Add3(sign, Scale3(forward, 0.006f)), right, forward, exitPortal.yaw);
                appended = appendExitSignModel(Add3(sign, Scale3(forward, -0.006f)), Scale3(right, -1.0f), Scale3(forward, -1.0f), exitPortal.yaw + kPi) || appended;
                if (!appended) {
                    StartupProfileLine(L"Emergency exit sign mesh was loaded but could not be appended; no handmade fallback was drawn.");
                }
            } else {
                StartupProfileLine(L"Emergency exit sign mesh missing; no handmade fallback was drawn.");
            }
        };
        addExitDoor();

        float paperDensity = std::clamp(settings_.paperDensity, 0.0f, 4.0f);
        float hallwayPaperDensity = std::clamp(settings_.hallwayPaperRunDensity, 0.0f, 4.0f);
        float chairChance = std::min(1.0f, 0.030f * std::clamp(settings_.chairDensity, 0.0f, 4.0f));
        float loosePaperChance = std::min(1.0f, 0.082f * paperDensity);
        float paperHallwayChance = std::min(1.0f, 0.13f * hallwayPaperDensity);
        float ventChance = gEffectDebugViewer && gDebugSliceEffect == DebugSliceEffect::AirVents ? 1.0f : 0.026f;
        float waterDamageChance = 0.0f;
        float waterLikeDamageChance = std::min(1.0f, 0.050f * std::clamp(settings_.waterDamageDensity, 0.0f, 4.0f));

        auto waterMaterial = [](float seed, float bandStart, float bandWidth) {
            float h = std::fmod(std::abs(seed) * 37.719f + 0.137f, 1.0f);
            float safeWidth = std::max(0.0f, std::min(bandWidth, 0.043f - bandStart));
            return 11.006f + bandStart + h * safeWidth;
        };
        constexpr float kWaterFloorLift = 0.035f;
        float waterCeilingY = wallH - 0.020f;
        struct WaterTileSurface {
            bool active = false;
            bool suppressCard = false;
            int side = 0;
            int mode = 0;
            float seed = 0.0f;
            float score = -1.0f;
        };
        std::vector<WaterTileSurface> floorWaterTiles(static_cast<size_t>(maze_.w * maze_.h));
        std::vector<WaterTileSurface> ceilingWaterTiles(static_cast<size_t>(maze_.w * maze_.h));
        auto waterTileIndex = [&](Tile t) {
            return static_cast<size_t>(t.y * maze_.w + t.x);
        };
        auto mergeWaterMode = [](int a, int b) {
            if (a == 3 && b == 3) return 3;
            if (a == 3) a = 0;
            if (b == 3) b = 0;
            bool central = a == 0 || a == 1 || b == 0 || b == 1;
            bool edge = a == 1 || a == 2 || b == 1 || b == 2;
            if (central && edge) return 1;
            if (edge) return 2;
            return 0;
        };
        auto oppositeWaterSide = [](int side) {
            if (side == 0) return 1;
            if (side == 1) return 0;
            if (side == 2) return 3;
            return 2;
        };
        auto markWaterTile = [&](Tile t, bool ceiling, int side, int mode, float seed, float score, bool suppressCard = false) {
            if (!maze_.IsOpen(t.x, t.y) || (!gEffectDebugViewer && (t == maze_.start || t == maze_.exit))) return;
            WaterTileSurface& surface = (ceiling ? ceilingWaterTiles : floorWaterTiles)[waterTileIndex(t)];
            side = std::clamp(side, 0, 3);
            mode = std::clamp(mode, 0, 3);
            if (!surface.active) {
                surface.active = true;
                surface.suppressCard = suppressCard;
                surface.side = side;
                surface.mode = mode;
                surface.seed = seed;
                surface.score = score;
                return;
            }
            surface.suppressCard = surface.suppressCard && suppressCard;
            surface.mode = mergeWaterMode(surface.mode, mode);
            if (score >= surface.score) {
                surface.side = side;
                surface.seed = seed;
                surface.score = score;
            }
        };
        auto emitFloorWaterPoolCard = [&](Tile owner, float cx, float cz, int side, float seed,
                                          float width, float depth, float yaw, float uvModeBase,
                                          float score) {
            if (!maze_.IsOpen(owner.x, owner.y) || (!gEffectDebugViewer && (owner == maze_.start || owner == maze_.exit))) return false;
            float w = width;
            float d = depth;
            for (int attempt = 0; attempt < 4; ++attempt) {
                if (footprintFitsMaze(cx, cz, w, d, yaw, 0.020f)) {
                    float cYaw = std::cos(yaw);
                    float sYaw = std::sin(yaw);
                    XMFLOAT3 right{cYaw, 0.0f, -sYaw};
                    XMFLOAT3 forward{sYaw, 0.0f, cYaw};
                    XMFLOAT3 center{cx, kWaterFloorLift, cz};
                    XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(forward,  d * 0.5f)));
                    XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(forward,  d * 0.5f)));
                    XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(forward, -d * 0.5f)));
                    XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(forward, -d * 0.5f)));
                    AddQuadUV(vertices, waterIndices, a, b, c0, d0, {0, 1, 0}, right,
                        {0, uvModeBase}, {1, uvModeBase}, {1, uvModeBase + 1.0f}, {0, uvModeBase + 1.0f},
                        waterMaterial(seed, 0.0f, 0.014f));
                    markWaterTile(owner, false, side, 0, seed, score, true);
                    return true;
                }
                w *= 0.86f;
                d *= 0.86f;
            }
            return false;
        };
        auto addCircularFloorWaterPool = [&](Tile origin, int side, float seed, float strength) {
            if (!maze_.IsOpen(origin.x, origin.y) || (!gEffectDebugViewer && (origin == maze_.start || origin == maze_.exit))) return false;
            XMFLOAT3 c = maze_.WorldCenter(origin, 0.0f);
            float minTile = std::max(0.10f, std::min(tileW, tileD));
            float h0 = LampHash(seed * 17.0f + c.x, c.z + 3.1f);
            float h1 = LampHash(seed * 23.0f - c.z, c.x + 5.7f);
            float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
            float radius = minTile * (0.42f + h0 * 0.20f + std::clamp(strength, 0.35f, 1.35f) * 0.18f);
            float cx = c.x + (h1 - 0.5f) * tileW * 0.20f;
            float cz = c.z + (h2 - 0.5f) * tileD * 0.20f;
            float yaw = (LampHash(seed * 43.0f + c.x, c.z) - 0.5f) * 0.18f;
            float width = radius * 2.0f;
            float depth = radius * (1.88f + LampHash(seed * 47.0f - c.x, c.z) * 0.16f);
            return emitFloorWaterPoolCard(origin, cx, cz, side, seed, width, depth, yaw, 0.0f, 1.34f);
        };
        auto markWaterBlob = [&](Tile origin, bool ceiling, int primarySide, int centerMode, float seed, float strength) {
            if (!ceiling && !gEffectDebugViewer) {
                if (addCircularFloorWaterPool(origin, primarySide, seed, strength)) return;
            }
            int radiusTiles = 0;
            if (gEffectDebugViewer) {
                radiusTiles = std::max(1, gDebugSliceTiles / 2);
            } else if (ceiling && centerMode != 3 && strength > 1.02f) {
                radiusTiles = 1;
            }
            float blobRadius = gEffectDebugViewer
                ? (0.70f + static_cast<float>(gDebugSliceTiles) * 0.36f + LampHash(seed * 19.0f, seed * 7.0f) * 0.22f)
                : (ceiling
                    ? (0.82f + std::clamp(strength, 0.35f, 1.35f) * 0.25f)
                    : (0.46f + std::clamp(strength, 0.35f, 1.35f) * 0.18f));
            for (int dy = -radiusTiles; dy <= radiusTiles; ++dy) {
                for (int dx = -radiusTiles; dx <= radiusTiles; ++dx) {
                    Tile t{origin.x + dx, origin.y + dy};
                    if (!maze_.IsOpen(t.x, t.y)) continue;
                    float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                    float edgeNoise = (tileHash(t.x, t.y, seed * 61.0f + (ceiling ? 17.0f : 5.0f)) - 0.5f) * 0.46f;
                    if (dist > 0.01f && dist + edgeNoise > blobRadius) continue;
                    int side = primarySide;
                    if (std::abs(dx) > std::abs(dy)) side = dx < 0 ? 2 : 3;
                    else if (std::abs(dy) > 0) side = dy < 0 ? 0 : 1;
                    int mode = centerMode;
                    if (dist > 0.75f) {
                        mode = ceiling ? 1 : 0;
                    }
                    float score = 1.30f - dist * 0.18f + tileHash(t.x, t.y, seed * 37.0f + 23.0f) * 0.08f;
                    markWaterTile(t, ceiling, side, mode,
                        seed + static_cast<float>(dx) * 0.071f + static_cast<float>(dy) * 0.113f,
                        score);
                }
            }
        };
        auto addWaterAttentionPoint = [&](const XMFLOAT3& pos, const XMFLOAT3& source, const XMFLOAT3& normal,
            float radius, float seed, bool requireFacing) {
            if (gEffectDebugViewer || bloodScarePoints_.size() > 384) return;
            BloodScarePoint scare{};
            scare.pos = pos;
            scare.source = source;
            scare.normal = normal;
            scare.radius = std::clamp(radius, maze_.TileAverage() * 0.78f, maze_.TileAverage() * 1.45f);
            scare.focusDelaySeconds = 0.20f + LampHash(seed * 19.0f + pos.x, pos.z - seed * 7.0f) * 0.46f;
            scare.dreadScale = 0.42f;
            scare.requireFacing = requireFacing;
            scare.revealBlood = false;
            bloodScarePoints_.push_back(scare);
        };
        auto emitWaterTileCard = [&](Tile t, bool ceiling, const WaterTileSurface& surface) {
            if (!surface.active) return;
            if (surface.suppressCard) return;
            float l = ox + static_cast<float>(t.x) * tileW;
            float r = l + tileW;
            float z0 = oz + static_cast<float>(t.y) * tileD;
            float z1 = z0 + tileD;
            float y = ceiling ? waterCeilingY : kWaterFloorLift;
            float u = static_cast<float>(surface.side);
            int neighborMask = 0;
            auto neighborWet = [&](int nx, int ny) {
                if (!maze_.IsOpen(nx, ny)) return false;
                const WaterTileSurface& neighbor = (ceiling ? ceilingWaterTiles : floorWaterTiles)[static_cast<size_t>(ny * maze_.w + nx)];
                return neighbor.active && !neighbor.suppressCard;
            };
            if (neighborWet(t.x, t.y - 1)) neighborMask |= 1;
            if (neighborWet(t.x, t.y + 1)) neighborMask |= 2;
            if (neighborWet(t.x - 1, t.y)) neighborMask |= 4;
            if (neighborWet(t.x + 1, t.y)) neighborMask |= 8;
            if (neighborWet(t.x - 1, t.y - 1)) neighborMask |= 16;
            if (neighborWet(t.x + 1, t.y - 1)) neighborMask |= 32;
            if (neighborWet(t.x - 1, t.y + 1)) neighborMask |= 64;
            if (neighborWet(t.x + 1, t.y + 1)) neighborMask |= 128;
            float vMode = static_cast<float>(surface.mode + neighborMask * 8);
            float material = waterMaterial(surface.seed, 0.0f, 0.014f);
            float h0 = LampHash(surface.seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y) + 3.1f);
            float h1 = LampHash(surface.seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x) + 5.7f);
            float h2 = LampHash(surface.seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);
            float h3 = LampHash(surface.seed * 41.0f + static_cast<float>(t.x) * 1.7f, static_cast<float>(t.y) * 2.3f);
            float sizeScore = std::clamp(0.70f + (surface.score - 0.75f) * 0.22f, 0.54f, 1.05f);
            float halfW = tileW * (ceiling ? (0.30f + h0 * 0.20f) : (0.20f + h0 * 0.17f)) * sizeScore;
            float halfD = tileD * (ceiling ? (0.30f + h1 * 0.20f) : (0.18f + h1 * 0.16f)) * sizeScore;
            if (surface.mode == 3) {
                halfW *= 0.62f;
                halfD *= 0.62f;
            }
            float cx = (l + r) * 0.5f + (h2 - 0.5f) * tileW * (ceiling ? 0.18f : 0.24f);
            float cz = (z0 + z1) * 0.5f + (h3 - 0.5f) * tileD * (ceiling ? 0.18f : 0.22f);
            auto pullTowardSide = [&](int side, float amount) {
                if (side == 0) cz -= tileD * amount;
                else if (side == 1) cz += tileD * amount;
                else if (side == 2) cx -= tileW * amount;
                else cx += tileW * amount;
            };
            if (surface.mode > 0 && surface.mode != 3) {
                pullTowardSide(surface.side, ceiling ? 0.16f : 0.13f);
                if (surface.side == 0 || surface.side == 1) halfD = std::max(halfD, tileD * (ceiling ? 0.43f : 0.36f));
                else halfW = std::max(halfW, tileW * (ceiling ? 0.43f : 0.36f));
            }
            if (neighborMask & 1) z0 = z0 + tileD * 0.015f; else z0 = std::max(z0 + tileD * 0.055f, cz - halfD);
            if (neighborMask & 2) z1 = z1 - tileD * 0.015f; else z1 = std::min(z1 - tileD * 0.055f, cz + halfD);
            if (neighborMask & 4) l = l + tileW * 0.015f; else l = std::max(l + tileW * 0.055f, cx - halfW);
            if (neighborMask & 8) r = r - tileW * 0.015f; else r = std::min(r - tileW * 0.055f, cx + halfW);
            if (r - l < tileW * 0.12f || z1 - z0 < tileD * 0.12f) return;
            if (ceiling) {
                AddQuadUV(vertices, waterIndices,
                    {l, y, z0}, {r, y, z0}, {r, y, z1}, {l, y, z1},
                    {0, -1, 0}, {1, 0, 0},
                    {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
            } else {
                AddQuadUV(vertices, waterIndices,
                    {l, y, z1}, {r, y, z1}, {r, y, z0}, {l, y, z0},
                    {0, 1, 0}, {1, 0, 0},
                    {u, vMode}, {u + 1.0f, vMode}, {u + 1.0f, vMode + 1.0f}, {u, vMode + 1.0f}, material);
            }
        };
        auto neighborForSideWater = [](Tile t, int side) {
            if (side == 0) return Tile{t.x, t.y - 1};
            if (side == 1) return Tile{t.x, t.y + 1};
            if (side == 2) return Tile{t.x - 1, t.y};
            return Tile{t.x + 1, t.y};
        };
        auto sideDirectionWater = [](int side) {
            if (side == 0) return XMFLOAT3{0.0f, 0.0f, -1.0f};
            if (side == 1) return XMFLOAT3{0.0f, 0.0f, 1.0f};
            if (side == 2) return XMFLOAT3{-1.0f, 0.0f, 0.0f};
            return XMFLOAT3{1.0f, 0.0f, 0.0f};
        };
        auto sideForwardYawWater = [](int side) {
            if (side == 0) return kPi;
            if (side == 1) return 0.0f;
            if (side == 2) return -kPi * 0.5f;
            return kPi * 0.5f;
        };
        auto wallHasWaterSurface = [&](Tile tile, int side) {
            if (!maze_.IsOpen(tile.x, tile.y)) return false;
            if (side == 0) return !maze_.IsOpen(tile.x, tile.y - 1);
            if (side == 1) return !maze_.IsOpen(tile.x, tile.y + 1);
            if (side == 2) return !maze_.IsOpen(tile.x - 1, tile.y);
            return !maze_.IsOpen(tile.x + 1, tile.y);
        };
        auto wallWaterSupportSpan = [&](Tile t, int side, float& minAlong, float& maxAlong) {
            if (!wallHasWaterSurface(t, side)) return false;
            if (side == 0 || side == 1) {
                int x0 = t.x;
                int x1 = t.x;
                while (wallHasWaterSurface({x0 - 1, t.y}, side)) --x0;
                while (wallHasWaterSurface({x1 + 1, t.y}, side)) ++x1;
                minAlong = ox + static_cast<float>(x0) * tileW;
                maxAlong = ox + static_cast<float>(x1 + 1) * tileW;
            } else {
                int y0 = t.y;
                int y1 = t.y;
                while (wallHasWaterSurface({t.x, y0 - 1}, side)) --y0;
                while (wallHasWaterSurface({t.x, y1 + 1}, side)) ++y1;
                minAlong = oz + static_cast<float>(y0) * tileD;
                maxAlong = oz + static_cast<float>(y1 + 1) * tileD;
            }
            return maxAlong - minAlong > 0.20f;
        };
        auto addWaterWallCard = [&](Tile t, int side, float lateral, float yCenter, float w, float h, bool sourceFromCeiling, float seed) {
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
            XMFLOAT3 right{1.0f, 0.0f, 0.0f};
            XMFLOAT3 center{c.x, yCenter, c.z};
            constexpr float kWaterWallDecalInset = 0.0045f;
            float minAlong = 0.0f;
            float maxAlong = 0.0f;
            if (!wallWaterSupportSpan(t, side, minAlong, maxAlong)) return false;
            constexpr float wallDecalMargin = 0.10f;
            minAlong += wallDecalMargin;
            maxAlong -= wallDecalMargin;
            float available = maxAlong - minAlong;
            if (available < 0.24f) return false;
            w = std::min(w, available);
            float halfW = w * 0.5f;
            float desiredAlong = (side == 0 || side == 1) ? c.x + lateral : c.z + lateral;
            float clampedAlong = std::clamp(desiredAlong, minAlong + halfW, maxAlong - halfW);
            if (side == 0) {
                normal = {0.0f, 0.0f, 1.0f};
                right = {1.0f, 0.0f, 0.0f};
                center = {clampedAlong, yCenter, c.z - tileD * 0.5f + kWaterWallDecalInset};
            } else if (side == 1) {
                normal = {0.0f, 0.0f, -1.0f};
                right = {-1.0f, 0.0f, 0.0f};
                center = {clampedAlong, yCenter, c.z + tileD * 0.5f - kWaterWallDecalInset};
            } else if (side == 2) {
                normal = {1.0f, 0.0f, 0.0f};
                right = {0.0f, 0.0f, 1.0f};
                center = {c.x - tileW * 0.5f + kWaterWallDecalInset, yCenter, clampedAlong};
            } else {
                normal = {-1.0f, 0.0f, 0.0f};
                right = {0.0f, 0.0f, -1.0f};
                center = {c.x + tileW * 0.5f - kWaterWallDecalInset, yCenter, clampedAlong};
            }
            h = sourceFromCeiling
                ? wallH - 0.003f
                : std::clamp(h, 0.08f, wallH - 0.003f);
            center.y = sourceFromCeiling
                ? wallH - 0.0015f - h * 0.5f
                : 0.0015f + h * 0.5f;
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up, -h * 0.5f)));
            XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up, -h * 0.5f)));
            XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up,  h * 0.5f)));
            XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up,  h * 0.5f)));
            float wallUvBase = sourceFromCeiling ? 3.0f : 4.0f;
            AddQuadUV(vertices, waterIndices, a, b, c0, d0, normal, right,
                {0, wallUvBase + 0.999f}, {1, wallUvBase + 0.999f},
                {1, wallUvBase + 0.001f}, {0, wallUvBase + 0.001f},
                waterMaterial(seed, 0.037f, 0.011f));
            if (sourceFromCeiling) {
                float minTile = std::max(0.10f, std::min(tileW, tileD));
                float h0 = LampHash(seed * 67.0f + center.x, center.z);
                float h1 = LampHash(seed * 71.0f - center.z, center.x);
                float poolW = std::clamp(w * (0.78f + h0 * 0.42f), minTile * 0.30f, minTile * 0.82f);
                float poolD = minTile * (0.34f + h1 * 0.30f);
                float poolYaw = std::atan2(normal.x, normal.z);
                XMFLOAT3 poolCenter = Add3({center.x, 0.0f, center.z}, Scale3(normal, poolD * 0.48f + 0.020f));
                emitFloorWaterPoolCard(t, poolCenter.x, poolCenter.z, side, seed + 0.83f,
                    poolW, poolD, poolYaw, 5.0f, 1.18f);
            }
            XMFLOAT3 attentionSource = sourceFromCeiling
                ? XMFLOAT3{center.x, wallH - 0.010f, center.z}
                : XMFLOAT3{center.x, 0.035f, center.z};
            addWaterAttentionPoint(center, attentionSource, normal,
                std::max(w, h) * 0.82f, seed + (sourceFromCeiling ? 0.57f : 0.29f), true);
            return true;
        };
        auto addWaterHorizontalSpill = [&](Tile t, int side, bool ceiling, float seed, float strength) {
            Tile n = neighborForSideWater(t, side);
            bool openNeighbor = maze_.IsOpen(n.x, n.y);
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            XMFLOAT3 dir = sideDirectionWater(side);
            XMFLOAT3 lateralAxis = (side == 0 || side == 1)
                ? XMFLOAT3{1.0f, 0.0f, 0.0f}
                : XMFLOAT3{0.0f, 0.0f, 1.0f};
            float axis = (side == 0 || side == 1) ? tileD : tileW;
            float cross = (side == 0 || side == 1) ? tileW : tileD;
            float h0 = LampHash(seed * 17.0f + c.x, c.z + static_cast<float>(side) * 3.1f);
            float h1 = LampHash(seed * 23.0f - c.z, c.x + static_cast<float>(side) * 5.7f);
            float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
            float length = axis * (0.50f + h0 * 0.58f) * std::clamp(strength, 0.68f, 1.32f);
            float width = cross * (0.24f + h1 * 0.42f);
            float along = openNeighbor ? axis * 0.50f + length * 0.04f : axis * 0.50f - length * 0.42f;
            float lateral = (h2 - 0.5f) * std::max(0.0f, cross - width) * 0.62f;
            XMFLOAT3 center = Add3(c, Add3(Scale3(dir, along), Scale3(lateralAxis, lateral)));
            float yaw = sideForwardYawWater(side) + (h1 - 0.5f) * 0.24f;
            if (footprintFitsMaze(center.x, center.z, width, length, yaw, openNeighbor ? 0.018f : 0.035f)) {
                if (ceiling) {
                    markWaterTile(t, true, side, 1, seed, 1.15f);
                    if (openNeighbor) {
                        markWaterTile(n, true, oppositeWaterSide(side), 2, seed, 0.86f);
                    }
                } else {
                    markWaterTile(t, false, side, 0, seed, 0.82f);
                    if (openNeighbor) {
                        markWaterTile(n, false, oppositeWaterSide(side), 0, seed, 0.62f);
                    }
                }
            }
            if (openNeighbor && LampHash(seed * 41.0f + static_cast<float>(side), c.x - c.z) < 0.48f) {
                XMFLOAT3 nc = maze_.WorldCenter(n, 0.0f);
                XMFLOAT3 satellite = Add3(nc, Add3(Scale3(dir, -axis * (0.18f + h2 * 0.12f)), Scale3(lateralAxis, -lateral * 0.36f)));
                float sw = width * (0.48f + h2 * 0.24f);
                float sl = length * (0.44f + h1 * 0.20f);
                if (footprintFitsMaze(satellite.x, satellite.z, sw, sl, yaw + (h0 - 0.5f) * 0.36f, 0.026f)) {
                    if (ceiling) {
                        markWaterTile(n, true, oppositeWaterSide(side), 2, seed + 0.031f, 0.74f);
                    } else {
                        markWaterTile(n, false, oppositeWaterSide(side), 0, seed + 0.031f, 0.54f);
                    }
                }
            } else if (!openNeighbor && wallHasWaterSurface(t, side)) {
                float wallW = width * (0.82f + h2 * 0.48f);
                float wallHgt = ceiling ? wallH - 0.003f : 0.12f + h0 * 0.18f;
                float yCenter = ceiling
                    ? wallH - wallHgt * 0.5f - 0.0015f
                    : wallHgt * 0.5f + 0.0015f;
                addWaterWallCard(t, side, lateral, yCenter, wallW, wallHgt, ceiling, seed + (ceiling ? 0.47f : 0.19f));
            }
        };
        auto emitFloorWaterBridge = [&](Tile t, int side) {
            Tile n = neighborForSideWater(t, side);
            if (!maze_.IsOpen(t.x, t.y) || !maze_.IsOpen(n.x, n.y)) return;
            const WaterTileSurface& aSurface = floorWaterTiles[waterTileIndex(t)];
            const WaterTileSurface& bSurface = floorWaterTiles[waterTileIndex(n)];
            if (!aSurface.active || !bSurface.active) return;
            if (aSurface.suppressCard || bSurface.suppressCard) return;

            float l = ox + static_cast<float>(t.x) * tileW;
            float r = l + tileW;
            float z0 = oz + static_cast<float>(t.y) * tileD;
            float z1 = z0 + tileD;
            constexpr float bridgeLift = 0.0018f;
            float y = kWaterFloorLift + bridgeLift;
            float material = waterMaterial((aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.071f, 0.0f, 0.014f);
            float seed = (aSurface.seed + bSurface.seed) * 0.5f + static_cast<float>(side) * 0.173f;
            float h0 = LampHash(seed * 17.0f + static_cast<float>(t.x), static_cast<float>(t.y));
            float h1 = LampHash(seed * 23.0f - static_cast<float>(t.y), static_cast<float>(t.x));
            float h2 = LampHash(seed * 31.0f + static_cast<float>(t.x) * 0.5f, static_cast<float>(t.y) * 0.5f);

            if (side == 1) {
                float seamZ = z1;
                float depth = std::min(tileD * (0.12f + h0 * 0.11f), 0.30f);
                float span = tileW * (0.28f + h1 * 0.34f);
                float cx = (l + r) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileW - span) * 0.72f;
                float x0 = std::max(l + tileW * 0.050f, cx - span * 0.5f);
                float x1 = std::min(r - tileW * 0.050f, cx + span * 0.5f);
                AddQuadUV(vertices, waterIndices,
                    {x0, y, seamZ + depth * 0.5f},
                    {x1, y, seamZ + depth * 0.5f},
                    {x1, y, seamZ - depth * 0.5f},
                    {x0, y, seamZ - depth * 0.5f},
                    {0, 1, 0}, {1, 0, 0},
                    {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
            } else if (side == 3) {
                float seamX = r;
                float width = std::min(tileW * (0.12f + h0 * 0.11f), 0.30f);
                float span = tileD * (0.28f + h1 * 0.34f);
                float cz = (z0 + z1) * 0.5f + (h2 - 0.5f) * std::max(0.0f, tileD - span) * 0.72f;
                float zz0 = std::max(z0 + tileD * 0.050f, cz - span * 0.5f);
                float zz1 = std::min(z1 - tileD * 0.050f, cz + span * 0.5f);
                AddQuadUV(vertices, waterIndices,
                    {seamX - width * 0.5f, y, zz1},
                    {seamX + width * 0.5f, y, zz1},
                    {seamX + width * 0.5f, y, zz0},
                    {seamX - width * 0.5f, y, zz0},
                    {0, 1, 0}, {1, 0, 0},
                    {0, 4}, {1, 4}, {1, 5}, {0, 5}, material);
            }
        };

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                if (!maze_.IsOpen(x, y)) continue;
                Tile t{x, y};
                if (!gEffectDebugViewer && (t == maze_.start || t == maze_.exit)) continue;
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float h0 = tileHash(x, y, 0.1f);
                float h1 = tileHash(x, y, 1.7f);
                float h2 = tileHash(x, y, 2.9f);
                bool openN = maze_.IsOpen(x, y - 1);
                bool openS = maze_.IsOpen(x, y + 1);
                bool openW = maze_.IsOpen(x - 1, y);
                bool openE = maze_.IsOpen(x + 1, y);
                bool corridorNS = openN && openS && !openW && !openE;
                bool corridorEW = openW && openE && !openN && !openS;

                if (h0 < chairChance && IsRoomLike(t)) {
                    float chairX = c.x + (h1 - 0.5f) * std::max(0.0f, tileW - 1.12f) * 0.42f;
                    float chairZ = c.z + (h2 - 0.5f) * std::max(0.0f, tileD - 1.12f) * 0.42f;
                    float chairYaw = h1 * kPi * 2.0f;
                    addChair({chairX, 0.0f, chairZ}, chairYaw, h2 < 0.55f);
                }

                bool paperHallway = false;
                if (corridorEW) paperHallway = tileHash(x / 4, y, 26.0f) < paperHallwayChance;
                if (corridorNS) paperHallway = paperHallway || tileHash(x, y / 4, 27.0f) < paperHallwayChance;
                if (paperHallway) {
                    int count = std::max(1, static_cast<int>((11.0f + tileHash(x, y, 4.0f) * 18.0f) * hallwayPaperDensity));
                    addLoosePapers(t, count, true);
                } else if (h1 < loosePaperChance) {
                    int count = std::max(1, static_cast<int>((2.0f + tileHash(x, y, 4.0f) * 6.0f) * paperDensity));
                    addLoosePapers(t, count, false);
                }

                int ventSides[4]{};
                int ventSideCount = 0;
                if (!openN) ventSides[ventSideCount++] = 0;
                if (!openS) ventSides[ventSideCount++] = 1;
                if (!openW) ventSides[ventSideCount++] = 2;
                if (!openE) ventSides[ventSideCount++] = 3;
                if (ventSideCount > 0 && tileHash(x, y, 31.0f) < ventChance) {
                    int sideIndex = std::min(ventSideCount - 1, static_cast<int>(tileHash(x, y, 32.0f) * ventSideCount));
                    addWallVent(t, ventSides[sideIndex], tileHash(x, y, 33.0f));
                }

                if (h2 < waterDamageChance) {
                    int primarySide = std::min(3, static_cast<int>(tileHash(x, y, 30.0f) * 4.0f));
                    float floorSeed = tileHash(x, y, 21.0f);
                    float strength = 0.74f + tileHash(x, y, 34.0f) * 0.62f;
                    float ceilingSize = tileHash(x, y, 42.0f);
                    bool compactCeiling = ceilingSize < 0.35f;
                    bool spreadingCeiling = ceilingSize >= 0.58f;
                    float ceilingSeed = tileHash(x, y, 29.0f);
                    markWaterBlob(t, false, primarySide, 0, floorSeed, strength * 0.92f);
                    markWaterBlob(t, true, primarySide, compactCeiling ? 3 : 0, ceilingSeed, compactCeiling ? strength * 0.32f : strength);
                    if (tileHash(x, y, 35.0f) < 0.42f) {
                        addWaterHorizontalSpill(t, primarySide, false, tileHash(x, y, 35.0f), strength * 0.58f);
                    }
                    if (spreadingCeiling) {
                        addWaterHorizontalSpill(t, primarySide, true, tileHash(x, y, 36.0f), strength * (0.58f + tileHash(x, y, 37.0f) * 0.24f));
                    }
                    if (tileHash(x, y, 38.0f) < 0.36f) {
                        int secondarySide = (primarySide + 1 + std::min(2, static_cast<int>(tileHash(x, y, 39.0f) * 3.0f))) & 3;
                        bool secondaryCeiling = spreadingCeiling && tileHash(x, y, 40.0f) > 0.42f;
                        addWaterHorizontalSpill(t, secondarySide, secondaryCeiling, tileHash(x, y, 41.0f), strength * 0.72f);
                    }
                    int waterWallSides[4]{};
                    int waterWallSideCount = 0;
                    if (!openN) waterWallSides[waterWallSideCount++] = 0;
                    if (!openS) waterWallSides[waterWallSideCount++] = 1;
                    if (!openW) waterWallSides[waterWallSideCount++] = 2;
                    if (!openE) waterWallSides[waterWallSideCount++] = 3;
                    if (waterWallSideCount > 0) {
                        int wallSide = waterWallSides[std::min(waterWallSideCount - 1,
                            static_cast<int>(tileHash(x, y, 43.0f) * static_cast<float>(waterWallSideCount)))];
                        bool ceilingRun = spreadingCeiling || tileHash(x, y, 44.0f) < 0.58f;
                        addWaterHorizontalSpill(t, wallSide, ceilingRun, tileHash(x, y, 45.0f), strength * 0.94f);
                        if (!ceilingRun) {
                            addWaterHorizontalSpill(t, wallSide, false, tileHash(x, y, 47.0f), strength * 0.70f);
                        }
                    }
                }
            }
        }

        if (false && gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            bool showFloorWater = gDebugSliceEffect == DebugSliceEffect::FloorWater ||
                gDebugSliceEffect == DebugSliceEffect::WallWater;
            bool showCeilingWater = gDebugSliceEffect == DebugSliceEffect::CeilingWater ||
                gDebugSliceEffect == DebugSliceEffect::WallWater;
            int debugCenterX = 1 + gDebugSliceTiles / 2;
            int debugCenterY = 1 + gDebugSliceTiles / 2;
            for (int y = 1; y < maze_.h - 1; ++y) {
                for (int x = 1; x < maze_.w - 1; ++x) {
                    if (!maze_.IsOpen(x, y)) continue;
                    Tile t{x, y};
                    bool openN = maze_.IsOpen(x, y - 1);
                    bool openS = maze_.IsOpen(x, y + 1);
                    bool openW = maze_.IsOpen(x - 1, y);
                    bool openE = maze_.IsOpen(x + 1, y);
                    int wallSide = !openN ? 0 : (!openW ? 2 : (!openE ? 3 : (!openS ? 1 : 0)));
                    float seed = tileHash(x, y, 140.0f);

                    if (gDebugSliceEffect == DebugSliceEffect::WallWater) {
                        if (!openN) {
                            addWaterHorizontalSpill(t, 0, true, seed + 0.01f, 1.26f);
                        }
                        if (!openW) {
                            addWaterHorizontalSpill(t, 2, true, seed + 0.03f, 1.18f);
                        }
                        if (!openE) {
                            addWaterHorizontalSpill(t, 3, true, seed + 0.05f, 1.14f);
                        }
                        continue;
                    }

                    if (x != debugCenterX || y != debugCenterY) continue;
                    if (showFloorWater) {
                        markWaterBlob(t, false, wallSide, 0, seed + 0.11f, 1.24f);
                    }
                    if (showCeilingWater) {
                        int mode = gDebugSliceTiles <= 1 ? 3 : 1;
                        markWaterBlob(t, true, wallSide, mode, seed + 0.21f, 1.28f);
                    }
                }
            }
        }

        auto addCeilingWaterBorderRunoff = [&](Tile t, int side, const WaterTileSurface& surface, float salt) {
            if (!surface.active || surface.mode == 3 || !wallHasWaterSurface(t, side)) return;
            XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
            float span = (side == 0 || side == 1) ? tileW : tileD;
            float seed = surface.seed + salt + static_cast<float>(side) * 0.071f;
            float h0 = LampHash(seed * 17.0f + c.x, c.z + static_cast<float>(side) * 3.1f);
            float h1 = LampHash(seed * 23.0f - c.z, c.x + static_cast<float>(side) * 5.7f);
            float h2 = LampHash(seed * 31.0f + c.x * 0.5f, c.z * 0.5f);
            float lateral = (h2 - 0.5f) * span * 0.48f;
            float wallW = span * (0.30f + h1 * 0.38f);
            float wallHgt = wallH * (0.52f + h0 * 0.46f);
            float yCenter = wallH - wallHgt * 0.5f - 0.0015f;
            addWaterWallCard(t, side, lateral, yCenter, wallW, wallHgt, true, seed + 0.47f);
        };

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (!maze_.IsOpen(x, y)) continue;
                const WaterTileSurface& ceilingSurface = ceilingWaterTiles[waterTileIndex(t)];
                if (!ceilingSurface.active || ceilingSurface.mode == 3) continue;

                if (ceilingSurface.mode >= 1) {
                    addCeilingWaterBorderRunoff(t, ceilingSurface.side, ceilingSurface, 0.0f);
                    continue;
                }

                for (int side = 0; side < 4; ++side) {
                    if (!wallHasWaterSurface(t, side)) continue;
                    float edgeBias = 0.18f + std::clamp(ceilingSurface.score - 1.0f, 0.0f, 0.45f);
                    if (!gEffectDebugViewer &&
                        LampHash(ceilingSurface.seed * 53.0f + static_cast<float>(side) * 11.0f,
                            static_cast<float>(x * 17 + y * 31)) > edgeBias) {
                        continue;
                    }
                    addCeilingWaterBorderRunoff(t, side, ceilingSurface, 0.19f + static_cast<float>(side) * 0.043f);
                }
            }
        }

        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (!maze_.IsOpen(x, y)) continue;
                size_t idx = waterTileIndex(t);
                const WaterTileSurface& floorSurface = floorWaterTiles[idx];
                const WaterTileSurface& ceilingSurface = ceilingWaterTiles[idx];
                XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
                if (floorSurface.active) {
                    XMFLOAT3 source{center.x, 0.075f, center.z};
                    addWaterAttentionPoint(source, source, {0.0f, 1.0f, 0.0f},
                        maze_.TileAverage() * 1.12f, floorSurface.seed + 0.13f, false);
                }
                if (ceilingSurface.active) {
                    XMFLOAT3 source{center.x, wallH - 0.055f, center.z};
                    addWaterAttentionPoint(source, source, {0.0f, -1.0f, 0.0f},
                        maze_.TileAverage() * 1.18f, ceilingSurface.seed + 0.31f, false);
                }
                emitWaterTileCard(t, false, floorWaterTiles[idx]);
                emitWaterTileCard(t, true, ceilingWaterTiles[idx]);
            }
        }

        std::vector<Tile> openTiles;
        openTiles.reserve(static_cast<size_t>(maze_.w * maze_.h));
        for (int y = 1; y < maze_.h - 1; ++y) {
            for (int x = 1; x < maze_.w - 1; ++x) {
                Tile t{x, y};
                if (maze_.IsOpen(x, y) && (gEffectDebugViewer || (!(t == maze_.start) && !(t == maze_.exit)))) {
                    openTiles.push_back(t);
                }
            }
        }

        if (!openTiles.empty()) {
            uint32_t scatterSeed = runtimeSeed_ ^ 0x61c88647u;

            float cabinetDensity = std::clamp(settings_.metalCabinetDensity, 0.0f, 4.0f);
            int cabinetTarget = cabinetDensity <= 0.001f
                ? 0
                : std::clamp(static_cast<int>(std::round(static_cast<float>(openTiles.size()) * 0.014f * cabinetDensity)), 2, 76);
            int placedCabinets = 0;
            int cabinetAttempts = cabinetTarget * 10;
            for (int cidx = 0; cidx < cabinetAttempts && placedCabinets < cabinetTarget; ++cidx) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(cidx, 971, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                if (t == maze_.start || t == maze_.exit) continue;
                int sides[4]{};
                int sideCount = 0;
                if (!maze_.IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                if (!maze_.IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                if (!maze_.IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                if (!maze_.IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                if (sideCount == 0) continue;
                int side = sides[std::min(sideCount - 1, static_cast<int>(Rand01(cidx, 977, scatterSeed) * static_cast<float>(sideCount)))];
                if (addMetalCabinetAgainstWall(t, side, Rand01(cidx, 983, scatterSeed))) {
                    ++placedCabinets;
                }
            }

            bool emitWaterLiquid = false;
            auto addBloodScare = [&](XMFLOAT3 pos, float span) {
                if (span < 0.56f) return;
                BloodScarePoint p{};
                p.pos = pos;
                p.source = pos.y < 0.35f
                    ? XMFLOAT3{pos.x, 0.18f, pos.z}
                    : XMFLOAT3{pos.x, std::clamp(pos.y, 0.18f, wallH - 0.055f), pos.z};
                p.radius = std::clamp(1.25f + span * 0.72f, 1.75f, 4.35f);
                p.focusDelaySeconds = 0.26f + LampHash(pos.x * 3.1f + span, pos.z * 5.7f) * 0.58f;
                p.waterLiquid = emitWaterLiquid;
                if (emitWaterLiquid) {
                    p.dreadScale = 0.30f;
                }
                bloodScarePoints_.push_back(p);
            };

            std::unordered_map<int, int> bloodCeilingLayers;
            std::unordered_set<int> bloodCenterSeepCovered;
            std::vector<FloorFootprint> bloodCeilingReservations;
            constexpr float kLiquidFloorReservationPad = 0.010f;
            auto liquidMaterial = [&](float rawSeed) {
                return (emitWaterLiquid ? 25.0f : 14.0f) + rawSeed;
            };
            auto bloodTileKey = [&](Tile tile) {
                return tile.y * std::max(1, maze_.w) + tile.x;
            };
            auto bloodCeilingFootprintClear = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.012f) {
                if (!footprintFitsMaze(px, pz, width, depth, yaw, pad)) return false;
                FloorFootprint candidate = makeFootprint(px, pz, width, depth, yaw, pad);
                for (const FloorFootprint& reserved : bloodCeilingReservations) {
                    if (footprintOverlap(candidate, reserved)) return false;
                }
                return true;
            };
            auto reserveBloodCeilingFootprint = [&](float px, float pz, float width, float depth, float yaw, float pad = 0.012f) {
                if (!bloodCeilingFootprintClear(px, pz, width, depth, yaw, pad)) return false;
                bloodCeilingReservations.push_back(makeFootprint(px, pz, width, depth, yaw, pad));
                return true;
            };
            auto nextBloodCeilingY = [&](float px, float pz, float seed) {
                Tile tile = maze_.TileFromWorld(px, pz);
                int key = bloodTileKey(tile);
                int& layer = bloodCeilingLayers[key];
                int usedLayer = std::min(layer, 10);
                ++layer;
                float jitter = (std::fmod(std::abs(seed) * 97.0f, 1.0f) - 0.5f) * kBloodCeilingDecalLayerStep * 0.24f;
                return wallH - kBloodCeilingDecalInset - static_cast<float>(usedLayer) * kBloodCeilingDecalLayerStep + jitter;
            };

            auto addBloodFloor = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift) {
                if (!longFloorFootprintClear(px, pz, w, d, yaw, kLiquidFloorReservationPad)) {
                    w *= 0.62f;
                    d *= 0.62f;
                    if (!floorFootprintClear(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                }
                if (!reserveFloorFootprint(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                AddFloorCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    kBloodFloorDecalLift + layerLift, liquidMaterial(0.02f + std::fmod(seed, 0.93f)));
                addBloodScare({px, 0.10f, pz}, std::max(w, d));
                return true;
            };

            auto addBloodCeiling = [&](float px, float pz, float w, float d, float yaw, float seed) {
                if (!longFloorFootprintClear(px, pz, w, d, yaw)) {
                    w *= 0.58f;
                    d *= 0.58f;
                    if (!floorFootprintClear(px, pz, w, d, yaw)) return false;
                }
                if (!reserveBloodCeilingFootprint(px, pz, w, d, yaw)) return false;
                AddCeilingCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    nextBloodCeilingY(px, pz, seed), liquidMaterial(0.08f + std::fmod(seed, 0.87f)));
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto addCenterSeepFloor = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift) {
                if (!reserveFloorFootprint(px, pz, w, d, yaw, kLiquidFloorReservationPad)) return false;
                AddFloorCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    kBloodFloorDecalLift + layerLift, liquidMaterial(0.43f + std::fmod(seed, 0.50f)));
                addBloodScare({px, 0.10f, pz}, std::max(w, d));
                return true;
            };

            auto addCenterSeepCeiling = [&](float px, float pz, float w, float d, float yaw, float seed) {
                if (!reserveBloodCeilingFootprint(px, pz, w, d, yaw, 0.010f)) return false;
                AddCeilingCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    nextBloodCeilingY(px, pz, seed + 0.29f), liquidMaterial(0.43f + std::fmod(seed, 0.50f)));
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto addBloodCeilingFloorPair = [&](float px, float pz, float w, float d, float yaw, float seed, float floorScale = 0.92f) {
                float floorW = w * floorScale;
                float floorD = d * floorScale;
                if (!floorFootprintClear(px, pz, floorW, floorD, yaw, kLiquidFloorReservationPad)) return false;
                if (!bloodCeilingFootprintClear(px, pz, w, d, yaw, 0.010f)) return false;
                floorReservations.push_back(makeFootprint(px, pz, floorW, floorD, yaw, kLiquidFloorReservationPad));
                bloodCeilingReservations.push_back(makeFootprint(px, pz, w, d, yaw, 0.010f));
                float material = liquidMaterial(0.43f + std::fmod(seed, 0.50f));
                AddFloorCard(vertices, transparentIndices, {px, 0.0f, pz}, floorW, floorD, yaw,
                    kBloodFloorDecalLift, material);
                AddCeilingCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    nextBloodCeilingY(px, pz, seed + 0.29f), material);
                addBloodScare({px, 0.10f, pz}, std::max(floorW, floorD));
                addBloodScare({px, wallH - 0.08f, pz}, std::max(w, d));
                return true;
            };

            auto wallHasSurface = [&](Tile tile, int side) {
                if (!maze_.IsOpen(tile.x, tile.y)) return false;
                if (side == 0) return !maze_.IsOpen(tile.x, tile.y - 1);
                if (side == 1) return !maze_.IsOpen(tile.x, tile.y + 1);
                if (side == 2) return !maze_.IsOpen(tile.x - 1, tile.y);
                return !maze_.IsOpen(tile.x + 1, tile.y);
            };

            auto wallSupportSpan = [&](Tile t, int side, float& minAlong, float& maxAlong) {
                if (!wallHasSurface(t, side)) return false;
                if (side == 0 || side == 1) {
                    int x0 = t.x;
                    int x1 = t.x;
                    while (wallHasSurface({x0 - 1, t.y}, side)) --x0;
                    while (wallHasSurface({x1 + 1, t.y}, side)) ++x1;
                    minAlong = ox + static_cast<float>(x0) * tileW;
                    maxAlong = ox + static_cast<float>(x1 + 1) * tileW;
                } else {
                    int y0 = t.y;
                    int y1 = t.y;
                    while (wallHasSurface({t.x, y0 - 1}, side)) --y0;
                    while (wallHasSurface({t.x, y1 + 1}, side)) ++y1;
                    minAlong = oz + static_cast<float>(y0) * tileD;
                    maxAlong = oz + static_cast<float>(y1 + 1) * tileD;
                }
                return maxAlong - minAlong > 0.20f;
            };

            auto addBloodWall = [&](Tile t, int side, float lateral, float yCenter, float w, float h, float seed) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                XMFLOAT3 center{c.x, yCenter, c.z};
                float minAlong = 0.0f;
                float maxAlong = 0.0f;
                if (!wallSupportSpan(t, side, minAlong, maxAlong)) return false;
                constexpr float kBloodWallDecalInset = 0.0050f;
                constexpr float wallDecalMargin = 0.13f;
                minAlong += wallDecalMargin;
                maxAlong -= wallDecalMargin;
                float available = maxAlong - minAlong;
                if (available < 0.28f) return false;
                w = std::min(w, available);
                float halfW = w * 0.5f;
                float desiredAlong = (side == 0 || side == 1) ? c.x + lateral : c.z + lateral;
                float clampedAlong = std::clamp(desiredAlong, minAlong + halfW, maxAlong - halfW);
                if (side == 0) {
                    normal = {0.0f, 0.0f, 1.0f};
                    right = {1.0f, 0.0f, 0.0f};
                    center = {clampedAlong, yCenter, c.z - tileD * 0.5f + kBloodWallDecalInset};
                } else if (side == 1) {
                    normal = {0.0f, 0.0f, -1.0f};
                    right = {-1.0f, 0.0f, 0.0f};
                    center = {clampedAlong, yCenter, c.z + tileD * 0.5f - kBloodWallDecalInset};
                } else if (side == 2) {
                    normal = {1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, 1.0f};
                    center = {c.x - tileW * 0.5f + kBloodWallDecalInset, yCenter, clampedAlong};
                } else {
                    normal = {-1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, -1.0f};
                    center = {c.x + tileW * 0.5f - kBloodWallDecalInset, yCenter, clampedAlong};
                }
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                constexpr float wallBloodFloorMargin = 0.002f;
                constexpr float wallBloodCeilingMargin = 0.004f;
                h = wallH - wallBloodFloorMargin - wallBloodCeilingMargin;
                center.y = wallBloodFloorMargin + h * 0.5f;
                XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(up,  h * 0.5f)));
                XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(up,  h * 0.5f)));
                AddQuadUV(vertices, transparentIndices, a, b, c0, d0, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, liquidMaterial(0.11f + std::fmod(seed, 0.83f)));
                float bottomY = center.y - h * 0.5f;
                if (bottomY > wallBloodFloorMargin + 0.045f) {
                    int dripStrips = 1 + static_cast<int>(LampHash(seed * 41.0f + center.x, center.z - seed * 13.0f) * 3.0f);
                    for (int strip = 0; strip < dripStrips; ++strip) {
                        float r0 = LampHash(seed * 53.0f + static_cast<float>(strip) * 7.1f, center.x + center.z);
                        float r1 = LampHash(seed * 67.0f + static_cast<float>(strip) * 11.3f, center.z - center.x);
                        float stripW = std::min(w * (0.10f + r0 * 0.18f), 0.34f);
                        float offset = (r1 - 0.5f) * std::max(0.0f, w - stripW) * 0.62f;
                        float bridgeTop = std::min(bottomY + 0.12f + r0 * 0.10f, wallH - wallBloodCeilingMargin);
                        float bridgeBottom = wallBloodFloorMargin;
                        float bridgeH = bridgeTop - bridgeBottom;
                        if (bridgeH <= 0.035f) continue;
                        XMFLOAT3 bridgeCenter = Add3(center, Add3(Scale3(right, offset), {0.0f, (bridgeTop + bridgeBottom) * 0.5f - center.y, 0.0f}));
                        XMFLOAT3 ba = Add3(bridgeCenter, Add3(Scale3(right, -stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                        XMFLOAT3 bb = Add3(bridgeCenter, Add3(Scale3(right,  stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                        XMFLOAT3 bc = Add3(bridgeCenter, Add3(Scale3(right,  stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                        XMFLOAT3 bd = Add3(bridgeCenter, Add3(Scale3(right, -stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                        AddQuadUV(vertices, transparentIndices, ba, bb, bc, bd, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0},
                            liquidMaterial(0.37f + std::fmod(seed + r0 * 0.61f + static_cast<float>(strip) * 0.17f, 0.51f)));
                    }
                }
                BloodScarePoint scare{};
                scare.pos = center;
                float sourceY = center.y + h * 0.40f;
                float topY = center.y + h * 0.5f;
                if (topY > wallH * 0.78f) {
                    sourceY = std::max(sourceY, topY - 0.035f);
                }
                scare.source = {center.x, std::clamp(sourceY, 0.18f, wallH - 0.055f), center.z};
                scare.normal = normal;
                scare.radius = std::clamp(1.25f + std::max(w, h) * 0.72f, 1.75f, 4.35f);
                scare.focusDelaySeconds = 0.30f + LampHash(seed * 43.0f + center.x, center.z) * 0.64f;
                scare.requireFacing = true;
                scare.waterLiquid = emitWaterLiquid;
                if (emitWaterLiquid) {
                    scare.dreadScale = 0.30f;
                }
                bloodScarePoints_.push_back(scare);
                return true;
            };

            auto neighborForSide = [](Tile t, int side) {
                if (side == 0) return Tile{t.x, t.y - 1};
                if (side == 1) return Tile{t.x, t.y + 1};
                if (side == 2) return Tile{t.x - 1, t.y};
                return Tile{t.x + 1, t.y};
            };

            auto oppositeSide = [](int side) {
                if (side == 0) return 1;
                if (side == 1) return 0;
                if (side == 2) return 3;
                return 2;
            };

            auto sideForwardYaw = [](int side) {
                if (side == 0) return kPi;
                if (side == 1) return 0.0f;
                if (side == 2) return -kPi * 0.5f;
                return kPi * 0.5f;
            };

            auto sideDirection = [](int side) {
                if (side == 0) return XMFLOAT3{0.0f, 0.0f, -1.0f};
                if (side == 1) return XMFLOAT3{0.0f, 0.0f, 1.0f};
                if (side == 2) return XMFLOAT3{-1.0f, 0.0f, 0.0f};
                return XMFLOAT3{1.0f, 0.0f, 0.0f};
            };

            auto addBloodCeilingWallRunoffs = [&](Tile t, float px, float pz, float w, float d, float yaw,
                float seed, int tag, int sourceSide) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float halfX = std::abs(std::cos(yaw)) * w * 0.5f + std::abs(std::sin(yaw)) * d * 0.5f;
                float halfZ = std::abs(std::sin(yaw)) * w * 0.5f + std::abs(std::cos(yaw)) * d * 0.5f;
                float xMin = c.x - tileW * 0.5f;
                float xMax = c.x + tileW * 0.5f;
                float zMin = c.z - tileD * 0.5f;
                float zMax = c.z + tileD * 0.5f;
                bool touches[4] = {
                    pz - halfZ <= zMin + tileD * 0.10f,
                    pz + halfZ >= zMax - tileD * 0.10f,
                    px - halfX <= xMin + tileW * 0.10f,
                    px + halfX >= xMax - tileW * 0.10f
                };

                for (int side = 0; side < 4; ++side) {
                    if (!touches[side] || side == sourceSide || !wallHasSurface(t, side)) continue;
                    float r0 = Rand01(tag * 23 + side, 1031, scatterSeed);
                    float lateral = (side == 0 || side == 1)
                        ? px - c.x + (Rand01(tag * 23 + side, 1049, scatterSeed) - 0.5f) * tileW * 0.12f
                        : pz - c.z + (Rand01(tag * 23 + side, 1051, scatterSeed) - 0.5f) * tileD * 0.12f;
                    float wallSpan = side == 0 || side == 1 ? tileW : tileD;
                    float contactW = std::clamp((side == 0 || side == 1 ? halfX : halfZ) * (0.95f + r0 * 0.42f),
                        0.42f, wallSpan * 0.96f);
                    float runH = wallH * (0.74f + Rand01(tag * 23 + side, 1057, scatterSeed) * 0.25f);
                    float yCenter = wallH - 0.060f - runH * 0.5f;
                    addBloodWall(t, side, lateral, yCenter, contactW, runH,
                        std::fmod(seed + 0.31f + static_cast<float>(side) * 0.111f, 0.83f));
                }
            };

            auto addFloorNeighborSpill = [&](Tile t, float sourceX, float sourceZ, float seed, int tag, float strength) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                for (int side = 0; side < 4; ++side) {
                    Tile n = neighborForSide(t, side);
                    if (!maze_.IsOpen(n.x, n.y)) continue;
                    float sideBias = 0.0f;
                    if (side == 0) sideBias = (c.z - sourceZ) / std::max(0.001f, tileD * 0.5f);
                    else if (side == 1) sideBias = (sourceZ - c.z) / std::max(0.001f, tileD * 0.5f);
                    else if (side == 2) sideBias = (c.x - sourceX) / std::max(0.001f, tileW * 0.5f);
                    else sideBias = (sourceX - c.x) / std::max(0.001f, tileW * 0.5f);
                    float chance = std::clamp(0.40f + strength * 0.34f + std::max(0.0f, sideBias) * 0.16f, 0.0f, 0.90f);
                    float r0 = Rand01(tag * 19 + side, 1009, scatterSeed);
                    if (r0 > chance) continue;
                    XMFLOAT3 dir = sideDirection(side);
                    float axis = side == 0 || side == 1 ? tileD : tileW;
                    float cross = side == 0 || side == 1 ? tileW : tileD;
                    float originalOverlap = axis * (0.16f + Rand01(tag * 19 + side, 1011, scatterSeed) * 0.08f);
                    float neighborReach = axis * (0.20f + Rand01(tag * 19 + side, 1013, scatterSeed) * 0.26f) *
                        std::clamp(strength, 0.70f, 1.12f);
                    float length = originalOverlap + neighborReach;
                    float width = cross * (0.24f + Rand01(tag * 19 + side, 1019, scatterSeed) * 0.28f);
                    float halfAxis = axis * 0.5f;
                    float centerOffset = halfAxis + (neighborReach - originalOverlap) * 0.5f;
                    XMFLOAT3 lateralAxis = side == 0 || side == 1
                        ? XMFLOAT3{1.0f, 0.0f, 0.0f}
                        : XMFLOAT3{0.0f, 0.0f, 1.0f};
                    float sourceLateral = (side == 0 || side == 1) ? sourceX - c.x : sourceZ - c.z;
                    float lateralLimit = std::max(0.0f, cross - width) * 0.48f;
                    float lateral = std::clamp(sourceLateral +
                        (Rand01(tag * 19 + side, 1017, scatterSeed) - 0.5f) * lateralLimit * 0.42f,
                        -lateralLimit, lateralLimit);
                    XMFLOAT3 spillCenter = Add3(c, Add3(Scale3(dir, centerOffset), Scale3(lateralAxis, lateral)));
                    float yaw = sideForwardYaw(side) + (Rand01(tag * 19 + side, 1021, scatterSeed) - 0.5f) * 0.18f;
                    addBloodFloor(spillCenter.x, spillCenter.z, width, length, yaw,
                        std::fmod(seed + 0.17f + static_cast<float>(side) * 0.071f, 0.93f),
                        static_cast<float>(side + 1) * kBloodFloorDecalLayerStep * 5.0f);
                }
            };

            auto addWaterFloorBorderContinuation = [&](Tile t, int side, const XMFLOAT3& wallCenter, const XMFLOAT3& right,
                const XMFLOAT3& inward, float width, float sourceDepth, float material, float seed, int tag) {
                if (!emitWaterLiquid) return false;
                Tile n = neighborForSide(t, oppositeSide(side));
                if (!maze_.IsOpen(n.x, n.y)) return false;
                float axisLength = (side == 0 || side == 1) ? tileD : tileW;
                float crossLength = (side == 0 || side == 1) ? tileW : tileD;
                float continuationDepth = axisLength * (0.54f + Rand01(tag, 1511, scatterSeed) * 0.38f);
                float continuationWidth = std::min(crossLength * 0.98f, width * (1.06f + Rand01(tag, 1517, scatterSeed) * 0.20f));
                float startDistance = std::max(0.04f, axisLength - 0.020f);
                XMFLOAT3 nearCenter = Add3({wallCenter.x, kBloodFloorDecalLift + 0.0015f, wallCenter.z},
                    Scale3(inward, startDistance));
                nearCenter = Add3(nearCenter, Scale3(right, (Rand01(tag, 1523, scatterSeed) - 0.5f) * width * 0.10f));
                XMFLOAT3 farCenter = Add3(nearCenter, Scale3(inward, continuationDepth));
                if (!footprintFitsMaze((nearCenter.x + farCenter.x) * 0.5f, (nearCenter.z + farCenter.z) * 0.5f,
                        continuationWidth, continuationDepth, sideForwardYaw(side), 0.010f)) {
                    return false;
                }
                XMFLOAT3 nearLeft = Add3(nearCenter, Scale3(right, -continuationWidth * 0.5f));
                XMFLOAT3 nearRight = Add3(nearCenter, Scale3(right, continuationWidth * 0.5f));
                XMFLOAT3 farRight = Add3(nearRight, Scale3(inward, continuationDepth));
                XMFLOAT3 farLeft = Add3(nearLeft, Scale3(inward, continuationDepth));
                float uvFar = -continuationDepth / std::max(0.08f, sourceDepth);
                AddQuadUV(vertices, transparentIndices,
                    farLeft, farRight, nearRight, nearLeft,
                    {0.0f, 1.0f, 0.0f}, right,
                    {0.0f, uvFar}, {1.0f, uvFar}, {1.0f, 0.0f}, {0.0f, 0.0f}, material);
                return true;
            };

            auto addCeilingPropagation = [&](Tile t, float px, float pz, float w, float d, float yaw, float seed, int tag, int sourceSide) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float halfX = std::abs(std::cos(yaw)) * w * 0.5f + std::abs(std::sin(yaw)) * d * 0.5f;
                float halfZ = std::abs(std::sin(yaw)) * w * 0.5f + std::abs(std::cos(yaw)) * d * 0.5f;
                float xMin = c.x - tileW * 0.5f;
                float xMax = c.x + tileW * 0.5f;
                float zMin = c.z - tileD * 0.5f;
                float zMax = c.z + tileD * 0.5f;
                bool touches[4] = {
                    pz - halfZ <= zMin + tileD * 0.10f,
                    pz + halfZ >= zMax - tileD * 0.10f,
                    px - halfX <= xMin + tileW * 0.10f,
                    px + halfX >= xMax - tileW * 0.10f
                };
                addBloodCeilingWallRunoffs(t, px, pz, w, d, yaw, seed, tag, sourceSide);

                for (int side = 0; side < 4; ++side) {
                    if (!touches[side]) continue;
                    float r0 = Rand01(tag * 23 + side, 1031, scatterSeed);
                    Tile n = neighborForSide(t, side);
                    if (maze_.IsOpen(n.x, n.y)) {
                        bool fromWallLeak = sourceSide >= 0;
                        if (fromWallLeak && side != oppositeSide(sourceSide)) continue;
                        if (!fromWallLeak && r0 > 0.62f) continue;
                        if (fromWallLeak && !gBloodDebugEveryWall && !settings_.bloodStudyView && r0 > 0.86f) continue;
                        XMFLOAT3 dir = sideDirection(side);
                        float axis = side == 0 || side == 1 ? tileD : tileW;
                        float span = side == 0 || side == 1 ? tileW : tileD;
                        float length = fromWallLeak
                            ? axis * (0.30f + Rand01(tag * 23 + side, 1037, scatterSeed) * 0.24f)
                            : axis * (0.66f + Rand01(tag * 23 + side, 1037, scatterSeed) * 0.36f);
                        float width = std::clamp((side == 0 || side == 1 ? halfX : halfZ) * (1.08f + r0 * 0.28f),
                            span * 0.34f, span * 0.96f);
                        float centerX = c.x + dir.x * (axis * 0.5f + length * 0.24f);
                        float centerZ = c.z + dir.z * (axis * 0.5f + length * 0.24f);
                        float spreadYaw = sideForwardYaw(side) + (Rand01(tag * 23 + side, 1041, scatterSeed) - 0.5f) * 0.26f;
                        float spreadSeed = std::fmod(seed + 0.23f + static_cast<float>(side) * 0.093f, 0.87f);
                        if (addBloodCeilingFloorPair(centerX, centerZ, width, length, spreadYaw, spreadSeed, 0.84f)) {
                            addBloodCeilingWallRunoffs(n, centerX, centerZ, width, length, spreadYaw,
                                spreadSeed, tag * 31 + side + 17, oppositeSide(side));
                        }
                    }
                }
            };

            auto addBloodRun = [&](Tile t, int runIndex) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                bool openE = maze_.IsOpen(t.x + 1, t.y);
                bool openW = maze_.IsOpen(t.x - 1, t.y);
                bool openN = maze_.IsOpen(t.x, t.y - 1);
                bool openS = maze_.IsOpen(t.x, t.y + 1);
                bool ew = openE || openW;
                bool ns = openN || openS;
                float yaw = Rand01(runIndex, 761, scatterSeed) * kPi * 2.0f;
                if (ew || ns) {
                    bool useEW = ew && (!ns || Rand01(runIndex, 763, scatterSeed) < 0.55f);
                    yaw = useEW ? 0.0f : kPi * 0.5f;
                    yaw += (Rand01(runIndex, 769, scatterSeed) - 0.5f) * 0.22f;
                }

                XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
                float runLength = tileAvg * (1.85f + Rand01(runIndex, 773, scatterSeed) * 4.65f);
                float runWidth = 0.44f + Rand01(runIndex, 787, scatterSeed) * 0.82f;
                float alongOffset = (Rand01(runIndex, 797, scatterSeed) - 0.5f) * tileAvg * 0.55f;
                float crossOffset = (Rand01(runIndex, 809, scatterSeed) - 0.5f) * tileAvg * 0.42f;
                XMFLOAT3 center = Add3(c, OrientedOffset(right, up, forward, alongOffset, 0.0f, crossOffset));
                float seed = Rand01(runIndex, 811, scatterSeed);

                bool placed = addBloodFloor(center.x, center.z, runLength, runWidth, yaw, seed, 0.0f);
                if (!placed) {
                    runLength *= 0.68f;
                    runWidth *= 0.82f;
                    placed = addBloodFloor(center.x, center.z, runLength, runWidth, yaw, seed, 0.0f);
                }
                if (!placed) return false;
                addFloorNeighborSpill(t, center.x, center.z, seed, runIndex, std::clamp(runLength / std::max(0.01f, tileAvg) * 0.36f, 0.65f, 1.22f));

                if (Rand01(runIndex, 821, scatterSeed) < 0.64f) {
                    XMFLOAT3 ceilingCenter = Add3(center, OrientedOffset(right, up, forward,
                        (Rand01(runIndex, 823, scatterSeed) - 0.5f) * runLength * 0.16f,
                        0.0f,
                        (Rand01(runIndex, 827, scatterSeed) - 0.5f) * runWidth * 0.55f));
                    float ceilingW = runLength * (0.68f + Rand01(runIndex, 829, scatterSeed) * 0.42f);
                    float ceilingD = runWidth * (0.76f + Rand01(runIndex, 839, scatterSeed) * 0.58f);
                    float ceilingYaw = yaw + (Rand01(runIndex, 853, scatterSeed) - 0.5f) * 0.38f;
                    float ceilingSeed = Rand01(runIndex, 857, scatterSeed);
                    if (addBloodCeilingFloorPair(ceilingCenter.x, ceilingCenter.z, ceilingW, ceilingD, ceilingYaw, ceilingSeed, 0.86f)) {
                        addCeilingPropagation(t, ceilingCenter.x, ceilingCenter.z, ceilingW, ceilingD, ceilingYaw, ceilingSeed, runIndex, -1);
                    }
                }

                int satelliteCount = 18 + static_cast<int>(Rand01(runIndex, 859, scatterSeed) * 36.0f);
                for (int i = 0; i < satelliteCount; ++i) {
                    float along = (Rand01(runIndex * 37 + i, 863, scatterSeed) - 0.5f) * runLength * 1.14f;
                    float cross = (Rand01(runIndex * 37 + i, 877, scatterSeed) - 0.5f) * runWidth * 2.45f;
                    XMFLOAT3 p = Add3(center, OrientedOffset(right, up, forward, along, 0.0f, cross));
                    float sizeBias = std::pow(Rand01(runIndex * 37 + i, 881, scatterSeed), 2.15f);
                    float spotW = 0.045f + sizeBias * 0.25f;
                    float spotD = 0.035f + std::pow(Rand01(runIndex * 37 + i, 883, scatterSeed), 2.05f) * 0.22f;
                    float spotYaw = yaw + (Rand01(runIndex * 37 + i, 887, scatterSeed) - 0.5f) * 1.25f;
                    addBloodFloor(p.x, p.z, spotW, spotD, spotYaw, Rand01(runIndex * 37 + i, 907, scatterSeed),
                        static_cast<float>(i) * kBloodFloorDecalLayerStep);
                }

                int sideCandidates[4]{};
                int sideCount = 0;
                bool mostlyEW = std::abs(right.x) >= std::abs(right.z);
                if (mostlyEW) {
                    if (!maze_.IsOpen(t.x, t.y - 1)) sideCandidates[sideCount++] = 0;
                    if (!maze_.IsOpen(t.x, t.y + 1)) sideCandidates[sideCount++] = 1;
                } else {
                    if (!maze_.IsOpen(t.x - 1, t.y)) sideCandidates[sideCount++] = 2;
                    if (!maze_.IsOpen(t.x + 1, t.y)) sideCandidates[sideCount++] = 3;
                }
                if (sideCount == 0) {
                    if (!maze_.IsOpen(t.x, t.y - 1)) sideCandidates[sideCount++] = 0;
                    if (!maze_.IsOpen(t.x, t.y + 1)) sideCandidates[sideCount++] = 1;
                    if (!maze_.IsOpen(t.x - 1, t.y)) sideCandidates[sideCount++] = 2;
                    if (!maze_.IsOpen(t.x + 1, t.y)) sideCandidates[sideCount++] = 3;
                }
                for (int sidx = 0; sidx < sideCount; ++sidx) {
                    if (Rand01(runIndex * 13 + sidx, 911, scatterSeed) < (sidx == 0 ? 0.92f : 0.52f)) {
                        int side = sideCandidates[sidx];
                        float lateral = (side == 0 || side == 1)
                            ? center.x - c.x + (Rand01(runIndex * 13 + sidx, 919, scatterSeed) - 0.5f) * tileW * 0.35f
                            : center.z - c.z + (Rand01(runIndex * 13 + sidx, 929, scatterSeed) - 0.5f) * tileD * 0.35f;
                        float wallLen = runLength * (0.48f + Rand01(runIndex * 13 + sidx, 937, scatterSeed) * 0.48f);
                        float wallH = 0.68f + Rand01(runIndex * 13 + sidx, 941, scatterSeed) * 1.74f;
                        float wallY = 0.62f + Rand01(runIndex * 13 + sidx, 947, scatterSeed) * 1.46f;
                        addBloodWall(t, side, lateral, wallY, wallLen, wallH, Rand01(runIndex * 13 + sidx, 953, scatterSeed));
                    }
                }
                return true;
            };

            auto addBloodBurst = [&](Tile t, int burstIndex) {
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float r0 = Rand01(burstIndex, 601, scatterSeed);
                float r1 = Rand01(burstIndex, 607, scatterSeed);
                float r2 = Rand01(burstIndex, 613, scatterSeed);
                float yaw = r2 * kPi * 2.0f;
                float px = c.x + (r0 - 0.5f) * tileW * 0.48f;
                float pz = c.z + (r1 - 0.5f) * tileD * 0.48f;
                float base = 0.62f + Rand01(burstIndex, 617, scatterSeed) * 0.98f;
                addBloodFloor(px, pz, base * (0.96f + Rand01(burstIndex, 619, scatterSeed) * 0.74f), base * (0.58f + Rand01(burstIndex, 623, scatterSeed) * 0.70f), yaw, r0, 0.0f);
                addFloorNeighborSpill(t, px, pz, r0, burstIndex, std::clamp(base / std::max(0.01f, tileAvg), 0.48f, 1.08f));
                if (Rand01(burstIndex, 631, scatterSeed) < 0.78f) {
                    float ceilingX = px + (Rand01(burstIndex, 641, scatterSeed) - 0.5f) * 0.45f;
                    float ceilingZ = pz + (Rand01(burstIndex, 643, scatterSeed) - 0.5f) * 0.45f;
                    float ceilingW = base * (0.78f + Rand01(burstIndex, 647, scatterSeed) * 0.86f);
                    float ceilingD = base * (0.64f + Rand01(burstIndex, 653, scatterSeed) * 0.78f);
                    float ceilingYaw = yaw + Rand01(burstIndex, 659, scatterSeed) * 0.9f;
                    if (addBloodCeilingFloorPair(ceilingX, ceilingZ, ceilingW, ceilingD, ceilingYaw, r1, 0.86f)) {
                        addCeilingPropagation(t, ceilingX, ceilingZ, ceilingW, ceilingD, ceilingYaw, r1, burstIndex, -1);
                    }
                }

                int sides[4]{};
                int sideCount = 0;
                if (!maze_.IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                if (!maze_.IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                if (!maze_.IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                if (!maze_.IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                for (int sidx = 0; sidx < sideCount; ++sidx) {
                    if (Rand01(burstIndex * 7 + sidx, 661, scatterSeed) > 0.68f && sidx > 0) continue;
                    float lateral = (Rand01(burstIndex * 7 + sidx, 673, scatterSeed) - 0.5f) *
                        ((sides[sidx] == 0 || sides[sidx] == 1) ? tileW : tileD) * 0.54f;
                    float yc = 0.88f + Rand01(burstIndex * 7 + sidx, 677, scatterSeed) * 1.26f;
                    float sw = 0.56f + Rand01(burstIndex * 7 + sidx, 683, scatterSeed) * 1.20f;
                    float sh = 0.52f + Rand01(burstIndex * 7 + sidx, 691, scatterSeed) * 1.30f;
                    addBloodWall(t, sides[sidx], lateral, yc, sw, sh, Rand01(burstIndex * 7 + sidx, 701, scatterSeed));
                }

                int droplets = 24 + static_cast<int>(Rand01(burstIndex, 709, scatterSeed) * 42.0f);
                for (int d = 0; d < droplets; ++d) {
                    float dyaw = Rand01(burstIndex * 31 + d, 719, scatterSeed) * kPi * 2.0f;
                    float radius = std::pow(Rand01(burstIndex * 31 + d, 727, scatterSeed), 0.56f) * tileAvg * 1.54f;
                    float dx = std::cos(dyaw) * radius;
                    float dz = std::sin(dyaw) * radius;
                    float tiny = std::pow(Rand01(burstIndex * 31 + d, 733, scatterSeed), 2.45f);
                    float dw = 0.028f + tiny * 0.20f;
                    float dd = 0.024f + std::pow(Rand01(burstIndex * 31 + d, 739, scatterSeed), 2.35f) * 0.18f;
                    addBloodFloor(px + dx, pz + dz, dw, dd, dyaw, Rand01(burstIndex * 31 + d, 743, scatterSeed),
                        static_cast<float>(d) * kBloodFloorDecalLayerStep);
                }
            };

            auto addBloodCenterDripTile = [&](Tile t, int dripIndex) {
                if (!maze_.IsOpen(t.x, t.y) || t == maze_.start || t == maze_.exit) return false;
                if (bloodCenterSeepCovered.find(bloodTileKey(t)) != bloodCenterSeepCovered.end()) return false;
                if (!maze_.IsOpen(t.x, t.y - 1) || !maze_.IsOpen(t.x, t.y + 1) ||
                    !maze_.IsOpen(t.x - 1, t.y) || !maze_.IsOpen(t.x + 1, t.y)) {
                    return false;
                }
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                bool diagNW = maze_.IsOpen(t.x - 1, t.y - 1);
                bool diagNE = maze_.IsOpen(t.x + 1, t.y - 1);
                bool diagSW = maze_.IsOpen(t.x - 1, t.y + 1);
                bool diagSE = maze_.IsOpen(t.x + 1, t.y + 1);
                bool roomCanvas = diagNW && diagNE && diagSW && diagSE;
                bool eastWestCanvas = Rand01(dripIndex, 1303, scatterSeed) < 0.5f;
                float w = tileW * (roomCanvas ? (1.62f + Rand01(dripIndex, 1307, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? (1.92f + Rand01(dripIndex, 1311, scatterSeed) * 0.24f) : 0.92f));
                float d = tileD * (roomCanvas ? (1.62f + Rand01(dripIndex, 1313, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? 0.92f : (1.92f + Rand01(dripIndex, 1317, scatterSeed) * 0.24f)));
                float yaw = 0.0f;
                float px = c.x + (Rand01(dripIndex, 1321, scatterSeed) - 0.5f) * tileW * 0.10f;
                float pz = c.z + (Rand01(dripIndex, 1327, scatterSeed) - 0.5f) * tileD * 0.10f;
                float seed = Rand01(dripIndex, 1331, scatterSeed);
                bool placed = addBloodCeilingFloorPair(px, pz, w, d, yaw, seed, 1.0f);
                if (!placed) {
                    w = tileW * 0.96f;
                    d = tileD * 0.96f;
                    px = c.x;
                    pz = c.z;
                    placed = addBloodCeilingFloorPair(px, pz, w, d, yaw, seed, 1.0f);
                }
                if (placed) {
                    auto markCovered = [&](Tile covered) {
                        if (maze_.IsOpen(covered.x, covered.y)) bloodCenterSeepCovered.insert(bloodTileKey(covered));
                    };
                    markCovered(t);
                    if (w > tileW * 1.15f) {
                        markCovered({t.x - 1, t.y});
                        markCovered({t.x + 1, t.y});
                    }
                    if (d > tileD * 1.15f) {
                        markCovered({t.x, t.y - 1});
                        markCovered({t.x, t.y + 1});
                    }
                    if (w > tileW * 1.15f && d > tileD * 1.15f) {
                        markCovered({t.x - 1, t.y - 1});
                        markCovered({t.x + 1, t.y - 1});
                        markCovered({t.x - 1, t.y + 1});
                        markCovered({t.x + 1, t.y + 1});
                    }
                }
                return placed;
            };

            auto addBloodLeak = [&](Tile t, int side, int leakIndex, bool wallOnly = false) {
                if (!wallHasSurface(t, side)) return false;
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float seed = Rand01(leakIndex, 701, scatterSeed);
                float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
                float leakW = wallSpan * (0.82f + Rand01(leakIndex, 707, scatterSeed) * 0.13f);
                float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
                float lateral = (Rand01(leakIndex, 709, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
                float topY = wallH - 0.0015f;
                float bottomY = 0.0015f;
                float h = std::max(0.2f, topY - bottomY);
                float centerY = (topY + bottomY) * 0.5f;
                XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                XMFLOAT3 inward{0.0f, 0.0f, 1.0f};
                XMFLOAT3 wallCenter{c.x, centerY, c.z};
                constexpr float kBloodLeakWallDecalInset = 0.0050f;
                constexpr float kBloodLeakSeamInset = 0.0010f;
                if (side == 0) {
                    normal = {0.0f, 0.0f, 1.0f};
                    right = {1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, 1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z - tileD * 0.5f + kBloodLeakWallDecalInset};
                } else if (side == 1) {
                    normal = {0.0f, 0.0f, -1.0f};
                    right = {-1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, -1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z + tileD * 0.5f - kBloodLeakWallDecalInset};
                } else if (side == 2) {
                    normal = {1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, 1.0f};
                    inward = {1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x - tileW * 0.5f + kBloodLeakWallDecalInset, centerY, c.z + lateral};
                } else {
                    normal = {-1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, -1.0f};
                    inward = {-1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x + tileW * 0.5f - kBloodLeakWallDecalInset, centerY, c.z + lateral};
                }

                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 a = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 b = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 c0 = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up,  h * 0.5f)));
                XMFLOAT3 d0 = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up,  h * 0.5f)));
                float sourceMat = liquidMaterial(0.965f + seed * 0.025f);
                float wallMat = sourceMat;
                AddQuadUV(vertices, transparentIndices, a, b, c0, d0, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, wallMat);

                float bloodQuality = std::clamp(settings_.bloodShaderQuality, 0.25f, 1.0f);
                bool addSeamCards = !wallOnly || gBloodDebugEveryWall || bloodQuality >= 0.52f ||
                    Rand01(leakIndex, 741, scatterSeed) < bloodQuality * 0.75f;
                auto addCeilingSeamCard = [&](float depth, float material) {
                    float seamY = nextBloodCeilingY(wallCenter.x, wallCenter.z, seed + 0.37f);
                    XMFLOAT3 seamWallEdge = Add3({wallCenter.x, seamY, wallCenter.z},
                        Scale3(inward, -kBloodLeakWallDecalInset));
                    XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(inward, kBloodLeakSeamInset));
                    XMFLOAT3 nearLeft = Add3(seamCenter, Scale3(right, -leakW * 0.5f));
                    XMFLOAT3 nearRight = Add3(seamCenter, Scale3(right, leakW * 0.5f));
                    XMFLOAT3 farRight = Add3(nearRight, Scale3(inward, depth));
                    XMFLOAT3 farLeft = Add3(nearLeft, Scale3(inward, depth));
                    AddQuadUV(vertices, transparentIndices,
                        nearLeft, nearRight, farRight, farLeft,
                        {0.0f, -1.0f, 0.0f}, right,
                        {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
                };

                auto addFloorSeamCard = [&](float depth, float material) {
                    XMFLOAT3 seamWallEdge = Add3({wallCenter.x, kBloodFloorDecalLift, wallCenter.z},
                        Scale3(inward, -kBloodLeakWallDecalInset));
                    XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(inward, kBloodLeakSeamInset));
                    XMFLOAT3 nearLeft = Add3(seamCenter, Scale3(right, -leakW * 0.5f));
                    XMFLOAT3 nearRight = Add3(seamCenter, Scale3(right, leakW * 0.5f));
                    XMFLOAT3 farRight = Add3(nearRight, Scale3(inward, depth));
                    XMFLOAT3 farLeft = Add3(nearLeft, Scale3(inward, depth));
                    AddQuadUV(vertices, transparentIndices,
                        farLeft, farRight, nearRight, nearLeft,
                        {0.0f, 1.0f, 0.0f}, right,
                        {0, 0}, {1, 0}, {1, 1}, {0, 1}, material);
                };

                float axisLength = (side == 0 || side == 1) ? tileD : tileW;
                Tile forwardTile = neighborForSide(t, oppositeSide(side));
                bool canSpreadForward = maze_.IsOpen(forwardTile.x, forwardTile.y);
                float sourceD = axisLength * (0.88f + Rand01(leakIndex, 719, scatterSeed) * 0.10f);
                if (canSpreadForward) {
                    sourceD += axisLength * (0.16f + Rand01(leakIndex, 721, scatterSeed) * 0.22f);
                }
                if (addSeamCards) {
                    addCeilingSeamCard(sourceD, sourceMat);
                    float seamYaw = sideForwardYaw(side);
                    XMFLOAT3 seamWallEdge = Add3({wallCenter.x, wallH - kBloodCeilingDecalInset, wallCenter.z},
                        Scale3(inward, -kBloodLeakWallDecalInset));
                    XMFLOAT3 seamCenter = Add3(seamWallEdge, Scale3(inward, sourceD * 0.5f + kBloodLeakSeamInset));
                    addCeilingPropagation(t, seamCenter.x, seamCenter.z, leakW, sourceD, seamYaw, seed, leakIndex, side);
                }

                float poolD = axisLength * (0.86f + Rand01(leakIndex, 727, scatterSeed) * 0.12f);
                if (canSpreadForward) {
                    poolD += axisLength * (0.18f + Rand01(leakIndex, 729, scatterSeed) * 0.24f);
                }
                float poolW = leakW;
                XMFLOAT3 floorCenter = Add3({wallCenter.x, 0.0f, wallCenter.z}, Scale3(inward, poolD * 0.5f + 0.006f));
                if (addSeamCards) {
                    addFloorSeamCard(poolD, sourceMat);
                    if (emitWaterLiquid && canSpreadForward) {
                        addWaterFloorBorderContinuation(t, side, wallCenter, right, inward, poolW, poolD,
                            sourceMat, seed, leakIndex * 17 + side);
                    }
                }
                if (wallOnly || gBloodDebugEveryWall) return true;

                BloodScarePoint scare{};
                scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
                scare.source = {wallCenter.x, wallH - 0.035f, wallCenter.z};
                scare.normal = normal;
                scare.radius = std::clamp(1.95f + std::max(poolW, poolD) * 0.96f, 2.65f, 5.65f);
                scare.focusDelaySeconds = 0.34f + Rand01(leakIndex, 733, scatterSeed) * 0.66f;
                scare.requireFacing = true;
                scare.waterLiquid = emitWaterLiquid;
                if (emitWaterLiquid) {
                    scare.dreadScale = 0.30f;
                }
                bloodScarePoints_.push_back(scare);
                return true;
            };

            auto emitWaterDamageUsingBloodSystem = [&]() {
                bool waterDebug = gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect);
                if (!waterDebug && waterLikeDamageChance <= 0.0001f) return;
                emitWaterLiquid = true;
                if (waterDebug) {
                    int debugIndex = 0;
                    for (Tile t : openTiles) {
                        int sideCount = 0;
                        for (int side = 0; side < 4; ++side) {
                            if (wallHasSurface(t, side)) {
                                ++sideCount;
                                addBloodLeak(t, side, 30000 + debugIndex++);
                            }
                        }
                        if (sideCount == 0) {
                            addBloodCenterDripTile(t, 33000 + debugIndex++);
                        }
                    }
                    emitWaterLiquid = false;
                    return;
                }

                int waterIndex = 0;
                for (Tile t : openTiles) {
                    if (t == maze_.start || t == maze_.exit) continue;
                    int sides[4]{};
                    int sideCount = 0;
                    if (wallHasSurface(t, 0)) sides[sideCount++] = 0;
                    if (wallHasSurface(t, 1)) sides[sideCount++] = 1;
                    if (wallHasSurface(t, 2)) sides[sideCount++] = 2;
                    if (wallHasSurface(t, 3)) sides[sideCount++] = 3;
                    float chance = waterLikeDamageChance;
                    if (sideCount == 0) chance *= 0.58f;
                    if (Rand01(waterIndex, 2501, scatterSeed) > chance) {
                        ++waterIndex;
                        continue;
                    }
                    if (sideCount == 0) {
                        addBloodCenterDripTile(t, 25000 + waterIndex);
                    } else {
                        int side = sides[std::min(sideCount - 1,
                            static_cast<int>(Rand01(waterIndex, 2507, scatterSeed) * static_cast<float>(sideCount)))];
                        addBloodLeak(t, side, 25000 + waterIndex);
                    }
                    ++waterIndex;
                }
                emitWaterLiquid = false;
            };

            emitWaterDamageUsingBloodSystem();

            auto waterLikeMaterial = [](float seed, float rawSeed) {
                return 25.0f + rawSeed + std::fmod(std::abs(seed) * 0.0017f, 0.0009f);
            };

            auto addWaterLikeFloor = [&](float px, float pz, float w, float d, float yaw, float seed, float layerLift, float rawSeed) {
                if (!footprintFitsMaze(px, pz, w, d, yaw, 0.010f)) return false;
                AddFloorCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    kBloodFloorDecalLift + 0.010f + layerLift, waterLikeMaterial(seed, rawSeed));
                return true;
            };

            auto addWaterLikeCeiling = [&](float px, float pz, float w, float d, float yaw, float seed, float rawSeed) {
                if (!reserveBloodCeilingFootprint(px, pz, w, d, yaw, 0.010f)) return false;
                AddCeilingCard(vertices, transparentIndices, {px, 0.0f, pz}, w, d, yaw,
                    nextBloodCeilingY(px, pz, seed + 0.61f), waterLikeMaterial(seed, rawSeed));
                return true;
            };

            auto addWaterLikeCenterTile = [&](Tile t, int dripIndex) {
                if (!maze_.IsOpen(t.x, t.y) || (!gEffectDebugViewer && (t == maze_.start || t == maze_.exit))) return false;
                if (bloodCenterSeepCovered.find(bloodTileKey(t)) != bloodCenterSeepCovered.end()) return false;
                if (!maze_.IsOpen(t.x, t.y - 1) || !maze_.IsOpen(t.x, t.y + 1) ||
                    !maze_.IsOpen(t.x - 1, t.y) || !maze_.IsOpen(t.x + 1, t.y)) {
                    return false;
                }
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                bool diagNW = maze_.IsOpen(t.x - 1, t.y - 1);
                bool diagNE = maze_.IsOpen(t.x + 1, t.y - 1);
                bool diagSW = maze_.IsOpen(t.x - 1, t.y + 1);
                bool diagSE = maze_.IsOpen(t.x + 1, t.y + 1);
                bool roomCanvas = diagNW && diagNE && diagSW && diagSE;
                bool eastWestCanvas = Rand01(dripIndex, 2303, scatterSeed) < 0.5f;
                float w = tileW * (roomCanvas ? (1.62f + Rand01(dripIndex, 2307, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? (1.92f + Rand01(dripIndex, 2311, scatterSeed) * 0.24f) : 0.92f));
                float d = tileD * (roomCanvas ? (1.62f + Rand01(dripIndex, 2313, scatterSeed) * 0.24f) :
                    (eastWestCanvas ? 0.92f : (1.92f + Rand01(dripIndex, 2317, scatterSeed) * 0.24f)));
                float px = c.x + (Rand01(dripIndex, 2321, scatterSeed) - 0.5f) * tileW * 0.10f;
                float pz = c.z + (Rand01(dripIndex, 2327, scatterSeed) - 0.5f) * tileD * 0.10f;
                float seed = Rand01(dripIndex, 2331, scatterSeed);
                bool ceilingPlaced = addWaterLikeCeiling(px, pz, w, d, 0.0f, seed, 0.43f + std::fmod(seed, 0.50f));
                bool floorPlaced = addWaterLikeFloor(px, pz, w, d, 0.0f, seed, 0.0f, 0.43f + std::fmod(seed, 0.50f));
                bool placed = ceilingPlaced && floorPlaced;
                if (placed) {
                    auto markCovered = [&](Tile covered) {
                        if (maze_.IsOpen(covered.x, covered.y)) bloodCenterSeepCovered.insert(bloodTileKey(covered));
                    };
                    markCovered(t);
                    if (w > tileW * 1.15f) {
                        markCovered({t.x - 1, t.y});
                        markCovered({t.x + 1, t.y});
                    }
                    if (d > tileD * 1.15f) {
                        markCovered({t.x, t.y - 1});
                        markCovered({t.x, t.y + 1});
                    }
                    if (w > tileW * 1.15f && d > tileD * 1.15f) {
                        markCovered({t.x - 1, t.y - 1});
                        markCovered({t.x + 1, t.y - 1});
                        markCovered({t.x - 1, t.y + 1});
                        markCovered({t.x + 1, t.y + 1});
                    }
                }
                return placed;
            };

            auto addWaterLikeLeak = [&](Tile t, int side, int leakIndex, bool wallOnly = false) {
                if (!wallHasSurface(t, side)) return false;
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float seed = Rand01(leakIndex, 2401, scatterSeed);
                float wallSpan = (side == 0 || side == 1) ? tileW : tileD;
                float leakW = wallSpan * (0.82f + Rand01(leakIndex, 2407, scatterSeed) * 0.13f);
                float lateralLimit = std::max(0.0f, wallSpan * 0.5f - leakW * 0.5f - 0.16f);
                float lateral = (Rand01(leakIndex, 2409, scatterSeed) - 0.5f) * lateralLimit * 2.0f;
                float topY = wallH - 0.0015f;
                float bottomY = 0.0015f;
                float h = std::max(0.2f, topY - bottomY);
                float centerY = (topY + bottomY) * 0.5f;
                XMFLOAT3 normal{0.0f, 0.0f, 1.0f};
                XMFLOAT3 right{1.0f, 0.0f, 0.0f};
                XMFLOAT3 inward{0.0f, 0.0f, 1.0f};
                XMFLOAT3 wallCenter{c.x, centerY, c.z};
                constexpr float kWaterLikeWallDecalInset = 0.0045f;
                constexpr float kWaterLikeSeamInset = 0.0010f;
                if (side == 0) {
                    normal = {0.0f, 0.0f, 1.0f};
                    right = {1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, 1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z - tileD * 0.5f + kWaterLikeWallDecalInset};
                } else if (side == 1) {
                    normal = {0.0f, 0.0f, -1.0f};
                    right = {-1.0f, 0.0f, 0.0f};
                    inward = {0.0f, 0.0f, -1.0f};
                    wallCenter = {c.x + lateral, centerY, c.z + tileD * 0.5f - kWaterLikeWallDecalInset};
                } else if (side == 2) {
                    normal = {1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, 1.0f};
                    inward = {1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x - tileW * 0.5f + kWaterLikeWallDecalInset, centerY, c.z + lateral};
                } else {
                    normal = {-1.0f, 0.0f, 0.0f};
                    right = {0.0f, 0.0f, -1.0f};
                    inward = {-1.0f, 0.0f, 0.0f};
                    wallCenter = {c.x + tileW * 0.5f - kWaterLikeWallDecalInset, centerY, c.z + lateral};
                }

                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 a = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 b = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up, -h * 0.5f)));
                XMFLOAT3 c0 = Add3(wallCenter, Add3(Scale3(right,  leakW * 0.5f), Scale3(up,  h * 0.5f)));
                XMFLOAT3 d0 = Add3(wallCenter, Add3(Scale3(right, -leakW * 0.5f), Scale3(up,  h * 0.5f)));
                float sourceMat = waterLikeMaterial(seed, 0.965f + seed * 0.025f);
                AddQuadUV(vertices, transparentIndices, a, b, c0, d0, normal, right, {0, 1}, {1, 1}, {1, 0}, {0, 0}, sourceMat);

                float axisLength = (side == 0 || side == 1) ? tileD : tileW;
                Tile forwardTile = neighborForSide(t, oppositeSide(side));
                bool canSpreadForward = maze_.IsOpen(forwardTile.x, forwardTile.y);
                float sourceD = axisLength * (0.88f + Rand01(leakIndex, 2419, scatterSeed) * 0.10f);
                if (canSpreadForward) sourceD += axisLength * (0.16f + Rand01(leakIndex, 2421, scatterSeed) * 0.22f);
                float seamYaw = sideForwardYaw(side);
                XMFLOAT3 ceilingEdge = Add3({wallCenter.x, wallH - kBloodCeilingDecalInset, wallCenter.z},
                    Scale3(inward, -kWaterLikeWallDecalInset));
                XMFLOAT3 ceilingCenter = Add3(ceilingEdge, Scale3(inward, sourceD * 0.5f + kWaterLikeSeamInset));
                addWaterLikeCeiling(ceilingCenter.x, ceilingCenter.z, leakW, sourceD, seamYaw, seed, 0.965f + seed * 0.025f);
                float poolD = axisLength * (0.86f + Rand01(leakIndex, 2427, scatterSeed) * 0.12f);
                if (canSpreadForward) poolD += axisLength * (0.18f + Rand01(leakIndex, 2429, scatterSeed) * 0.24f);
                XMFLOAT3 floorCenter = Add3({wallCenter.x, 0.0f, wallCenter.z}, Scale3(inward, poolD * 0.5f + 0.006f));
                addWaterLikeFloor(floorCenter.x, floorCenter.z, leakW, poolD, seamYaw, seed, 0.0f, 0.965f + seed * 0.025f);

                if (!wallOnly && !gEffectDebugViewer) {
                    BloodScarePoint scare{};
                    scare.pos = {floorCenter.x, 0.10f, floorCenter.z};
                    scare.source = {wallCenter.x, wallH - 0.035f, wallCenter.z};
                    scare.normal = normal;
                    scare.radius = std::clamp(1.65f + std::max(leakW, poolD) * 0.70f, 2.10f, 4.80f);
                    scare.focusDelaySeconds = 0.22f + Rand01(leakIndex, 2433, scatterSeed) * 0.44f;
                    scare.requireFacing = true;
                    scare.dreadScale = 0.42f;
                    scare.revealBlood = false;
                    bloodScarePoints_.push_back(scare);
                }
                return true;
            };

            auto emitWaterLikeDamage = [&]() {
                bool waterDebug = gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect);
                if (!waterDebug && waterLikeDamageChance <= 0.0001f) return;
                if (waterDebug) {
                    int debugIndex = 0;
                    for (Tile t : openTiles) {
                        int sideCount = 0;
                        for (int side = 0; side < 4; ++side) {
                            if (wallHasSurface(t, side)) {
                                ++sideCount;
                                addWaterLikeLeak(t, side, 30000 + debugIndex++, true);
                            }
                        }
                        if (sideCount == 0) {
                            addWaterLikeCenterTile(t, 33000 + debugIndex++);
                        }
                    }
                    return;
                }
                int waterIndex = 0;
                for (Tile t : openTiles) {
                    if (t == maze_.start || t == maze_.exit) continue;
                    int sides[4]{};
                    int sideCount = 0;
                    if (wallHasSurface(t, 0)) sides[sideCount++] = 0;
                    if (wallHasSurface(t, 1)) sides[sideCount++] = 1;
                    if (wallHasSurface(t, 2)) sides[sideCount++] = 2;
                    if (wallHasSurface(t, 3)) sides[sideCount++] = 3;
                    float chance = waterLikeDamageChance;
                    if (sideCount == 0) chance *= 0.58f;
                    if (Rand01(waterIndex, 2501, scatterSeed) > chance) {
                        ++waterIndex;
                        continue;
                    }
                    if (sideCount == 0) {
                        addWaterLikeCenterTile(t, 25000 + waterIndex);
                    } else {
                        int side = sides[std::min(sideCount - 1,
                            static_cast<int>(Rand01(waterIndex, 2507, scatterSeed) * static_cast<float>(sideCount)))];
                        addWaterLikeLeak(t, side, 25000 + waterIndex);
                    }
                    ++waterIndex;
                }
            };

            if (false) emitWaterLikeDamage();

            if (settings_.bloodStudyView) {
                bloodStudyTile_ = FindBloodStudyTile();
                XMFLOAT3 studyCenter = maze_.WorldCenter(bloodStudyTile_, 0.0f);
                int studyLeaks = 0;
                for (int side = 0; side < 4; ++side) {
                    if (wallHasSurface(bloodStudyTile_, side) &&
                        addBloodLeak(bloodStudyTile_, side, 30000 + side)) {
                        ++studyLeaks;
                    }
                }
                if (studyLeaks <= 0) {
                    if (!addBloodCenterDripTile(bloodStudyTile_, 30017)) {
                        addBloodBurst(bloodStudyTile_, 30017);
                    }
                }
            }

            bool bloodWorldGeometry = !gBloodDebugEveryWall &&
                settings_.bloodWorldCoverage > 0.001f &&
                (settings_.bloodWorldFlicker || settings_.bloodWorldAlwaysOn);
            if (bloodWorldGeometry) {
                int worldLeakIndex = 0;
                float bloodQuality = std::clamp(settings_.bloodShaderQuality, 0.25f, 1.0f);
                float coverageScale = std::clamp(bloodQuality * bloodQuality * 1.35f, 0.16f, 1.0f);
                float densityGate = std::clamp(settings_.bloodSplatterDensity, 0.0f, 1.0f);
                float coverage = std::clamp(settings_.bloodWorldCoverage * densityGate * coverageScale, 0.0f, 1.0f);
                for (Tile t : openTiles) {
                    int wallSides = 0;
                    for (int side = 0; side < 4; ++side) {
                        if (!wallHasSurface(t, side)) continue;
                        ++wallSides;
                        if (coverage < 0.999f && Rand01(worldLeakIndex, 1201, scatterSeed) > coverage) {
                            ++worldLeakIndex;
                            continue;
                        }
                        addBloodLeak(t, side, 20000 + worldLeakIndex, true);
                        ++worldLeakIndex;
                    }
                    if (wallSides == 0) {
                        if (coverage >= 0.999f || Rand01(worldLeakIndex, 1207, scatterSeed) <= coverage * 0.62f) {
                            addBloodCenterDripTile(t, 20000 + worldLeakIndex);
                        }
                        ++worldLeakIndex;
                    }
                }
            }

            if (gBloodDebugEveryWall && gDebugSliceEffect == DebugSliceEffect::Blood) {
                int debugLeakIndex = 0;
                for (Tile t : openTiles) {
                    for (int side = 0; side < 4; ++side) {
                        if (wallHasSurface(t, side)) {
                            addBloodLeak(t, side, 10000 + debugLeakIndex++);
                        }
                    }
                    if (maze_.IsOpen(t.x, t.y - 1) && maze_.IsOpen(t.x, t.y + 1) &&
                        maze_.IsOpen(t.x - 1, t.y) && maze_.IsOpen(t.x + 1, t.y)) {
                        addBloodCenterDripTile(t, 13000 + debugLeakIndex++);
                    }
                }
            } else {
                int bloodLeaks = std::clamp(static_cast<int>(std::round(static_cast<float>(settings_.bloodBurstCount) * settings_.bloodSplatterDensity)), 0, 180);
                int bloodLeakAttempts = std::max(0, bloodLeaks * 10);
                int placedBloodLeaks = 0;
                for (int b = 0; b < bloodLeakAttempts && placedBloodLeaks < bloodLeaks; ++b) {
                    Tile t = openTiles[std::min(openTiles.size() - 1,
                        static_cast<size_t>(Rand01(b, 571, scatterSeed) * static_cast<float>(openTiles.size())))];
                    if (t == maze_.start || t == maze_.exit) continue;
                    int sides[4]{};
                    int sideCount = 0;
                    if (!maze_.IsOpen(t.x, t.y - 1)) sides[sideCount++] = 0;
                    if (!maze_.IsOpen(t.x, t.y + 1)) sides[sideCount++] = 1;
                    if (!maze_.IsOpen(t.x - 1, t.y)) sides[sideCount++] = 2;
                    if (!maze_.IsOpen(t.x + 1, t.y)) sides[sideCount++] = 3;
                    if (sideCount == 0) {
                        if (addBloodCenterDripTile(t, b)) {
                            ++placedBloodLeaks;
                        }
                        continue;
                    }
                    int side = sides[std::min(sideCount - 1, static_cast<int>(Rand01(b, 579, scatterSeed) * static_cast<float>(sideCount)))];
                    if (addBloodLeak(t, side, b)) {
                        ++placedBloodLeaks;
                    }
                }
            }

            float roomClutterDensity = std::clamp(settings_.chairDensity * 0.85f, 0.0f, 4.0f);
            int roomGroups = roomClutterDensity <= 0.001f
                ? 0
                : std::clamp(static_cast<int>(static_cast<float>(openTiles.size()) * 0.010f * roomClutterDensity), 4, 42);
            int roomAttempts = roomGroups * 7;
            int placedRoomGroups = 0;
            for (int g = 0; g < roomAttempts && placedRoomGroups < roomGroups; ++g) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(g, 277, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                if (!IsRoomLike(t)) continue;
                if (addRoomClutterGroup(t, g, scatterSeed)) {
                    ++placedRoomGroups;
                }
            }

            int basePages = std::clamp(static_cast<int>(openTiles.size() / 3), 260, 900);
            int targetPages = std::clamp(static_cast<int>(basePages * paperDensity), 0, 3600);
            int attempts = targetPages * 4;
            int placed = 0;
            for (int i = 0; i < attempts && placed < targetPages; ++i) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(i, 17, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float px = c.x + (Rand01(i, 37, scatterSeed) - 0.5f) * std::max(0.10f, tileW - kA4PaperLongMeters - 0.10f);
                float pz = c.z + (Rand01(i, 41, scatterSeed) - 0.5f) * std::max(0.10f, tileD - kA4PaperLongMeters - 0.10f);
                float yaw = Rand01(i, 43, scatterSeed) * kPi * 2.0f;
                float lift = 0.039f + Rand01(i, 47, scatterSeed) * 0.010f;
                if (addPaperAt(px, pz, yaw, lift)) {
                    if (Rand01(i, 49, scatterSeed) < 0.010f) {
                        addCassetteAt(px, pz, yaw, lift + 0.003f, Rand01(i, 51, scatterSeed));
                    }
                    ++placed;
                }
            }

            int baseRuns = std::clamp(static_cast<int>(openTiles.size() / 220), 5, 14);
            int runs = std::clamp(static_cast<int>(std::round(baseRuns * hallwayPaperDensity)), 0, 56);
            for (int run = 0; run < runs; ++run) {
                size_t tileIndex = std::min(openTiles.size() - 1,
                    static_cast<size_t>(Rand01(run, 53, scatterSeed) * static_cast<float>(openTiles.size())));
                Tile t = openTiles[tileIndex];
                bool ew = maze_.IsOpen(t.x - 1, t.y) && maze_.IsOpen(t.x + 1, t.y);
                bool ns = maze_.IsOpen(t.x, t.y - 1) && maze_.IsOpen(t.x, t.y + 1);
                if (!ew && !ns) continue;
                bool useEW = ew && (!ns || Rand01(run, 59, scatterSeed) < 0.5f);
                XMFLOAT3 c = maze_.WorldCenter(t, 0.0f);
                float runLen = (useEW ? tileW : tileD) * (2.4f + Rand01(run, 61, scatterSeed) * 4.8f);
                int count = std::max(1, static_cast<int>((22.0f + Rand01(run, 67, scatterSeed) * 34.0f) * hallwayPaperDensity));
                for (int p = 0; p < count; ++p) {
                    float along = (Rand01(run * 97 + p, 71, scatterSeed) - 0.5f) * runLen;
                    float cross = (Rand01(run * 97 + p, 73, scatterSeed) - 0.5f) * ((useEW ? tileD : tileW) * 0.82f);
                    float px = c.x + (useEW ? along : cross);
                    float pz = c.z + (useEW ? cross : along);
                    float yaw = Rand01(run * 97 + p, 97, scatterSeed) * kPi * 2.0f;
                    float lift = 0.052f + p * 0.0011f + Rand01(run * 97 + p, 101, scatterSeed) * 0.008f;
                    if (addPaperAt(px, pz, yaw, lift) &&
                        Rand01(run * 97 + p, 103, scatterSeed) < 0.010f) {
                        addCassetteAt(px, pz, yaw, lift + 0.003f, Rand01(run * 97 + p, 107, scatterSeed));
                    }
                }
                propLookPoints_.push_back({c.x, 0.16f, c.z});
            }
        }

        const int lampStrideTiles = std::max(1, static_cast<int>(std::round(settings_.lampSpacing / std::max(0.001f, maze_.TileAverage()))));
        const int lampParityX = maze_.start.x % lampStrideTiles;
        const int lampParityY = maze_.start.y % lampStrideTiles;
        auto pickCeilingLampMesh = [&](int cellX, int cellZ) -> const StaticPropMesh* {
            std::array<const StaticPropMesh*, 4> candidates{};
            int count = 0;
            for (const StaticPropMesh& mesh : ceilingLampPropMeshes_) {
                if (!mesh.vertices.empty()) {
                    candidates[static_cast<size_t>(count++)] = &mesh;
                }
            }
            if (count <= 0) return nullptr;
            int index = std::min(count - 1, static_cast<int>(LampHash(static_cast<float>(cellX) + 13.7f,
                static_cast<float>(cellZ) - 29.4f) * static_cast<float>(count)));
            return candidates[static_cast<size_t>(index)];
        };
        struct LampFixtureFit {
            float spanY = 0.12f;
            float scale = 1.0f;
            float actualW = 0.8f;
            float actualD = 0.32f;
            float actualH = 0.12f;
            float aspect = 1.0f;
            bool square = false;
            bool hasMesh = false;
        };
        auto computeLampFixtureFit = [&](const StaticPropMesh* lampMesh, float halfW, float halfD) {
            LampFixtureFit fit{};
            fit.actualW = std::max(0.05f, halfW * 2.0f);
            fit.actualD = std::max(0.05f, halfD * 2.0f);
            fit.actualH = 0.12f;
            fit.aspect = std::max(fit.actualW, fit.actualD) / std::max(0.001f, std::min(fit.actualW, fit.actualD));
            fit.square = fit.aspect < 1.18f;
            if (lampMesh && !lampMesh->vertices.empty()) {
                float spanX = std::max(0.001f, propSpan(*lampMesh, 0));
                fit.spanY = std::max(0.001f, propSpan(*lampMesh, 1));
                float spanZ = std::max(0.001f, propSpan(*lampMesh, 2));
                float targetW = (halfW + 0.105f) * 2.0f;
                float targetD = (halfD + 0.085f) * 2.0f;
                float fitW = spanZ;
                float fitD = spanX;
                fit.scale = std::min(targetW / fitW, targetD / fitD);
                fit.scale = std::clamp(fit.scale, 0.05f, 4.0f);
                fit.actualW = fitW * fit.scale;
                fit.actualD = fitD * fit.scale;
                fit.actualH = fit.spanY * fit.scale;
                fit.aspect = std::max(fitW, fitD) / std::max(0.001f, std::min(fitW, fitD));
                fit.square = fit.aspect < 1.18f;
                fit.hasMesh = true;
            }
            return fit;
        };
        auto addLampFixture = [&](float cx, float ly, float cz, float halfW, float halfD, float fixtureYaw, float lampMaterial,
                                  const StaticPropMesh* lampMesh, bool grounded = false) {
            if (lampMesh && !lampMesh->vertices.empty()) {
                LampFixtureFit fit = computeLampFixtureFit(lampMesh, halfW, halfD);
                float meshYaw = fixtureYaw + kPi * 0.5f;
                bool appended = grounded
                    ? AppendStaticPropMeshGrounded(vertices, indices, *lampMesh, {cx, ly, cz}, meshYaw, fit.scale, fit.scale, fit.scale)
                    : AppendStaticPropMesh(vertices, indices, *lampMesh, {cx, ly, cz}, meshYaw, fit.scale, fit.scale, fit.scale);
                if (appended) {
                    float diffuserY = grounded
                        ? ly + std::max(0.020f, fit.actualH * 0.28f)
                        : ly + lampMesh->min.y * fit.scale - 0.006f;
                    float rodYaw = fixtureYaw;
                    float rodLength = std::max(fit.actualW, fit.actualD);
                    float rodCross = std::min(fit.actualW, fit.actualD);
                    bool lengthAlongRight = fit.actualW >= fit.actualD;
                    if (!lengthAlongRight) {
                        rodYaw += kPi * 0.5f;
                    }
                    float c = std::cos(fixtureYaw);
                    float s = std::sin(fixtureYaw);
                    XMFLOAT3 forward{s, 0.0f, c};
                    XMFLOAT3 right{c, 0.0f, -s};
                    XMFLOAT3 spacingAxis = lengthAlongRight ? forward : right;
                    int rodCount = 2;
                    if (fit.aspect < 1.35f) {
                        rodCount = 4;
                    } else if (fit.aspect < 2.35f) {
                        rodCount = 3;
                    }
                    float rodHalfLength = std::max(0.050f, rodLength * 0.490f);
                    float rodHalfWidth = std::clamp(rodCross * 0.040f, 0.010f, 0.022f);
                    float rodInset = std::clamp(rodCross * 0.14f, 0.018f, 0.055f);
                    float usableCross = std::max(0.0f, rodCross * 0.5f - rodHalfWidth - rodInset);
                    for (int rod = 0; rod < rodCount; ++rod) {
                        float t = rodCount > 1
                            ? (static_cast<float>(rod) / static_cast<float>(rodCount - 1)) * 2.0f - 1.0f
                            : 0.0f;
                        float offset = t * usableCross;
                        XMFLOAT3 center = Add3({cx, diffuserY, cz}, Scale3(spacingAxis, offset));
                        AddOrientedBox(vertices, indices, center,
                            {rodHalfLength, 0.004f, rodHalfWidth}, rodYaw, lampMaterial);
                    }
                    return;
                }
            }
            float c = std::cos(fixtureYaw);
            float s = std::sin(fixtureYaw);
            XMFLOAT3 right{c, 0.0f, -s};
            XMFLOAT3 up{0.0f, 1.0f, 0.0f};
            XMFLOAT3 forward{s, 0.0f, c};
            auto fixturePos = [&](float x, float y, float z) {
                return Add3({cx, ly, cz}, OrientedOffset(right, up, forward, x, y, z));
            };
            AddOrientedBox(vertices, indices, fixturePos(0.0f, 0.030f, 0.0f), {halfW + 0.105f, 0.016f, halfD + 0.085f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos(0.0f, 0.0f, 0.0f), {halfW, 0.018f, halfD}, fixtureYaw, lampMaterial);
            AddOrientedBox(vertices, indices, fixturePos(-halfW - 0.035f, 0.006f, 0.0f), {0.035f, 0.043f, halfD + 0.075f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos( halfW + 0.035f, 0.006f, 0.0f), {0.035f, 0.043f, halfD + 0.075f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos(0.0f, 0.006f, -halfD - 0.035f), {halfW + 0.085f, 0.043f, 0.035f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos(0.0f, 0.006f,  halfD + 0.035f), {halfW + 0.085f, 0.043f, 0.035f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos(0.0f, -0.030f, 0.0f), {0.020f, 0.018f, halfD * 0.88f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos(0.0f, -0.036f, -halfD * 0.38f), {halfW * 0.94f, 0.010f, 0.018f}, fixtureYaw, lampMaterial);
            AddOrientedBox(vertices, indices, fixturePos(0.0f, -0.036f,  halfD * 0.38f), {halfW * 0.94f, 0.010f, 0.018f}, fixtureYaw, lampMaterial);
            AddOrientedBox(vertices, indices, fixturePos(-halfW * 0.84f, -0.034f, -halfD * 0.38f), {0.018f, 0.014f, 0.030f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos( halfW * 0.84f, -0.034f, -halfD * 0.38f), {0.018f, 0.014f, 0.030f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos(-halfW * 0.84f, -0.034f,  halfD * 0.38f), {0.018f, 0.014f, 0.030f}, fixtureYaw, 10.0f);
            AddOrientedBox(vertices, indices, fixturePos( halfW * 0.84f, -0.034f,  halfD * 0.38f), {0.018f, 0.014f, 0.030f}, fixtureYaw, 10.0f);
        };
        for (int tileY = 0; tileY < maze_.h; ++tileY) {
            if ((tileY - lampParityY) % lampStrideTiles != 0) continue;
            for (int tileX = 0; tileX < maze_.w; ++tileX) {
                if ((tileX - lampParityX) % lampStrideTiles != 0) continue;
                if (!maze_.IsOpen(tileX, tileY)) continue;

                Tile lampTile{tileX, tileY};
                XMFLOAT3 lampCenter = maze_.WorldCenter(lampTile, 0.0f);
                float cx = lampCenter.x;
                float cz = lampCenter.z;
                int cellX = (tileX - lampParityX) / lampStrideTiles;
                int cellZ = (tileY - lampParityY) / lampStrideTiles;
                bool brokenZone = LampBrokenZone(cellX, cellZ);
                bool lampOn = !brokenZone && LampSeed(cellX, cellZ) >= 1.0f - settings_.lampOnRatio;
                bool showDarkFixture = !brokenZone &&
                    LampHash(static_cast<float>(cellX) + 73.1f, static_cast<float>(cellZ) - 41.7f) < settings_.darkLampVisibleRatio;
                bool brokenFixture = brokenZone && settings_.sparkParticles &&
                    LampHash(static_cast<float>(cellX) - 19.7f, static_cast<float>(cellZ) + 88.4f) < settings_.sparkEmitterRatio;
                if (!lampOn && !showDarkFixture && !brokenFixture) continue;

                float halfW = tileW * 0.34f;
                float halfD = tileD * 0.12f;
                float fixtureYaw = kPi * 0.5f;
                float lampMaterial = lampOn
                    ? 3.0f + LampSeed(cellX, cellZ) * 0.49f
                    : 5.0f;
                const StaticPropMesh* lampMesh = pickCeilingLampMesh(cellX, cellZ);
                LampFixtureFit lampFit = computeLampFixtureFit(lampMesh, halfW, halfD);
                float ly = lampMesh
                    ? wallH - (brokenFixture ? 0.055f : 0.030f)
                    : wallH - (brokenFixture ? 0.19f : 0.12f);
                if (brokenFixture) {
                    float bodyHalfW = std::max(0.055f, lampFit.actualW * 0.5f);
                    float bodyHalfD = std::max(0.045f, lampFit.actualD * 0.5f);
                    float dropYaw = fixtureYaw + kPi * 0.5f +
                        (LampHash(static_cast<float>(cellX) + 113.9f, static_cast<float>(cellZ) - 57.4f) - 0.5f) * kPi * 1.35f;
                    float footprintW = std::max((halfW + 0.17f) * 2.0f, lampFit.actualW + 0.24f);
                    float footprintD = std::max((halfD + 0.14f) * 2.0f, lampFit.actualD + 0.24f);
                    bool floorClear = reserveFloorFootprint(cx, cz, footprintW, footprintD, dropYaw, 0.060f);
                    auto fixtureOffset = [&](float yaw, float x, float y, float z) {
                        float c = std::cos(yaw);
                        float s = std::sin(yaw);
                        XMFLOAT3 right{c, 0.0f, -s};
                        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                        XMFLOAT3 forward{s, 0.0f, c};
                        return Add3({cx, y, cz}, OrientedOffset(right, up, forward, x, 0.0f, z));
                    };
                    float cableX = std::min(bodyHalfW * 0.58f, std::max(0.035f, bodyHalfW - 0.035f));
                    float cableZ = std::min(bodyHalfD * 0.50f, std::max(0.030f, bodyHalfD - 0.030f));
                    auto addCeilingCable = [&](float x, float z, float halfLen, float thickness) {
                        AddOrientedBox(vertices, indices,
                            fixtureOffset(fixtureYaw, x, wallH - halfLen - 0.004f, z),
                            {thickness, halfLen, thickness}, fixtureYaw, 10.0f);
                    };
                    addCeilingCable(-cableX, -cableZ * 0.55f, 0.22f, 0.0075f);
                    addCeilingCable( cableX * 0.82f,  cableZ * 0.72f, 0.18f, 0.0070f);
                    addCeilingCable(-cableX * 0.15f,  cableZ * 0.22f, 0.13f, 0.0060f);
                    AddOrientedBox(vertices, indices,
                        fixtureOffset(fixtureYaw, -cableX * 0.32f, wallH - 0.27f, cableZ * 0.10f),
                        {std::max(0.030f, bodyHalfW * 0.18f), 0.0055f, 0.0055f}, fixtureYaw, 10.0f);
                    if (floorClear) {
                        float floorY = lampFit.square ? 0.075f : 0.108f;
                        addLampFixture(cx, floorY, cz, halfW, halfD, dropYaw, 5.0f, lampMesh, true);
                        AddOrientedBox(vertices, indices,
                            fixtureOffset(dropYaw, -bodyHalfW * 0.58f, 0.060f, -bodyHalfD * 0.58f),
                            {0.006f, 0.006f, std::max(0.030f, bodyHalfD * 0.38f)}, dropYaw, 10.0f);
                        AddOrientedBox(vertices, indices,
                            fixtureOffset(dropYaw,  bodyHalfW * 0.42f, 0.058f,  bodyHalfD * 0.54f),
                            {std::max(0.035f, bodyHalfW * 0.24f), 0.006f, 0.006f}, dropYaw, 10.0f);
                    }
                    sparkEmitters_.push_back({fixtureOffset(fixtureYaw, -cableX, wallH - 0.40f, -cableZ * 0.55f)});
                    continue;
                }
                addLampFixture(cx, ly, cz, halfW, halfD, fixtureYaw, lampMaterial, lampMesh);
                if (lampOn) {
                    runtimeLamps_.push_back({
                        lampTile,
                        {cx, wallH - 0.28f, cz},
                        0.0f,
                        RandRange(0.08f, 0.72f),
                        false
                    });
                }
            }
        }

        CreateLampDamageTexture();

        floorCeilingStartIndex_ = static_cast<UINT>(indices.size());
        for (int y = 0; y < maze_.h; ++y) {
            int x = 0;
            while (x < maze_.w) {
                while (x < maze_.w && !maze_.IsOpen(x, y)) ++x;
                int start = x;
                while (x < maze_.w && maze_.IsOpen(x, y)) ++x;
                if (start < x) addFloorCeilingRun(y, start, x);
            }
        }
        floorCeilingIndexCount_ = static_cast<UINT>(indices.size()) - floorCeilingStartIndex_;
        staticWaterStartIndex_ = static_cast<UINT>(indices.size());
        indices.insert(indices.end(), waterIndices.begin(), waterIndices.end());
        staticWaterIndexCount_ = static_cast<UINT>(indices.size()) - staticWaterStartIndex_;
        staticTransparentStartIndex_ = static_cast<UINT>(indices.size());
        indices.insert(indices.end(), transparentIndices.begin(), transparentIndices.end());
        staticTransparentIndexCount_ = static_cast<UINT>(indices.size()) - staticTransparentStartIndex_;
        staticPropShadowStartIndex_ = static_cast<UINT>(indices.size());
        indices.insert(indices.end(), propShadowIndices.begin(), propShadowIndices.end());
        staticPropShadowIndexCount_ = static_cast<UINT>(indices.size()) - staticPropShadowStartIndex_;

        D3D11_BUFFER_DESC vb{};
        vb.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(Vertex));
        vb.Usage = D3D11_USAGE_IMMUTABLE;
        vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        D3D11_SUBRESOURCE_DATA vd{vertices.data(), 0, 0};
        device_->CreateBuffer(&vb, &vd, &vertexBuffer_);

        D3D11_BUFFER_DESC ib{};
        ib.ByteWidth = static_cast<UINT>(indices.size() * sizeof(uint32_t));
        ib.Usage = D3D11_USAGE_IMMUTABLE;
        ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
        D3D11_SUBRESOURCE_DATA id{indices.data(), 0, 0};
        device_->CreateBuffer(&ib, &id, &indexBuffer_);
        indexCount_ = static_cast<UINT>(indices.size());

        D3D11_BUFFER_DESC mb{};
        mb.ByteWidth = sizeof(Vertex) * 6;
        mb.Usage = D3D11_USAGE_DYNAMIC;
        mb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        mb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&mb, nullptr, &monsterBuffer_);

        D3D11_BUFFER_DESC db{};
        db.ByteWidth = sizeof(Vertex) * kDynamicVertexCapacity;
        db.Usage = D3D11_USAGE_DYNAMIC;
        db.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        db.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&db, nullptr, &dynamicBuffer_);

        D3D11_BUFFER_DESC ob{};
        ob.ByteWidth = sizeof(OverlayVertex) * kOverlayVertexCapacity;
        ob.Usage = D3D11_USAGE_DYNAMIC;
        ob.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        ob.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device_->CreateBuffer(&ob, nullptr, &overlayBuffer_);
    }

    float MonsterSightDistance() const {
        float visibleDistance = std::max(0.1f, settings_.monsterVisibleDistance);
        if (settings_.fogDarkness > 0.02f) {
            visibleDistance = std::min(visibleDistance, std::max(0.1f, settings_.fogEndMeters));
        }
        return visibleDistance;
    }

    bool MonsterLineOfSightToPlayer() const {
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        if (!maze_.IsOpen(mt.x, mt.y) || !maze_.IsOpen(ct.x, ct.y)) return false;
        float dx = monster_.x - camera_.x;
        float dz = monster_.z - camera_.z;
        float visibleDistance = MonsterSightDistance();
        if (dx * dx + dz * dz > visibleDistance * visibleDistance) return false;
        return maze_.LineClear(mt, ct);
    }

    float MonsterHeadBobOffset() const {
        float phase = monsterPreview_ ? time_ * 2.35f : monsterHeadBobPhase_;
        float chase = monsterPreview_ ? 0.0f : monsterHeadChaseBlend_;
        float amplitude = Lerp(0.050f, 0.086f, chase);
        float secondary = Lerp(0.006f, 0.014f, chase);
        return std::sin(phase) * amplitude + std::sin(phase * 2.0f + 0.65f) * secondary;
    }

    // Player/camera movement, navigation, attention, chase, and camera-state helpers. 
#include "game/player_camera_movement.inl"

    bool MonsterFootprintOpen(const XMFLOAT3& pos) const {
        float visualScale = std::clamp(settings_.monsterScale, 0.35f, 1.35f);
        float radius = std::clamp(0.62f * visualScale, 0.26f, 0.92f);
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { radius, 0.0f},
            {-radius, 0.0f},
            {0.0f,  radius},
            {0.0f, -radius},
            { radius * 0.92f,  radius * 0.24f},
            {-radius * 0.92f,  radius * 0.24f},
            { radius * 0.92f, -radius * 0.24f},
            {-radius * 0.92f, -radius * 0.24f},
            { radius * 0.24f,  radius * 0.92f},
            {-radius * 0.24f,  radius * 0.92f},
            { radius * 0.24f, -radius * 0.92f},
            {-radius * 0.24f, -radius * 0.92f},
            { radius * 0.62f,  radius * 0.62f},
            {-radius * 0.62f,  radius * 0.62f},
            { radius * 0.62f, -radius * 0.62f},
            {-radius * 0.62f, -radius * 0.62f}
        };
        for (const XMFLOAT2& s : samples) {
            Tile tile = maze_.TileFromWorld(pos.x + s.x, pos.z + s.y);
            if (!maze_.IsOpen(tile.x, tile.y)) return false;
        }
        return true;
    }

    bool MonsterMoveSegmentOpen(const XMFLOAT3& from, const XMFLOAT3& to) const {
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(2, static_cast<int>(std::ceil(len / (maze_.TileMinimum() * 0.055f))));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            XMFLOAT3 p{from.x + dx * t, 0.0f, from.z + dz * t};
            if (!MonsterFootprintOpen(p)) return false;
        }
        return true;
    }

    void MoveMonsterToward(const XMFLOAT3& target, float distance) {
        if (distance <= 0.0001f) return;
        XMFLOAT3 start = monster_;
        XMFLOAT3 delta{target.x - start.x, 0.0f, target.z - start.z};
        float len = Length3(delta);
        if (len <= 0.0001f) return;
        XMFLOAT3 dir = Scale3(delta, 1.0f / len);
        float remaining = std::min(distance, len);
        int steps = std::max(1, static_cast<int>(std::ceil(remaining / (maze_.TileMinimum() * 0.055f))));
        XMFLOAT3 pos = start;
        bool moved = false;
        for (int i = 0; i < steps; ++i) {
            float step = remaining / static_cast<float>(steps);
            XMFLOAT3 next{pos.x + dir.x * step, 0.0f, pos.z + dir.z * step};
            if (!MonsterFootprintOpen(next)) {
                XMFLOAT3 center = maze_.WorldCenter(MonsterTile(), 0.0f);
                if (MonsterFootprintOpen(center)) {
                    monster_ = center;
                    monsterPath_.clear();
                    monsterPathIndex_ = 0;
                    monsterRepath_ = 0.0f;
                }
                break;
            }
            pos = next;
            moved = true;
        }
        if (moved) {
            monster_ = pos;
            float moveYaw = std::atan2(monster_.x - start.x, monster_.z - start.z);
            if (std::isfinite(moveYaw)) {
                monsterYaw_ += AngleWrap(moveYaw - monsterYaw_) * 0.35f;
            }
        }
    }

    bool ValidMonsterTile(Tile t) const {
        return maze_.IsOpen(t.x, t.y);
    }

    void ClearMonsterPath() {
        monsterPath_.clear();
        monsterPathIndex_ = 0;
        monsterRepath_ = 0.0f;
    }

    void SetMonsterGoal(Tile goal, bool force = false) {
        if (!ValidMonsterTile(goal)) return;
        if (force || !(goal == monsterGoal_)) {
            monsterGoal_ = goal;
            ClearMonsterPath();
        }
    }

    bool MonsterCanSeePlayer() const {
        return MonsterLineOfSightToPlayer();
    }

    void UpdateMonsterHeadAnimation(float dt, bool seesPlayer) {
        dt = std::clamp(dt, 0.0f, 0.10f);
        Tile mt = MonsterTile();
        bool validTile = ValidMonsterTile(mt);
        bool openScanSpace = validTile &&
            (maze_.OpenNeighborCount(mt) >= 3 || maze_.LocalOpenCount(mt, 2) >= 14);
        bool activeChase = seesPlayer || monsterChasingVisible_ || monsterHasLastKnown_ ||
            monsterRecognizedForChase_ || chaseMemoryTimer_ > 0.0f || chasePanic_ > 0.08f;

        monsterCanSeePlayerNow_ = seesPlayer;
        monsterHeadChaseBlend_ += ((activeChase ? 1.0f : 0.0f) - monsterHeadChaseBlend_) *
            std::min(1.0f, dt * (activeChase ? 3.4f : 1.25f));
        monsterHeadLockAmount_ += ((seesPlayer ? 1.0f : 0.0f) - monsterHeadLockAmount_) *
            std::min(1.0f, dt * (seesPlayer ? 7.5f : 2.4f));

        float bobSpeed = Lerp(2.35f, 7.85f, monsterHeadChaseBlend_);
        monsterHeadBobPhase_ += dt * bobSpeed;
        if (monsterHeadBobPhase_ > kPi * 64.0f) {
            monsterHeadBobPhase_ = std::fmod(monsterHeadBobPhase_, kPi * 2.0f);
        }

        float scanRate = seesPlayer ? 0.0f : (openScanSpace ? 0.92f : 0.34f);
        monsterHeadScanPhase_ += dt * scanRate;
        if (monsterHeadScanPhase_ > kPi * 64.0f) {
            monsterHeadScanPhase_ = std::fmod(monsterHeadScanPhase_, kPi * 2.0f);
        }

        float yawRange = openScanSpace ? 0.62f : 0.25f;
        float targetYaw = seesPlayer ? 0.0f :
            (std::sin(monsterHeadScanPhase_) * yawRange +
             std::sin(monsterHeadScanPhase_ * 0.43f + 1.1f) * yawRange * 0.24f);
        float targetPitch = seesPlayer ? 0.0f :
            (std::sin(monsterHeadScanPhase_ * 0.71f + 0.4f) * (openScanSpace ? 0.085f : 0.035f) - 0.018f);
        float response = seesPlayer ? 8.0f : (openScanSpace ? 1.75f : 1.05f);
        monsterHeadYawOffset_ += AngleWrap(targetYaw - monsterHeadYawOffset_) *
            std::min(1.0f, dt * response);
        monsterHeadPitchOffset_ += (targetPitch - monsterHeadPitchOffset_) *
            std::min(1.0f, dt * response);
    }

    bool MonsterReachedTile(Tile t) const {
        if (!(MonsterTile() == t)) return false;
        XMFLOAT3 center = maze_.WorldCenter(t, 0.0f);
        float dx = center.x - monster_.x;
        float dz = center.z - monster_.z;
        float tile = maze_.TileMinimum();
        return dx * dx + dz * dz < tile * tile * 0.035f;
    }

    void AlertMonsterToSound(const XMFLOAT3& pos) {
        Tile sound = maze_.TileFromWorld(pos.x, pos.z);
        if (!ValidMonsterTile(sound)) return;
        monsterSoundTile_ = sound;
        monsterHasSound_ = true;
        monsterHasLastKnown_ = false;
        monsterChasingVisible_ = false;
        monsterSearchTimer_ = 0.0f;
        monsterRoamTimer_ = 0.0f;
        SetMonsterGoal(sound, true);
    }

    void AlertMonsterToPlayerTrigger(const XMFLOAT3& fallbackPos) {
        Tile player = CameraTile();
        if (ValidMonsterTile(player)) {
            XMFLOAT3 ping = maze_.WorldCenter(player, 0.0f);
            AlertMonsterToSound(ping);
        } else {
            AlertMonsterToSound(fallbackPos);
        }
    }

    Tile ChooseMonsterRoamTile(Tile from) {
        Tile best = from;
        float bestScore = -1.0e9f;
        for (int attempt = 0; attempt < 56; ++attempt) {
            Tile t{
                1 + static_cast<int>(rng_() % std::max(1, maze_.w - 2)),
                1 + static_cast<int>(rng_() % std::max(1, maze_.h - 2))
            };
            if (!ValidMonsterTile(t) || t == maze_.start) continue;
            std::vector<Tile> path = maze_.Path(from, t);
            if (path.size() < 7) continue;
            float cameraSeparation = TileDistanceSq(t, CameraTile());
            float score = static_cast<float>(path.size()) * 1.25f
                + static_cast<float>(maze_.LocalOpenCount(t, 2)) * 3.0f
                + std::min(cameraSeparation, 180.0f) * 0.18f
                + RandRange(0.0f, 24.0f);
            if (maze_.LineClear(t, CameraTile())) score -= 35.0f;
            if (score > bestScore) {
                bestScore = score;
                best = t;
            }
        }

        if (best == from) {
            std::vector<Tile> neighbors = maze_.Neighbors(from);
            if (!neighbors.empty()) {
                best = neighbors[static_cast<size_t>(rng_() % neighbors.size())];
            }
        }
        return best;
    }

    void UpdateMonster(float dt) {
        monsterRepath_ -= dt;
        Tile mt = MonsterTile();
        Tile ct = CameraTile();
        if (!maze_.IsOpen(mt.x, mt.y)) {
            monster_ = maze_.WorldCenter(maze_.exit, 0.0f);
            ClearMonsterPath();
            UpdateMonsterHeadAnimation(dt, false);
            return;
        }

        bool seesPlayer = MonsterCanSeePlayer();
        if (seesPlayer) {
            monsterChasingVisible_ = true;
            monsterHasLastKnown_ = true;
            monsterLastKnownTile_ = ct;
            monsterSearchTimer_ = 3.2f;
            SetMonsterGoal(ct);
        } else if (monsterChasingVisible_) {
            monsterChasingVisible_ = false;
            if (monsterHasLastKnown_) {
                monsterSearchTimer_ = RandRange(1.35f, 2.85f);
                SetMonsterGoal(monsterLastKnownTile_, true);
            }
        } else if (monsterHasLastKnown_) {
            SetMonsterGoal(monsterLastKnownTile_);
            if (MonsterReachedTile(monsterLastKnownTile_)) {
                monsterSearchTimer_ -= dt;
                if (monsterSearchTimer_ <= 0.0f) {
                    monsterHasLastKnown_ = false;
                    monsterGoal_ = {-1000, -1000};
                    ClearMonsterPath();
                }
            }
        } else if (monsterHasSound_) {
            SetMonsterGoal(monsterSoundTile_);
            if (MonsterReachedTile(monsterSoundTile_)) {
                monsterHasSound_ = false;
                monsterGoal_ = {-1000, -1000};
                monsterRoamTimer_ = 0.0f;
                ClearMonsterPath();
            }
        } else {
            monsterRoamTimer_ -= dt;
            if (!ValidMonsterTile(monsterRoamTile_) || MonsterReachedTile(monsterRoamTile_) || monsterRoamTimer_ <= 0.0f) {
                monsterRoamTile_ = ChooseMonsterRoamTile(mt);
                monsterRoamTimer_ = RandRange(4.5f, 10.0f);
                SetMonsterGoal(monsterRoamTile_, true);
            } else {
                SetMonsterGoal(monsterRoamTile_);
            }
        }

        UpdateMonsterHeadAnimation(dt, seesPlayer);
        if (!ValidMonsterTile(monsterGoal_)) return;

        bool needPath = monsterRepath_ <= 0.0f || monsterPathIndex_ >= monsterPath_.size()
            || monsterPath_.empty() || !(monsterPath_.back() == monsterGoal_);
        if (needPath) {
            monsterPath_ = maze_.Path(mt, monsterGoal_);
            monsterPathIndex_ = monsterPath_.size() > 1 ? 1 : 0;
            monsterRepath_ = seesPlayer ? 0.22f : (monsterHasLastKnown_ ? 0.42f : 0.95f);
            if (monsterPath_.empty()) {
                monsterHasSound_ = false;
                monsterHasLastKnown_ = false;
                monsterRoamTimer_ = 0.0f;
                monsterGoal_ = {-1000, -1000};
                return;
            }
        }
        if (monsterPathIndex_ < monsterPath_.size()) {
            XMFLOAT3 target = maze_.WorldCenter(monsterPath_[monsterPathIndex_], 0.0f);
            XMFLOAT3 tileCenter = maze_.WorldCenter(mt, 0.0f);
            if (!MonsterMoveSegmentOpen(monster_, target)) {
                target = tileCenter;
                monsterPathIndex_ = 0;
                monsterRepath_ = 0.0f;
            }
            float dx = target.x - monster_.x;
            float dz = target.z - monster_.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < 0.12f) {
                ++monsterPathIndex_;
                if (monsterPathIndex_ == 1) {
                    monsterRepath_ = 0.0f;
                }
            } else {
                float speed = 0.82f * settings_.monsterSpeed;
                if (seesPlayer) {
                    float proximity = Clamp01((7.2f - MonsterDistance()) / 6.2f);
                    speed = Lerp(2.20f, 3.65f, proximity) * settings_.monsterSprintSpeed;
                } else if (monsterHasLastKnown_) {
                    speed = 2.10f * settings_.monsterSprintSpeed;
                } else if (monsterHasSound_) {
                    speed = 1.62f * settings_.monsterSpeed;
                }
                MoveMonsterToward(target, std::min(dist, speed * dt));
            }
        }
    }

    bool PlayerLooksAt(const XMFLOAT3& p, float maxDist, float minDot) const {
        float dx = p.x - camera_.x;
        float dy = p.y - camera_.y;
        float dz = p.z - camera_.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < 0.001f || dist > maxDist) return false;
        XMFLOAT3 viewDir = DirectionFromYawPitch(yaw_, lookPitch_);
        float dot = (dx * viewDir.x + dy * viewDir.y + dz * viewDir.z) / dist;
        if (dot < minDot) return false;
        Tile targetTile = maze_.TileFromWorld(p.x, p.z);
        return maze_.LineClear(CameraTile(), targetTile);
    }

    float DistanceToPoint(const XMFLOAT3& p) const {
        float dx = p.x - camera_.x;
        float dy = p.y - camera_.y;
        float dz = p.z - camera_.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    float ScareSensoryWeight(const XMFLOAT3& p, float visualDistance, float minDot, float loudDistance) const {
        if (PlayerLooksAt(p, visualDistance, minDot)) return 1.0f;
        float dist = DistanceToPoint(p);
        if (dist > loudDistance) return 0.0f;
        float close = Clamp01((loudDistance - dist) / std::max(0.1f, loudDistance));
        return 0.42f + close * 0.46f;
    }

    bool ScareSourceAhead(const XMFLOAT3& p, float minDistance, float maxDistance, int maxPathTiles, float minForwardDot) const {
        float dx = p.x - camera_.x;
        float dz = p.z - camera_.z;
        float horizontalDist = std::sqrt(dx * dx + dz * dz);
        if (horizontalDist < minDistance || horizontalDist > maxDistance) return false;

        Tile cameraTile = CameraTile();
        Tile sourceTile = maze_.TileFromWorld(p.x, p.z);
        if (!maze_.LineClear(cameraTile, sourceTile)) return false;

        XMFLOAT3 forward = Forward();
        float facing = horizontalDist > 0.001f ? (dx * forward.x + dz * forward.z) / horizontalDist : 1.0f;
        bool upcomingPath = false;
        size_t first = std::min(pathIndex_, path_.size());
        size_t last = std::min(path_.size(), first + static_cast<size_t>(std::max(1, maxPathTiles)));
        for (size_t i = first; i < last; ++i) {
            if (path_[i] == sourceTile) {
                upcomingPath = true;
                break;
            }
        }
        if (sourceTile == cameraTile && facing > 0.18f) {
            upcomingPath = true;
        }
        return (upcomingPath && facing > -0.18f) || facing >= minForwardDot;
    }

    void EmitSparkBurstAt(const XMFLOAT3& pos, float intensity = 1.0f) {
        if (!settings_.sparkParticles || settings_.sparkMaxParticles <= 0) return;
        int count = static_cast<int>((5.0f + RandRange(0.0f, 8.9f)) * intensity);
        for (int i = 0; i < count && sparks_.size() < static_cast<size_t>(settings_.sparkMaxParticles); ++i) {
            float yaw = RandRange(-1.15f, 1.15f) + LampHash(pos.x, pos.z) * kPi;
            float speed = RandRange(0.35f, 1.25f) * (0.85f + intensity * 0.30f);
            SparkParticle sp{};
            sp.pos = pos;
            sp.vel = {std::sin(yaw) * speed, RandRange(0.15f, 0.92f) * (0.7f + intensity * 0.45f), std::cos(yaw) * speed};
            sp.age = 0.0f;
            sp.life = RandRange(0.55f, 1.35f) * (0.85f + intensity * 0.12f);
            sp.size = RandRange(0.018f, 0.040f) * settings_.sparkSize * (0.9f + intensity * 0.12f);
            sparks_.push_back(sp);
        }
        SparkFlash flash{};
        flash.pos = pos;
        flash.age = 0.0f;
        flash.life = std::clamp(0.16f + intensity * 0.035f, 0.16f, 0.42f);
        flash.intensity = std::clamp(intensity * 2.2f, 0.3f, 12.0f);
        sparkFlashes_.push_back(flash);
    }

    void SpawnSparkBurst(const SparkEmitter& emitter, float intensity = 1.0f) {
        EmitSparkBurstAt(emitter.pos, intensity);
    }

    void ScheduleSparkChain(const XMFLOAT3& pos, float intensity, int bursts) {
        SparkChain chain{};
        chain.pos = pos;
        chain.timer = RandRange(0.055f, 0.16f);
        chain.intensity = intensity;
        chain.remaining = std::max(0, bursts);
        sparkChains_.push_back(chain);
    }

    void MarkLampDamagePixel(Tile tile, float damage) {
        if (!maze_.InBounds(tile.x, tile.y) || lampDamagePixels_.empty()) return;
        size_t index = static_cast<size_t>(tile.y * maze_.w + tile.x);
        if (index >= lampDamagePixels_.size()) return;
        uint8_t value = Byte(damage);
        if (value > lampDamagePixels_[index]) {
            lampDamagePixels_[index] = value;
            lampDamageDirty_ = true;
        }
    }

    void BreakRuntimeLamp(RuntimeLampState& lamp) {
        if (lamp.broken) return;
        lamp.broken = true;
        lamp.damage = 1.0f;
        MarkLampDamagePixel(lamp.tile, lamp.damage);
        if (settings_.sparkParticles) {
            float intensity = std::max(2.2f, PickBrokenLampSparkIntensity() * 1.18f);
            EmitSparkBurstAt(lamp.pos, intensity);
            ScheduleSparkChain(lamp.pos, intensity * settings_.effectBrokenLampChainIntensityScale,
                std::max(2, PickBrokenLampChainBursts() + 1));
        }
    }

    void UpdateMonsterLampDamage(float dt) {
        if (dt <= 0.0f || monsterPreview_ || gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView) return;
        if (runtimeLamps_.empty() || lampDamagePixels_.empty()) return;

        Tile monsterTile = MonsterTile();
        if (!maze_.IsOpen(monsterTile.x, monsterTile.y)) return;
        float tileAvg = std::max(0.1f, maze_.TileAverage());
        float influenceRadius = std::max(tileAvg * 2.15f, 3.25f);
        float breakRadius = influenceRadius * 0.72f;

        for (RuntimeLampState& lamp : runtimeLamps_) {
            if (lamp.broken) continue;

            float dx = lamp.pos.x - monster_.x;
            float dz = lamp.pos.z - monster_.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            bool affected = dist <= influenceRadius && maze_.LineClear(monsterTile, lamp.tile);
            float oldDamage = lamp.damage;

            if (affected) {
                float proximity = Clamp01((influenceRadius - dist) / std::max(0.001f, influenceRadius));
                float close = SmoothStep(0.0f, 1.0f, proximity);
                lamp.damage = std::max(lamp.damage, 0.12f * close);
                lamp.damage += dt * (0.10f + close * 0.92f + close * close * 0.65f);
                if (dist < breakRadius && lamp.damage > 0.58f) {
                    lamp.damage += dt * Lerp(0.18f, 0.58f, close);
                }
            } else if (lamp.damage > 0.055f) {
                lamp.damage += dt * (0.014f + lamp.damage * 0.036f);
            }

            lamp.damage = Clamp01(lamp.damage);
            if (lamp.damage > oldDamage + 0.001f) {
                MarkLampDamagePixel(lamp.tile, lamp.damage);
            }

            float fail = SmoothStep(0.28f, 0.96f, lamp.damage);
            if (fail > 0.001f) {
                lamp.sparkTimer -= dt * Lerp(0.85f, affected ? 3.5f : 1.7f, fail);
                if (lamp.sparkTimer <= 0.0f) {
                    if (settings_.sparkParticles) {
                        float intensity = Lerp(0.42f, 2.55f, fail);
                        EmitSparkBurstAt(lamp.pos, intensity);
                        if (fail > 0.68f && RandRange(0.0f, 1.0f) < Lerp(0.16f, 0.62f, fail)) {
                            ScheduleSparkChain(lamp.pos, intensity * settings_.effectBrokenLampChainIntensityScale,
                                std::max(1, PickBrokenLampChainBursts() / 2));
                        }
                    }
                    float cooldown = Lerp(1.75f, 0.12f, fail);
                    lamp.sparkTimer = RandRange(cooldown * 0.55f, cooldown * 1.25f);
                    if (affected && fail > 0.80f) {
                        lamp.damage = Clamp01(lamp.damage + 0.025f);
                        MarkLampDamagePixel(lamp.tile, lamp.damage);
                    }
                }
            }

            if (lamp.damage >= 0.995f) {
                BreakRuntimeLamp(lamp);
            }
        }
    }

    void SpawnSteamBurst(const SteamEmitter& emitter, float intensity = 1.0f) {
        int count = static_cast<int>(RandRange(12.0f, 26.0f) * intensity);
        for (int i = 0; i < count && steam_.size() < 420; ++i) {
            float side = RandRange(-0.42f, 0.42f);
            float rise = RandRange(-0.08f, 0.18f);
            XMFLOAT3 right{emitter.dir.z, 0.0f, -emitter.dir.x};
            SteamParticle sp{};
            sp.pos = Add3(emitter.pos, Add3(Scale3(right, side), {0.0f, rise, 0.0f}));
            float spread = RandRange(-0.32f, 0.32f);
            sp.vel = {
                emitter.dir.x * RandRange(0.55f, 1.25f) * intensity + right.x * spread,
                RandRange(0.16f, 0.62f) * intensity,
                emitter.dir.z * RandRange(0.55f, 1.25f) * intensity + right.z * spread
            };
            sp.age = 0.0f;
            sp.life = RandRange(1.0f, 2.4f) * (0.85f + intensity * 0.18f);
            sp.size = RandRange(0.22f, 0.52f) * (0.85f + intensity * 0.25f);
            steam_.push_back(sp);
        }
    }

    bool SpawnVentDrop(const SteamEmitter& emitter) {
        if (ventDrops_.size() >= kMaxVentDrops) return false;
        VentDrop d{};
        d.pos = emitter.pos;
        d.vel = {emitter.dir.x * RandRange(0.45f, 0.95f), RandRange(0.22f, 0.75f), emitter.dir.z * RandRange(0.45f, 0.95f)};
        d.yaw = std::atan2(emitter.dir.x, emitter.dir.z) + RandRange(-0.4f, 0.4f);
        d.roll = RandRange(-0.35f, 0.35f);
        d.angular = RandRange(-4.8f, 4.8f);
        d.life = RandRange(2.5f, 4.6f);
        ventDrops_.push_back(d);
        return true;
    }

    void BeginVentReaction(const SteamEmitter& emitter, float sensory) {
        if (IsThreatVisible() || ChasePanicActive()) return;
        ventReactionTarget_ = emitter.pos;
        ventReactionAway_ = Normalize3({camera_.x - emitter.pos.x, 0.0f, camera_.z - emitter.pos.z}, emitter.dir);
        ventReactionDuration_ = RandRange(1.35f, 1.85f) * Lerp(0.90f, 1.12f, Clamp01(sensory));
        ventReactionLookDelay_ = RandRange(0.14f, 0.38f) * Lerp(1.08f, 0.82f, Clamp01(sensory));
        ventReactionBackDuration_ = RandRange(0.58f, 0.96f);
        ventReactionScanSeed_ = RandRange(0.0f, kPi * 2.0f);
        ventReactionTimer_ = ventReactionDuration_;
        stopTimer_ = 0.0f;
        headScanTimer_ = 0.0f;
        junctionScanActive_ = false;
        lookBack_ = false;
        propLookTimer_ = 0.0f;
        bloodFocusTimer_ = 0.0f;
        flashlightAgitation_ = std::max(flashlightAgitation_, 0.55f + sensory * 0.30f);
    }

    void UpdateScareEvents(float dt) {
        scareCooldown_ = std::max(0.0f, scareCooldown_ - dt);
        fleshFlickerTimer_ = std::max(0.0f, fleshFlickerTimer_ - dt);
        bloodWorldFlickerTimer_ = std::max(0.0f, bloodWorldFlickerTimer_ - dt);
        float scareFrequency = JumpscareFrequency();
        float scareScale = ScareCooldownScale();
        if (scareFrequency <= 0.001f) {
            fleshFlickerTimer_ = 0.0f;
            fleshFlickerCooldown_ = 1000000.0f;
            bloodWorldFlickerTimer_ = 0.0f;
            bloodWorldFlickerCooldown_ = 1000000.0f;
            return;
        }
        if (IsThreatVisible() || ChasePanicActive()) {
            scareCooldown_ = std::max(scareCooldown_, 0.80f);
            ventReactionTimer_ = 0.0f;
            return;
        }
        if (settings_.fleshFlicker) {
            fleshFlickerCooldown_ = std::max(0.0f, fleshFlickerCooldown_ - dt);
            if (fleshFlickerCooldown_ <= 0.0f && fleshFlickerTimer_ <= 0.0f && scareCooldown_ <= 0.0f) {
                fleshFlickerDuration_ = RandRange(settings_.fleshFlickerDuration * 0.82f, settings_.fleshFlickerDuration * 1.24f);
                fleshFlickerTimer_ = fleshFlickerDuration_;
                fleshFlickerCooldown_ = RandRange(settings_.fleshFlickerMinSeconds, settings_.fleshFlickerMaxSeconds) * scareScale;
                scareCooldown_ = std::max(scareCooldown_, fleshFlickerDuration_ + RandRange(8.0f, 18.0f) * scareScale);
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.62f);
                AddDread(settings_.dreadFleshGain);
            }
        } else {
            fleshFlickerTimer_ = 0.0f;
            fleshFlickerCooldown_ = 1000000.0f;
        }
        if (settings_.bloodWorldFlicker && settings_.bloodWorldCoverage > 0.001f) {
            bloodWorldFlickerCooldown_ = std::max(0.0f, bloodWorldFlickerCooldown_ - dt);
            if (bloodWorldActivationTime_ < -900.0f &&
                bloodWorldFlickerCooldown_ <= 0.0f && bloodWorldFlickerTimer_ <= 0.0f && scareCooldown_ <= 0.0f) {
                bloodWorldFlickerDuration_ = RandRange(settings_.bloodWorldFlickerDuration * 0.82f, settings_.bloodWorldFlickerDuration * 1.24f);
                bloodWorldFlickerTimer_ = bloodWorldFlickerDuration_;
                bloodWorldActivationTime_ = time_;
                bloodWorldFlickerCooldown_ = RandRange(settings_.bloodWorldFlickerMinSeconds, settings_.bloodWorldFlickerMaxSeconds) * scareScale;
                scareCooldown_ = std::max(scareCooldown_, bloodWorldFlickerDuration_ + RandRange(9.0f, 20.0f) * scareScale);
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.72f);
                AddDread(std::max(settings_.dreadJumpscareGain * 0.90f, 0.30f));
            }
        } else {
            bloodWorldFlickerTimer_ = 0.0f;
            bloodWorldFlickerCooldown_ = 1000000.0f;
        }
        if (deathActive_ || exitTransitionActive_) return;

        Tile currentTile = CameraTile();
        bool enteredTile = !(currentTile == scareEventTile_);
        if (enteredTile) {
            scareEventTile_ = currentTile;
        }

        for (SparkEmitter& emitter : sparkEmitters_) {
            if (emitter.triggered) continue;
            if (!ScareSourceAhead(emitter.pos,
                maze_.TileMinimum() * 0.78f,
                maze_.TileAverage() * 2.65f,
                4,
                0.10f)) continue;
            float sensory = std::max(ScareSensoryWeight(emitter.pos, 8.5f, 0.80f, 2.35f), 0.72f);
            if (scareCooldown_ <= 0.0f && sensory > 0.0f) {
                emitter.triggered = true;
                float intensity = PickBrokenLampSparkIntensity();
                AlertMonsterToPlayerTrigger(emitter.pos);
                SpawnSparkBurst(emitter, intensity);
                ScheduleSparkChain(emitter.pos, intensity * settings_.effectBrokenLampChainIntensityScale, PickBrokenLampChainBursts());
                scareCooldown_ = RandRange(9.0f, 18.0f) * scareScale;
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.42f + sensory * 0.43f);
                AddDread(settings_.dreadJumpscareGain *
                    Clamp01(intensity / std::max(0.1f, settings_.effectBrokenLampSparkIntensityMax)) * sensory);
                if (sensory > 0.55f) {
                    stumbleTimer_ = std::max(stumbleTimer_, 0.12f + sensory * 0.06f);
                    stumbleDuration_ = std::max(stumbleDuration_, 0.16f + sensory * 0.06f);
                    stumbleYawOffset_ = RandRange(-0.18f, 0.18f) * sensory;
                }
            }
        }

        for (SteamEmitter& emitter : steamEmitters_) {
            if (emitter.triggered) continue;
            if (!ScareSourceAhead(emitter.pos,
                maze_.TileMinimum() * 0.82f,
                maze_.TileAverage() * 2.80f,
                4,
                0.08f)) continue;
            float sensory = std::max(ScareSensoryWeight(emitter.pos, 7.8f, 0.76f, 2.10f), 0.70f);
            if (sensory > 0.0f && scareCooldown_ <= 0.0f) {
                emitter.triggered = true;
                AlertMonsterToPlayerTrigger(emitter.pos);
                SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                if (!emitter.panelDropped &&
                    RandRange(0.0f, 1.0f) < settings_.effectAirVentPanelDropChance &&
                    SpawnVentDrop(emitter)) {
                    emitter.panelDropped = true;
                    flashlightAgitation_ = std::max(flashlightAgitation_, 0.34f + sensory * 0.41f);
                    AddDread(settings_.dreadJumpscareGain * 0.78f * sensory);
                }
                scareCooldown_ = RandRange(10.0f, 21.0f) * scareScale;
                AddDread(settings_.dreadJumpscareGain * 0.62f * sensory);
                flashlightAgitation_ = std::max(flashlightAgitation_, 0.28f + sensory * 0.36f);
                BeginVentReaction(emitter, sensory);
            }
        }
    }

    void UpdateSparks(float dt) {
        if (!settings_.sparkParticles) {
            sparks_.clear();
            sparkFlashes_.clear();
            sparkChains_.clear();
            return;
        }
        for (SparkChain& chain : sparkChains_) {
            chain.timer -= dt;
            while (chain.remaining > 0 && chain.timer <= 0.0f) {
                EmitSparkBurstAt(chain.pos, chain.intensity * RandRange(0.68f, 1.18f));
                --chain.remaining;
                chain.timer += RandRange(0.075f, 0.22f);
            }
        }
        sparkChains_.erase(std::remove_if(sparkChains_.begin(), sparkChains_.end(), [](const SparkChain& chain) {
            return chain.remaining <= 0;
        }), sparkChains_.end());

        sparkCooldown_ = 1000000.0f;

        for (SparkParticle& sp : sparks_) {
            sp.age += dt;
            sp.vel.y -= 3.8f * dt;
            sp.pos.x += sp.vel.x * dt;
            sp.pos.y += sp.vel.y * dt;
            sp.pos.z += sp.vel.z * dt;
            if (sp.pos.y < 0.055f && sp.vel.y < 0.0f) {
                sp.pos.y = 0.055f;
                sp.vel.y = -sp.vel.y * 0.34f;
                sp.vel.x *= 0.58f;
                sp.vel.z *= 0.58f;
                if (std::abs(sp.vel.y) < 0.08f) sp.age = sp.life;
            }
        }
        sparks_.erase(std::remove_if(sparks_.begin(), sparks_.end(), [](const SparkParticle& sp) {
            return sp.age >= sp.life;
        }), sparks_.end());

        for (SparkFlash& flash : sparkFlashes_) {
            flash.age += dt;
        }
        sparkFlashes_.erase(std::remove_if(sparkFlashes_.begin(), sparkFlashes_.end(), [](const SparkFlash& flash) {
            return flash.age >= flash.life;
        }), sparkFlashes_.end());
    }

    void UpdateSteamAndDrops(float dt) {
        for (SteamParticle& sp : steam_) {
            sp.age += dt;
            sp.vel.y += 0.18f * dt;
            sp.vel.x *= std::max(0.0f, 1.0f - dt * 0.42f);
            sp.vel.z *= std::max(0.0f, 1.0f - dt * 0.42f);
            sp.pos.x += sp.vel.x * dt;
            sp.pos.y += sp.vel.y * dt;
            sp.pos.z += sp.vel.z * dt;
        }
        steam_.erase(std::remove_if(steam_.begin(), steam_.end(), [](const SteamParticle& sp) {
            return sp.age >= sp.life;
        }), steam_.end());

        for (VentDrop& d : ventDrops_) {
            d.age += dt;
            if (!d.landed) {
                d.vel.y -= 4.6f * dt;
                d.pos.x += d.vel.x * dt;
                d.pos.y += d.vel.y * dt;
                d.pos.z += d.vel.z * dt;
                d.roll += d.angular * dt;
                if (d.pos.y < kVentDropFloorY || d.age >= d.life) {
                    d.pos.y = kVentDropFloorY;
                    d.vel.y = -d.vel.y * 0.18f;
                    d.vel.x *= 0.38f;
                    d.vel.z *= 0.38f;
                    d.angular *= 0.28f;
                    if (std::abs(d.vel.y) < 0.16f || d.age >= d.life) {
                        d.landed = true;
                        d.vel = {};
                        d.angular = 0.0f;
                        d.roll = kPi * 0.5f;
                        SparkEmitter impact{{d.pos.x, d.pos.y + 0.08f, d.pos.z}, 0.0f};
                        SpawnSparkBurst(impact, 1.6f);
                        AlertMonsterToPlayerTrigger(impact.pos);
                    }
                }
            }
        }
    }

    void UpdateSimulation(float dt) {
        if (deathActive_) {
            UpdateDeath(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        fadeInTimer_ = std::max(0.0f, fadeInTimer_ - dt);
        if (monsterPreview_) {
            UpdateMonsterHeadAnimation(dt, false);
            SetMonsterPreviewCamera(time_);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            UpdateDebugSliceLoop(dt);
            if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
                UpdateSteamAndDrops(dt);
            } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
                UpdateSparks(dt);
            }
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (settings_.bloodStudyView) {
            ApplyBloodStudyCamera();
            bloodWorldActivationTime_ = time_ - 46.0f;
            UpdateSparks(dt);
            UpdateSteamAndDrops(dt);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }
        UpdateScareEvents(dt);
        UpdateSparks(dt);
        UpdateSteamAndDrops(dt);
        if (exitTransitionActive_) {
            UpdateExitTransition(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        if (VisibleInFront(maze_.exit)) exitSpotted_ = true;
        UpdateMonster(dt);
        UpdateMonsterLampDamage(dt);
        float monsterDist = MonsterDistance();
        if (monsterDist < settings_.monsterKillDistance && maze_.LineClear(CameraTile(), MonsterTile())) {
            playerHealth_ = 0.0f;
            BeginDeath();
            UpdateDeath(dt);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }

        bool threat = IsThreatVisible();
        UpdateChasePanic(dt, threat, monsterDist);
        bool panicActive = threat || ChasePanicActive();
        float dangerTarget = threat ? Clamp01((8.0f - monsterDist) / 6.6f) : chasePanic_ * 0.38f;
        dangerLevel_ += (dangerTarget - dangerLevel_) * std::min(1.0f, dt * (dangerTarget > dangerLevel_ ? 2.2f : 0.85f));
        UpdateDread(dt, threat, monsterDist);
        UpdateMonsterSightDread(dt, threat, monsterDist);
        UpdateBloodDread(dt);
        UpdateMonsterProximityBlood(dt);
        UpdateDreadMeterDisplay(dt);

        if (threat && !MonsterSightingFreezeActive()) {
            stopTimer_ = 0.0f;
            headScanTimer_ = 0.0f;
            headScanDuration_ = 0.0f;
            lookBack_ = false;
            junctionScanActive_ = false;
            branchLookTimer_ = 0.0f;
            roomSurveyTimer_ = 0.0f;
            propLookTimer_ = 0.0f;
            bloodFocusTimer_ = 0.0f;
            ventReactionTimer_ = 0.0f;
            threatRepath_ -= dt;
            if (threatRepath_ <= 0.0f || pathIndex_ >= path_.size()) {
                ChoosePath(true);
                bool runningToExit = !path_.empty() && path_.back() == maze_.exit;
                bool committedEscape = FirstThreatLineBreakIndex(path_, MonsterTile(), 7) >= 0 || FirstBranchIndex(path_, 6) >= 0;
                threatRepath_ = runningToExit ? RandRange(3.4f, 5.4f) : (committedEscape ? RandRange(2.6f, 4.2f) : RandRange(1.2f, 2.2f));
            }
        } else if (threat) {
            threatRepath_ = 0.0f;
            path_.clear();
            pathIndex_ = 0;
        } else if (panicActive) {
            threatRepath_ = 0.0f;
            chaseLookBackTimer_ = std::max(0.0f, chaseLookBackTimer_ - dt * 1.20f);
            chaseLookBackCooldown_ = std::max(0.0f, chaseLookBackCooldown_ - dt);
            stumbleTimer_ = std::max(0.0f, stumbleTimer_ - dt * 1.10f);
        } else {
            threatRepath_ = 0.0f;
            chaseLookBackTimer_ = 0.0f;
            chaseLookBackCooldown_ = std::max(0.0f, chaseLookBackCooldown_ - dt);
            stumbleTimer_ = std::max(0.0f, stumbleTimer_ - dt * 2.0f);
        }
        if (runtimeMode_ == RendererRuntimeMode::PlayableGame) {
            UpdateManualPlayer(dt);
        } else {
            UpdatePathFollower(dt);
        }
        UpdateFlashlightAim(dt);
        UpdateAirParticles(dt);
        UpdateAirParticleFocus(dt);
    }

    void AppendDynamicQuad(std::vector<Vertex>& verts,
                           XMFLOAT3 a, XMFLOAT3 b, XMFLOAT3 c, XMFLOAT3 d,
                           XMFLOAT3 normal, XMFLOAT3 tangent, float material) {
        XMFLOAT2 uvs[4] = {{0, 1}, {1, 1}, {1, 0}, {0, 0}};
        XMFLOAT3 pos[4] = {a, b, c, d};
        int order[6] = {0, 1, 2, 0, 2, 3};
        for (int i = 0; i < 6; ++i) {
            int idx = order[i];
            verts.push_back({pos[idx], normal, tangent, uvs[idx], material});
        }
    }

    void AppendDynamicBoxAxes(std::vector<Vertex>& verts, XMFLOAT3 center,
                              XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                              XMFLOAT3 half, float material) {
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        auto p = [&](float x, float y, float z) {
            return Add3(center, Add3(Scale3(right, x * half.x), Add3(Scale3(up, y * half.y), Scale3(forward, z * half.z))));
        };
        AppendDynamicQuad(verts, p(-1, -1,  1), p( 1, -1,  1), p( 1,  1,  1), p(-1,  1,  1), forward, right, material);
        AppendDynamicQuad(verts, p( 1, -1, -1), p(-1, -1, -1), p(-1,  1, -1), p( 1,  1, -1), Scale3(forward, -1.0f), Scale3(right, -1.0f), material);
        AppendDynamicQuad(verts, p( 1, -1,  1), p( 1, -1, -1), p( 1,  1, -1), p( 1,  1,  1), right, Scale3(forward, -1.0f), material);
        AppendDynamicQuad(verts, p(-1, -1, -1), p(-1, -1,  1), p(-1,  1,  1), p(-1,  1, -1), Scale3(right, -1.0f), forward, material);
        AppendDynamicQuad(verts, p(-1,  1,  1), p( 1,  1,  1), p( 1,  1, -1), p(-1,  1, -1), up, right, material);
        AppendDynamicQuad(verts, p(-1, -1, -1), p( 1, -1, -1), p( 1, -1,  1), p(-1, -1,  1), Scale3(up, -1.0f), right, material);
    }

    void AppendDynamicBox(std::vector<Vertex>& verts, XMFLOAT3 center, XMFLOAT3 half, float yaw, float material) {
        XMFLOAT3 right{std::cos(yaw), 0.0f, -std::sin(yaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(yaw), 0.0f, std::cos(yaw)};
        AppendDynamicBoxAxes(verts, center, right, up, forward, half, material);
    }

    void AppendSegmentBox(std::vector<Vertex>& verts, XMFLOAT3 a, XMFLOAT3 b,
                          float halfWidth, float halfDepth, float material) {
        XMFLOAT3 axis = Sub3(b, a);
        float len = Length3(axis);
        if (len <= 0.001f) return;
        XMFLOAT3 upAxis = Scale3(axis, 1.0f / len);
        XMFLOAT3 ref{0.0f, 1.0f, 0.0f};
        if (std::abs(Dot3(upAxis, ref)) > 0.88f) ref = {1.0f, 0.0f, 0.0f};
        XMFLOAT3 right = Normalize3(Cross3(ref, upAxis), {1.0f, 0.0f, 0.0f});
        XMFLOAT3 forward = Normalize3(Cross3(upAxis, right), {0.0f, 0.0f, 1.0f});
        AppendDynamicBoxAxes(verts, Lerp3(a, b, 0.5f), right, upAxis, forward, {halfWidth, len * 0.5f, halfDepth}, material);
    }

    void AppendDynamicEllipsoid(std::vector<Vertex>& verts, XMFLOAT3 center,
                                XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                                XMFLOAT3 radius, int slices, int stacks, float material) {
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        slices = std::clamp(slices, 6, 32);
        stacks = std::clamp(stacks, 4, 20);

        struct P {
            XMFLOAT3 pos;
            XMFLOAT3 normal;
            XMFLOAT3 tangent;
            XMFLOAT2 uv;
        };
        auto point = [&](int sx, int sy) {
            float u = static_cast<float>(sx) / static_cast<float>(slices);
            float v = static_cast<float>(sy) / static_cast<float>(stacks);
            float theta = u * kPi * 2.0f;
            float phi = -kPi * 0.5f + v * kPi;
            float cp = std::cos(phi);
            float sp = std::sin(phi);
            float ct = std::cos(theta);
            float st = std::sin(theta);

            XMFLOAT3 pos = Add3(center, Add3(Scale3(right, ct * cp * radius.x),
                Add3(Scale3(up, sp * radius.y), Scale3(forward, st * cp * radius.z))));
            XMFLOAT3 n = Normalize3(Add3(Scale3(right, ct * cp / std::max(0.001f, radius.x)),
                Add3(Scale3(up, sp / std::max(0.001f, radius.y)),
                     Scale3(forward, st * cp / std::max(0.001f, radius.z)))), up);
            XMFLOAT3 t = Normalize3(Add3(Scale3(right, -st), Scale3(forward, ct)), right);
            return P{pos, n, t, {u, v}};
        };
        auto tri = [&](const P& a, const P& b, const P& c) {
            verts.push_back({a.pos, a.normal, a.tangent, a.uv, material});
            verts.push_back({b.pos, b.normal, b.tangent, b.uv, material});
            verts.push_back({c.pos, c.normal, c.tangent, c.uv, material});
        };

        for (int y = 0; y < stacks; ++y) {
            for (int x = 0; x < slices; ++x) {
                P a = point(x, y);
                P b = point(x + 1, y);
                P c = point(x + 1, y + 1);
                P d = point(x, y + 1);
                tri(a, b, c);
                tri(a, c, d);
            }
        }
    }

    XMFLOAT3 ActiveSkullRotationDegrees() const {
        return monsterUsingAltSkull_
            ? XMFLOAT3{settings_.monsterAltSkullPitchDegrees, settings_.monsterAltSkullYawDegrees, settings_.monsterAltSkullRollDegrees}
            : XMFLOAT3{settings_.monsterSkullPitchDegrees, settings_.monsterSkullYawDegrees, settings_.monsterSkullRollDegrees};
    }

    XMFLOAT3 RotateSkullLocalVector(XMFLOAT3 v) const {
        XMFLOAT3 degrees = ActiveSkullRotationDegrees();
        float pitch = degrees.x * kPi / 180.0f;
        float yaw = degrees.y * kPi / 180.0f;
        float roll = degrees.z * kPi / 180.0f;

        float cy = std::cos(yaw);
        float sy = std::sin(yaw);
        v = {v.x * cy + v.z * sy, v.y, -v.x * sy + v.z * cy};

        float cp = std::cos(pitch);
        float sp = std::sin(pitch);
        v = {v.x, v.y * cp - v.z * sp, v.y * sp + v.z * cp};

        float cr = std::cos(roll);
        float sr = std::sin(roll);
        v = {v.x * cr - v.y * sr, v.x * sr + v.y * cr, v.z};
        return v;
    }

    bool AppendExternalSkullMesh(std::vector<Vertex>& verts, XMFLOAT3 center,
                                 XMFLOAT3 right, XMFLOAT3 up, XMFLOAT3 forward,
                                 float modelXZ, float modelY) {
        if (skullMesh_.empty()) return false;
        if (verts.size() + skullMesh_.size() + 512 > kDynamicVertexCapacity) return false;
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});
        forward = Normalize3(forward, {0.0f, 0.0f, 1.0f});
        for (const Vertex& src : skullMesh_) {
            XMFLOAT3 local = RotateSkullLocalVector({-src.pos.x, src.pos.y, -src.pos.z});
            XMFLOAT3 nLocal = RotateSkullLocalVector({-src.normal.x, src.normal.y, -src.normal.z});
            XMFLOAT3 tLocal = RotateSkullLocalVector({-src.tangent.x, src.tangent.y, -src.tangent.z});
            XMFLOAT3 pos = Add3(center, Add3(Scale3(right, local.x * modelXZ),
                Add3(Scale3(up, local.y * modelY), Scale3(forward, local.z * modelXZ))));
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, nLocal.x),
                Add3(Scale3(up, nLocal.y), Scale3(forward, nLocal.z))), forward);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, tLocal.x),
                Add3(Scale3(up, tLocal.y), Scale3(forward, tLocal.z))), right);
            verts.push_back({pos, normal, tangent, src.uv, src.material});
        }
        return true;
    }

    void AppendDynamicDoor(std::vector<Vertex>& verts) {
        float halfW = 0.55f;
        float halfH = 1.10f;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        float angle = exitDoorAngle_;
        XMFLOAT3 right = RotateYVec(exitDoorRight_, angle);
        XMFLOAT3 normal = RotateYVec(exitDoorNormal_, angle);
        XMFLOAT3 center = Add3(exitDoorHinge_, Add3(Scale3(right, halfW), Scale3(normal, 0.018f)));
        AppendDynamicBoxAxes(verts, center, right, up, normal, {halfW, halfH, 0.026f}, 6.0f);

        XMFLOAT3 knobCenter = Add3(center, OrientedOffset(right, up, normal, halfW * 0.63f, -0.08f, 0.070f));
        XMFLOAT3 kr = Scale3(right, 0.050f);
        XMFLOAT3 ku = Scale3(up, 0.050f);
        AppendDynamicQuad(verts,
            Add3(knobCenter, Add3(Scale3(kr, -1.0f), Scale3(ku, -1.0f))),
            Add3(knobCenter, Add3(kr, Scale3(ku, -1.0f))),
            Add3(knobCenter, Add3(kr, ku)),
            Add3(knobCenter, Add3(Scale3(kr, -1.0f), ku)),
            normal, right, 10.0f);
    }

    void AppendMonsterBillboard(std::vector<Vertex>& solidVerts, std::vector<Vertex>& transparentVerts) {
        float modelY = std::clamp(settings_.monsterScale, 0.35f, 1.25f);
        float modelXZ = std::clamp(settings_.monsterScale, 0.35f, 1.35f);
        float dist = MonsterDistance();
        bool canTrackPlayer = !monsterPreview_ && MonsterLineOfSightToPlayer();
        float faceYaw = monsterYaw_;
        if (canTrackPlayer) {
            float cameraYaw = std::atan2(camera_.x - monster_.x, camera_.z - monster_.z);
            faceYaw += AngleWrap(cameraYaw - faceYaw) * 0.42f;
        }

        XMFLOAT3 right{std::cos(faceYaw), 0.0f, -std::sin(faceYaw)};
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        XMFLOAT3 forward{std::sin(faceYaw), 0.0f, std::cos(faceYaw)};
        float hover = 0.22f + std::sin(time_ * 1.55f + monster_.x * 0.07f + monster_.z * 0.05f) * 0.050f;
        auto off = [&](float x, float y, float z) {
            return Add3(monster_, OrientedOffset(right, up, forward, x * modelXZ, y * modelY + hover, z * modelXZ));
        };
        auto box = [&](float x, float y, float z, float hx, float hy, float hz, float material) {
            AppendDynamicBoxAxes(solidVerts, off(x, y, z), right, up, forward,
                {hx * modelXZ, hy * modelY, hz * modelXZ}, material);
        };
        auto seg = [&](XMFLOAT3 a, XMFLOAT3 b, float w, float d, float material) {
            AppendSegmentBox(solidVerts, a, b, w * modelXZ, d * modelXZ, material);
        };

        float twitch = std::sin(time_ * 13.7f + monster_.x * 0.3f) * 0.035f;
        float breathe = std::sin(time_ * 2.3f) * 0.030f;
        float deathHeadLock = deathActive_ ? SmoothStep(0.0f, 0.22f, deathTimer_) : 0.0f;
        constexpr float boneMat = 9.65f;
        constexpr float darkMat = 10.0f;

        auto smokeMaterial = [&](float seed) {
            return 11.08f + std::fmod(std::abs(seed), 0.34f);
        };
        auto smokeBand = [&](XMFLOAT3 a, XMFLOAT3 b, float halfA, float halfB, float material) {
            XMFLOAT3 axis = Sub3(b, a);
            float len = Length3(axis);
            if (len <= 0.001f) return;
            XMFLOAT3 axisN = Scale3(axis, 1.0f / len);
            XMFLOAT3 mid = Lerp3(a, b, 0.5f);
            XMFLOAT3 toCam = Normalize3(Sub3(camera_, mid), forward);
            XMFLOAT3 side = Normalize3(Cross3(axisN, toCam), right);
            XMFLOAT3 normal = Normalize3(Cross3(side, axisN), toCam);
            AppendDynamicQuad(transparentVerts,
                Add3(a, Scale3(side, -halfA * modelXZ)),
                Add3(a, Scale3(side, halfA * modelXZ)),
                Add3(b, Scale3(side, halfB * modelXZ)),
                Add3(b, Scale3(side, -halfB * modelXZ)),
                normal, side, material);
        };
        auto smokePuff = [&](float x, float y, float z, float halfW, float halfH, float material) {
            XMFLOAT3 center = off(x, y, z);
            XMFLOAT3 toCam = Normalize3(Sub3(camera_, center), forward);
            XMFLOAT3 side = Normalize3(Cross3(up, toCam), right);
            XMFLOAT3 puffUp = Normalize3(Cross3(toCam, side), up);
            XMFLOAT3 hw = Scale3(side, halfW * modelXZ);
            XMFLOAT3 hh = Scale3(puffUp, halfH * modelY);
            AppendDynamicQuad(transparentVerts,
                Add3(center, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(center, Add3(hw, Scale3(hh, -1.0f))),
                Add3(center, Add3(hw, hh)),
                Add3(center, Add3(Scale3(hw, -1.0f), hh)),
                toCam, side, material);
        };

        AppendDynamicEllipsoid(solidVerts, off(0.0f, 0.88f + breathe * 0.35f, kMonsterSmokeBackOffset * 0.34f),
            right, up, forward, {0.255f * modelXZ, 0.760f * modelY, 0.215f * modelXZ}, 18, 12, darkMat);
        AppendDynamicEllipsoid(solidVerts, off(0.0f, 1.26f + breathe * 0.25f, kMonsterSmokeBackOffset * 0.18f),
            right, up, forward, {0.205f * modelXZ, 0.530f * modelY, 0.178f * modelXZ}, 16, 10, darkMat);
        AppendDynamicEllipsoid(solidVerts, off(0.0f, 0.42f, kMonsterSmokeBackOffset * 0.58f),
            right, up, forward, {0.315f * modelXZ, 0.390f * modelY, 0.255f * modelXZ}, 16, 9, darkMat);

        constexpr int monsterSmokePuffs = 220;
        for (int i = 0; i < monsterSmokePuffs; ++i) {
            float fi = static_cast<float>(i);
            float a = fi * 2.399963f + std::sin(time_ * (0.31f + fi * 0.017f)) * 0.34f;
            float layer = std::fmod(fi * 0.618034f, 1.0f);
            float column = std::fmod(fi * 0.381966f, 1.0f) - 0.5f;
            float lower = 1.0f - layer;
            float y = 0.05f + layer * 1.64f + std::sin(time_ * (0.74f + fi * 0.023f) + fi) * (0.060f + lower * 0.048f);
            float radius = 0.06f + 0.30f * (1.0f - std::abs(layer - 0.52f)) + lower * 0.28f;
            float x = std::cos(a) * radius + std::sin(time_ * 0.47f + fi) * 0.045f;
            float z = std::sin(a) * radius + kMonsterSmokeBackOffset * 0.34f + column * 0.20f +
                std::cos(time_ * 0.39f + fi * 1.7f) * 0.045f;
            float pulse = 0.88f + std::sin(time_ * (0.68f + fi * 0.031f) + fi * 3.1f) * 0.12f;
            float lowerScale = Lerp(1.04f, 2.55f, std::pow(lower, 1.30f));
            smokePuff(x, y, z,
                (0.166f + 0.104f * (1.0f - std::abs(layer - 0.50f))) * pulse * lowerScale,
                (0.156f + 0.122f * (1.0f - std::abs(layer - 0.45f))) * pulse * lowerScale,
                smokeMaterial(1.07f + fi * 0.043f));
        }

        const int capeStrips = 0;
        for (int i = 0; i < capeStrips; ++i) {
            float a0 = (static_cast<float>(i) / static_cast<float>(capeStrips)) * kPi * 2.0f;
            float a1 = (static_cast<float>(i + 1) / static_cast<float>(capeStrips)) * kPi * 2.0f;
            float wave0 = std::sin(time_ * 2.6f + a0 * 2.0f + monster_.x * 0.11f) * 0.035f;
            float wave1 = std::sin(time_ * 2.6f + a1 * 2.0f + monster_.z * 0.09f) * 0.035f;
            float topR0 = 0.33f + wave0 * 0.60f;
            float topR1 = 0.33f + wave1 * 0.60f;
            float midR0 = 0.50f + wave0 * 1.35f;
            float midR1 = 0.50f + wave1 * 1.35f;
            float botR0 = 0.40f + wave0 * 1.15f;
            float botR1 = 0.40f + wave1 * 1.15f;
            float torn0 = std::sin(static_cast<float>(i) * 2.91f + monster_.x) * 0.055f;
            float torn1 = std::sin(static_cast<float>(i + 1) * 2.91f + monster_.z) * 0.055f;

            XMFLOAT3 pTop0 = off(std::cos(a0) * topR0, 1.52f + breathe * 0.45f, std::sin(a0) * topR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pTop1 = off(std::cos(a1) * topR1, 1.52f + breathe * 0.45f, std::sin(a1) * topR1 + kMonsterSmokeBackOffset);
            XMFLOAT3 pMid0 = off(std::cos(a0) * midR0, 0.86f + wave0 * 0.55f + breathe, std::sin(a0) * midR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pMid1 = off(std::cos(a1) * midR1, 0.86f + wave1 * 0.55f + breathe, std::sin(a1) * midR1 + kMonsterSmokeBackOffset);
            XMFLOAT3 pBot0 = off(std::cos(a0) * botR0, 0.20f + torn0, std::sin(a0) * botR0 + kMonsterSmokeBackOffset);
            XMFLOAT3 pBot1 = off(std::cos(a1) * botR1, 0.20f + torn1, std::sin(a1) * botR1 + kMonsterSmokeBackOffset);
            float amid = (a0 + a1) * 0.5f;
            XMFLOAT3 normal = Normalize3(Add3(Scale3(right, std::cos(amid)), Scale3(forward, std::sin(amid))), forward);
            XMFLOAT3 tangent = Normalize3(Add3(Scale3(right, -std::sin(amid)), Scale3(forward, std::cos(amid))), right);
            float material = smokeMaterial(static_cast<float>(i) * 0.037f + monster_.x * 0.021f + monster_.z * 0.013f);
            AppendDynamicQuad(transparentVerts, pBot0, pBot1, pMid1, pMid0, normal, tangent, material);
            AppendDynamicQuad(transparentVerts, pMid0, pMid1, pTop1, pTop0, normal, tangent, material + 0.017f);
        }
        for (int i = 0; i < 0; ++i) {
            float a = static_cast<float>(i) / 7.0f * kPi * 2.0f + std::sin(time_ * 0.43f + i) * 0.22f;
            float wobble = std::sin(time_ * 1.1f + i * 2.3f) * 0.06f;
            XMFLOAT3 a0 = off(std::cos(a) * (0.10f + wobble), 1.42f + breathe, std::sin(a) * (0.10f + wobble) + kMonsterSmokeBackOffset);
            XMFLOAT3 a1 = off(std::cos(a + 0.38f) * (0.56f + wobble), 0.56f, std::sin(a + 0.38f) * (0.56f + wobble) + kMonsterSmokeBackOffset);
            XMFLOAT3 a2 = off(std::cos(a + 0.74f) * (0.86f + wobble), 0.03f, std::sin(a + 0.74f) * (0.86f + wobble) + kMonsterSmokeBackOffset);
            smokeBand(a0, a1, 0.09f, 0.20f, smokeMaterial(0.41f + i * 0.061f));
            smokeBand(a1, a2, 0.14f, 0.27f, smokeMaterial(0.63f + i * 0.053f));
        }
        // Keep the body as overlapping volumetric puffs; hard bands read as cards in preview.

        float liveLock = canTrackPlayer ? monsterHeadLockAmount_ : 0.0f;
        float headLock = std::max(deathHeadLock, liveLock);
        float scanWeight = 1.0f - Clamp01(headLock);
        float headYaw = faceYaw + monsterHeadYawOffset_ * scanWeight +
            twitch * (1.0f - deathHeadLock * 0.85f) * scanWeight;
        XMFLOAT3 hRight{std::cos(headYaw), 0.0f, -std::sin(headYaw)};
        XMFLOAT3 hUp = up;
        XMFLOAT3 hForward{std::sin(headYaw), 0.0f, std::cos(headYaw)};
        XMFLOAT3 skull = off(0.0f, 2.00f + MonsterHeadBobOffset(), kMonsterHeadForwardOffset);
        float headPitch = monsterHeadPitchOffset_ * scanWeight;
        if (std::abs(headPitch) > 0.0005f) {
            hForward = Normalize3(Add3(Scale3(hForward, std::cos(headPitch)), Scale3(hUp, std::sin(headPitch))), hForward);
            hUp = Normalize3(Cross3(hForward, hRight), hUp);
        }
        if (headLock > 0.001f) {
            XMFLOAT3 cameraFocus{camera_.x, camera_.y + 0.04f, camera_.z};
            XMFLOAT3 lookForward = Normalize3(Sub3(cameraFocus, skull), hForward);
            hForward = Normalize3(Lerp3(hForward, lookForward, Clamp01(headLock)), lookForward);
            hRight = Normalize3(Cross3(up, hForward), hRight);
            hUp = Normalize3(Cross3(hForward, hRight), up);
        }
        bool externalSkull = AppendExternalSkullMesh(solidVerts, skull, hRight, hUp, hForward, modelXZ, modelY);
        if (!externalSkull) {
            AppendDynamicEllipsoid(solidVerts, skull, hRight, hUp, hForward,
                {0.178f * modelXZ, 0.162f * modelY, 0.145f * modelXZ}, 18, 10, boneMat);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Add3(Scale3(hForward, 0.190f * modelXZ), Scale3(hUp, -0.020f * modelY))), hRight, hUp, hForward,
                {0.108f * modelXZ, 0.074f * modelY, 0.196f * modelXZ}, 16, 8, boneMat);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Add3(Scale3(hForward, 0.318f * modelXZ), Scale3(hUp, -0.090f * modelY))), hRight, hUp, hForward,
                {0.126f * modelXZ, 0.040f * modelY, 0.056f * modelXZ}, 14, 6, darkMat);
            AppendDynamicEllipsoid(solidVerts, Add3(skull, Add3(Scale3(hForward, 0.075f * modelXZ), Scale3(hUp, -0.214f * modelY))), hRight, hUp, hForward,
                {0.058f * modelXZ, 0.152f * modelY, 0.050f * modelXZ}, 12, 7, boneMat);
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 cheekA = Add3(skull, OrientedOffset(hRight, hUp, hForward, 0.120f * side * modelXZ, -0.030f * modelY, 0.110f * modelXZ));
                XMFLOAT3 cheekB = Add3(skull, OrientedOffset(hRight, hUp, hForward, 0.215f * side * modelXZ, -0.120f * modelY, 0.220f * modelXZ));
                seg(cheekA, cheekB, 0.012f, 0.010f, boneMat);
            }
        }

        if (!externalSkull) {
            auto hornPoint = [&](float x, float y, float z) {
                return Add3(skull, OrientedOffset(hRight, hUp, hForward, x * modelXZ, y * modelY, z * modelXZ));
            };
            for (int side = -1; side <= 1; side += 2) {
                XMFLOAT3 h0 = hornPoint(0.10f * side, 0.11f, -0.035f);
                XMFLOAT3 h1 = hornPoint(0.30f * side, 0.38f, -0.080f);
                XMFLOAT3 h2 = hornPoint(0.56f * side, 0.55f, -0.120f);
                XMFLOAT3 h3 = hornPoint(0.75f * side, 0.61f, -0.080f);
                seg(h0, h1, 0.028f, 0.023f, boneMat);
                seg(h1, h2, 0.024f, 0.020f, boneMat);
                seg(h2, h3, 0.018f, 0.016f, boneMat);
                seg(h1, hornPoint(0.24f * side, 0.58f, 0.040f), 0.016f, 0.014f, boneMat);
                seg(h2, hornPoint(0.54f * side, 0.76f, -0.020f), 0.014f, 0.012f, boneMat);
                seg(h2, hornPoint(0.68f * side, 0.46f, 0.060f), 0.014f, 0.012f, boneMat);
            }
        }

        auto appendEye = [&](float xOffset, float eyeUp, float eyeForward, float variant) {
            float eyeHalfW = externalSkull ? 0.074f : 0.055f;
            float eyeHalfH = externalSkull ? 0.052f : 0.040f;
            XMFLOAT3 center = Add3(skull, OrientedOffset(hRight, hUp, hForward,
                xOffset * modelXZ, eyeUp * modelY, eyeForward * modelXZ));
            XMFLOAT3 eyeNormal = hForward;
            XMFLOAT3 eyeRight = hRight;
            XMFLOAT3 eyeUpAxis = hUp;
            if (externalSkull) {
                float sideSign = xOffset >= 0.0f ? 1.0f : -1.0f;
                eyeNormal = Normalize3(Add3(Scale3(hRight, sideSign * 0.42f), Scale3(hForward, 0.90f)), hForward);
                eyeRight = Normalize3(Cross3(hUp, eyeNormal), hRight);
                eyeUpAxis = hUp;
                AppendDynamicEllipsoid(solidVerts, center, eyeRight, eyeUpAxis, eyeNormal,
                    {0.045f * modelXZ, 0.034f * modelY, 0.028f * modelXZ}, 12, 7, 10.70f + variant * 0.05f);
            }
            XMFLOAT3 ew = Scale3(eyeRight, eyeHalfW * modelXZ);
            XMFLOAT3 eh = Scale3(eyeUpAxis, eyeHalfH * modelY);
            AppendDynamicQuad(transparentVerts,
                Add3(center, Add3(Scale3(ew, -1.0f), Scale3(eh, -1.0f))),
                Add3(center, Add3(ew, Scale3(eh, -1.0f))),
                Add3(center, Add3(ew, eh)),
                Add3(center, Add3(Scale3(ew, -1.0f), eh)),
                eyeNormal, eyeRight, 12.05f + variant);
        };
        if (externalSkull) {
            appendEye(monsterUsingAltSkull_ ? settings_.monsterAltRightEyeX : settings_.monsterRightEyeX,
                monsterUsingAltSkull_ ? settings_.monsterAltRightEyeY : settings_.monsterRightEyeY,
                monsterUsingAltSkull_ ? settings_.monsterAltRightEyeZ : settings_.monsterRightEyeZ,
                0.14f);
            appendEye(monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeX : settings_.monsterLeftEyeX,
                monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeY : settings_.monsterLeftEyeY,
                monsterUsingAltSkull_ ? settings_.monsterAltLeftEyeZ : settings_.monsterLeftEyeZ,
                0.34f);
        } else {
            appendEye(-0.060f, 0.025f, 0.162f, 0.14f);
            appendEye(0.060f, 0.025f, 0.162f, 0.34f);
        }
    }

    void AppendSparkBillboards(std::vector<Vertex>& verts) {
        XMVECTOR cam = XMLoadFloat3(&camera_);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        for (const SparkParticle& spark : sparks_) {
            float lifeLeft = Clamp01(1.0f - spark.age / std::max(0.001f, spark.life));
            if (lifeLeft <= 0.0f) continue;
            XMVECTOR pos = XMLoadFloat3(&spark.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = spark.size * (0.55f + lifeLeft * lifeLeft * 1.35f);
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * size;
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 13.05f + lifeLeft * 0.38f);
        }
    }

    void AppendAirParticleBillboards(std::vector<Vertex>& verts) {
        if (!settings_.airParticles || airParticles_.empty() || monsterPreview_) return;
        XMFLOAT3 lightOrigin = FlashlightOrigin();
        XMFLOAT3 lightDir = Normalize3(FlashlightForward(), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 cameraForward = Normalize3(DirectionFromYawPitch(yaw_, lookPitch_), {0.0f, 0.0f, 1.0f});
        float maxDist = std::clamp(settings_.flashlightShadowDistanceMeters * 0.70f, 5.5f, 12.0f);
        float coneHalf = std::clamp(settings_.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalf);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalf * 0.50f));
        int maxParticles = std::min<int>(static_cast<int>(airParticles_.size()), std::clamp(static_cast<int>(3400.0f * std::clamp(settings_.airParticleDensity, 0.0f, 4.0f)), 0, 11000));
        int emitted = 0;
        XMFLOAT3 worldUp{0.0f, 1.0f, 0.0f};
        for (const AirParticle& p : airParticles_) {
            if (emitted >= maxParticles) break;
            XMFLOAT3 pos = p.pos;
            XMFLOAT3 fromLight = Sub3(pos, lightOrigin);
            float lightDist = Length3(fromLight);
            if (lightDist < 0.30f || lightDist > maxDist) continue;
            XMFLOAT3 ray = Scale3(fromLight, 1.0f / std::max(0.001f, lightDist));
            float cone = SmoothStep(coneOuter, coneInner, Dot3(ray, lightDir));
            if (cone <= 0.018f) continue;
            XMFLOAT3 fromCamera = Sub3(pos, camera_);
            float cameraDepth = Dot3(fromCamera, cameraForward);
            if (cameraDepth <= 0.035f) continue;
            float focusBlur = Clamp01(Clamp01(std::abs(lightDist - airFocusDistance_) / (0.62f + lightDist * 0.18f)) *
                std::clamp(settings_.airParticleBlur, 0.0f, 3.0f));
            float distanceT = Clamp01(lightDist / maxDist);
            float distanceScale = Lerp(0.52f, 0.085f, distanceT);
            if (p.nearLayer > 0.5f) {
                distanceScale = std::max(distanceScale, p.nearLayer > 1.5f ? 0.92f : 0.72f);
            }
            float size = p.size * distanceScale * (1.0f + focusBlur * 0.24f);
            float projectedPixels = (size / std::max(0.06f, cameraDepth)) * static_cast<float>(std::max<LONG>(1, height_)) * 0.72f;
            if (p.nearLayer < 0.5f && projectedPixels < 0.22f) continue;
            float lifeFade = SmoothStep(0.0f, 2.8f, p.age) * (1.0f - SmoothStep(p.life - 5.2f, p.life, p.age));
            if (lifeFade <= 0.01f) continue;
            size *= Lerp(0.18f, 1.0f, lifeFade);
            XMFLOAT3 toCam = Normalize3(Sub3(camera_, pos), Scale3(lightDir, -1.0f));
            XMFLOAT3 right = Normalize3(Cross3(worldUp, toCam), {1.0f, 0.0f, 0.0f});
            XMFLOAT3 up = Normalize3(Cross3(toCam, right), worldUp);
            float angle = p.angle;
            XMFLOAT3 side = Normalize3(Add3(Scale3(right, std::cos(angle)), Scale3(up, std::sin(angle))), right);
            XMFLOAT3 vertical = Normalize3(Add3(Scale3(up, std::cos(angle)), Scale3(right, -std::sin(angle))), up);
            float halfW = size * (0.82f + p.aspect * 0.10f);
            float halfH = size * (0.82f + (1.0f / std::max(0.60f, p.aspect)) * 0.10f);
            XMFLOAT3 hw = Scale3(side, halfW);
            XMFLOAT3 hh = Scale3(vertical, halfH);
            float material = 15.0f + std::min(0.985f, lifeFade * 0.94f + p.seed * 0.035f);
            AppendDynamicQuad(verts,
                Add3(pos, Add3(Scale3(hw, -1.0f), Scale3(hh, -1.0f))),
                Add3(pos, Add3(hw, Scale3(hh, -1.0f))),
                Add3(pos, Add3(hw, hh)),
                Add3(pos, Add3(Scale3(hw, -1.0f), hh)),
                toCam, side, material);
            ++emitted;
        }
    }

    void AppendSteamBillboards(std::vector<Vertex>& verts) {
        XMVECTOR cam = XMLoadFloat3(&camera_);
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        for (const SteamParticle& sp : steam_) {
            float lifeLeft = Clamp01(1.0f - sp.age / std::max(0.001f, sp.life));
            if (lifeLeft <= 0.0f) continue;
            XMVECTOR pos = XMLoadFloat3(&sp.pos);
            XMVECTOR toCam = XMVector3Normalize(cam - pos);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, toCam));
            float size = sp.size * (0.65f + (1.0f - lifeLeft) * 1.55f);
            XMVECTOR halfW = right * size;
            XMVECTOR halfH = up * (size * 0.78f);
            XMFLOAT3 n, t, a, b, c, d;
            XMStoreFloat3(&n, toCam);
            XMStoreFloat3(&t, right);
            XMStoreFloat3(&a, pos - halfW - halfH);
            XMStoreFloat3(&b, pos + halfW - halfH);
            XMStoreFloat3(&c, pos + halfW + halfH);
            XMStoreFloat3(&d, pos - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, d, n, t, 12.58f + lifeLeft * 0.35f);
        }
    }

    void AppendVentDrops(std::vector<Vertex>& verts) {
        for (const VentDrop& d : ventDrops_) {
            XMFLOAT3 right = RotateYVec({1.0f, 0.0f, 0.0f}, d.yaw);
            XMFLOAT3 forward = RotateYVec({0.0f, 0.0f, 1.0f}, d.yaw);
            XMVECTOR upVec = XMVector3Normalize(XMVectorSet(0, std::cos(d.roll), 0, 0) + XMLoadFloat3(&forward) * std::sin(d.roll));
            XMVECTOR rightVec = XMLoadFloat3(&right);
            XMVECTOR normalVec = XMVector3Normalize(XMVector3Cross(upVec, rightVec));
            XMVECTOR center = XMLoadFloat3(&d.pos);
            XMVECTOR halfW = rightVec * d.halfW;
            XMVECTOR halfH = upVec * d.halfH;
            XMFLOAT3 n, t, a, b, c, e;
            XMStoreFloat3(&n, normalVec);
            XMStoreFloat3(&t, rightVec);
            XMStoreFloat3(&a, center - halfW - halfH);
            XMStoreFloat3(&b, center + halfW - halfH);
            XMStoreFloat3(&c, center + halfW + halfH);
            XMStoreFloat3(&e, center - halfW + halfH);
            AppendDynamicQuad(verts, a, b, c, e, n, t, 10.0f);
            AppendDynamicQuad(verts, b, a, e, c, Scale3(n, -1.0f), Scale3(t, -1.0f), 10.0f);
        }
    }

    void UpdateDynamicGeometry() {
        std::vector<Vertex>& opaqueVerts = dynamicOpaqueVerts_;
        std::vector<Vertex>& transparentVerts = dynamicTransparentVerts_;
        opaqueVerts.clear();
        transparentVerts.clear();
        if (opaqueVerts.capacity() < 32768) opaqueVerts.reserve(32768);
        if (transparentVerts.capacity() < 131072) transparentVerts.reserve(131072);
        AppendDynamicDoor(opaqueVerts);
        AppendVentDrops(opaqueVerts);
        AppendMonsterBillboard(opaqueVerts, transparentVerts);
        AppendAirParticleBillboards(transparentVerts);
        AppendSparkBillboards(transparentVerts);
        AppendSteamBillboards(transparentVerts);
        if (opaqueVerts.size() > kDynamicVertexCapacity) opaqueVerts.resize(kDynamicVertexCapacity);
        size_t remaining = static_cast<size_t>(kDynamicVertexCapacity) - opaqueVerts.size();
        if (transparentVerts.size() > remaining) transparentVerts.resize(remaining);

        dynamicOpaqueVertexCount_ = static_cast<UINT>(opaqueVerts.size());
        dynamicTransparentVertexCount_ = static_cast<UINT>(transparentVerts.size());
        dynamicVertexCount_ = static_cast<UINT>(opaqueVerts.size() + transparentVerts.size());
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (dynamicBuffer_ && dynamicVertexCount_ > 0 &&
            SUCCEEDED(context_->Map(dynamicBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            auto* dst = static_cast<uint8_t*>(mapped.pData);
            size_t opaqueBytes = opaqueVerts.size() * sizeof(Vertex);
            size_t transparentBytes = transparentVerts.size() * sizeof(Vertex);
            if (opaqueBytes > 0) {
                std::memcpy(dst, opaqueVerts.data(), opaqueBytes);
            }
            if (transparentBytes > 0) {
                std::memcpy(dst + opaqueBytes, transparentVerts.data(), transparentBytes);
            }
            context_->Unmap(dynamicBuffer_.Get(), 0);
        }
    }

    void UploadSceneConstants(const SceneConstants& cb) {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (SUCCEEDED(context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            std::memcpy(mapped.pData, &cb, sizeof(cb));
            context_->Unmap(constantBuffer_.Get(), 0);
        }
    }

    void UploadLampDamageTexture() {
        if (!lampDamageDirty_ || !lampDamageTexture_ || lampDamagePixels_.empty()) return;
        context_->UpdateSubresource(
            lampDamageTexture_.Get(),
            0,
            nullptr,
            lampDamagePixels_.data(),
            static_cast<UINT>(maze_.w),
            0);
        lampDamageDirty_ = false;
    }

    std::array<XMFLOAT4, 2> ActiveSparkLights() const {
        std::array<XMFLOAT4, 2> lights{};
        std::array<float, 2> power{};
        for (const SparkFlash& flash : sparkFlashes_) {
            float lifeLeft = Clamp01(1.0f - flash.age / std::max(0.001f, flash.life));
            float p = flash.intensity * lifeLeft * lifeLeft;
            if (p <= 0.02f) continue;
            int slot = power[0] <= power[1] ? 0 : 1;
            if (p > power[slot]) {
                power[slot] = p;
                lights[static_cast<size_t>(slot)] = {flash.pos.x, flash.pos.y, flash.pos.z, p};
            }
        }
        return lights;
    }

    void DrawOverlayVertices(const std::vector<OverlayVertex>& verts) {
        if (verts.empty() || !overlayBuffer_ || !overlayVertexShader_ || !overlayPixelShader_) return;
        size_t count = std::min(verts.size(), static_cast<size_t>(kOverlayVertexCapacity));
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(context_->Map(overlayBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
        std::memcpy(mapped.pData, verts.data(), count * sizeof(OverlayVertex));
        context_->Unmap(overlayBuffer_.Get(), 0);

        UINT stride = sizeof(OverlayVertex);
        UINT offset = 0;
        float blendFactor[4] = {};
        context_->IASetInputLayout(overlayInputLayout_.Get());
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->IASetVertexBuffers(0, 1, overlayBuffer_.GetAddressOf(), &stride, &offset);
        context_->VSSetShader(overlayVertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        context_->PSSetShader(overlayPixelShader_.Get(), nullptr, 0);
        context_->OMSetDepthStencilState(depthDisabledState_.Get(), 0);
        context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
        context_->Draw(static_cast<UINT>(count), 0);
    }

    void DrawPostProcess() {
        if (!sceneColorSrv_ || !postVertexShader_ || !postPixelShader_ || !postSampler_ || !rtv_) return;
        float blendFactor[4] = {};
        ID3D11RenderTargetView* target = rtv_.Get();
        context_->OMSetRenderTargets(1, &target, nullptr);
        context_->OMSetDepthStencilState(depthDisabledState_.Get(), 0);
        context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
        context_->RSSetState(rasterState_.Get());
        context_->IASetInputLayout(nullptr);
        context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(postVertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        context_->PSSetShader(postPixelShader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        ID3D11ShaderResourceView* srv = sceneColorSrv_.Get();
        ID3D11SamplerState* sampler = postSampler_.Get();
        context_->PSSetShaderResources(0, 1, &srv);
        context_->PSSetSamplers(0, 1, &sampler);
        context_->Draw(3, 0);
        ID3D11ShaderResourceView* nullSrv = nullptr;
        context_->PSSetShaderResources(0, 1, &nullSrv);
    }

    void DrawDreadMeterOverlay() {
        if (!settings_.dreadEnabled || !settings_.dreadDebugMeter || !overlayBuffer_ ||
            !overlayVertexShader_ || !overlayPixelShader_ || width_ <= 0 || height_ <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(48);
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };

        float x = 18.0f;
        float y = 18.0f;
        float w = 170.0f;
        float h = 13.0f;
        float fill = std::round((w - 4.0f) * Clamp01(dreadMeterLevel_));
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.07f, 0.025f, 0.025f, 0.74f});
        pushRect(x, y, w, h, {0.015f, 0.008f, 0.008f, 0.86f});
        if (fill > 0.5f) {
            pushRect(x + 2.0f, y + 2.0f, fill, h - 4.0f, {0.72f, 0.055f, 0.042f, 0.93f});
            pushRect(x + 2.0f, y + 2.0f, fill, 2.0f, {1.0f, 0.20f, 0.13f, 0.38f});
        }
        pushRect(x - 2.0f, y - 2.0f, w + 4.0f, 1.0f, {0.40f, 0.08f, 0.07f, 0.86f});
        pushRect(x - 2.0f, y + h + 1.0f, w + 4.0f, 1.0f, {0.23f, 0.035f, 0.035f, 0.86f});
        pushRect(x - 2.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});
        pushRect(x + w + 1.0f, y - 2.0f, 1.0f, h + 4.0f, {0.33f, 0.055f, 0.052f, 0.86f});

        DrawOverlayVertices(verts);
    }

    void DrawGameHudOverlay() {
        if (runtimeMode_ != RendererRuntimeMode::PlayableGame || !overlayBuffer_ ||
            !overlayVertexShader_ || !overlayPixelShader_ || width_ <= 0 || height_ <= 0) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(96);
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };
        auto pushBar = [&](float x, float y, float w, float h, float fill, XMFLOAT4 fillColor) {
            fill = Clamp01(fill);
            pushRect(x - 2.0f, y - 2.0f, w + 4.0f, h + 4.0f, {0.015f, 0.014f, 0.012f, 0.78f});
            pushRect(x, y, w, h, {0.03f, 0.028f, 0.024f, 0.84f});
            float fillW = std::round(w * fill);
            if (fillW > 0.5f) {
                pushRect(x, y, fillW, h, fillColor);
                pushRect(x, y, fillW, std::max(1.0f, h * 0.22f), {1.0f, 1.0f, 1.0f, 0.16f});
            }
        };

        float x = 24.0f;
        float y = static_cast<float>(height_) - 58.0f;
        float w = std::clamp(static_cast<float>(width_) * 0.22f, 180.0f, 290.0f);
        pushBar(x, y, w, 14.0f, playerHealth_ / 100.0f, {0.72f, 0.05f, 0.045f, 0.92f});
        pushBar(x, y + 24.0f, w, 10.0f, playerStamina_ / 100.0f, {0.88f, 0.72f, 0.23f, 0.88f});
        DrawOverlayVertices(verts);
    }

    void DrawMapOverlay() {
        if (!settings_.mapOverlay || !overlayBuffer_ || !overlayVertexShader_ || !overlayPixelShader_ ||
            width_ <= 0 || height_ <= 0 || maze_.w <= 0 || maze_.h <= 0 || maze_.open.empty()) {
            return;
        }

        std::vector<OverlayVertex> verts;
        verts.reserve(std::min(static_cast<size_t>(kOverlayVertexCapacity),
            static_cast<size_t>(maze_.w * maze_.h * 6 + 96)));
        auto ndcX = [&](float px) { return px / static_cast<float>(width_) * 2.0f - 1.0f; };
        auto ndcY = [&](float py) { return 1.0f - py / static_cast<float>(height_) * 2.0f; };
        auto pushRect = [&](float x, float y, float w, float h, XMFLOAT4 color) {
            if (verts.size() + 6 > static_cast<size_t>(kOverlayVertexCapacity)) return;
            float l = ndcX(x);
            float r = ndcX(x + w);
            float t = ndcY(y);
            float b = ndcY(y + h);
            verts.push_back({{l, b}, color});
            verts.push_back({{l, t}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{l, b}, color});
            verts.push_back({{r, t}, color});
            verts.push_back({{r, b}, color});
        };

        float maxW = std::clamp(static_cast<float>(width_) * 0.22f, 125.0f, 245.0f);
        float maxH = std::clamp(static_cast<float>(height_) * 0.22f, 110.0f, 210.0f);
        float cell = std::max(1.15f, std::min(maxW / static_cast<float>(maze_.w), maxH / static_cast<float>(maze_.h)));
        float mapW = cell * static_cast<float>(maze_.w);
        float mapH = cell * static_cast<float>(maze_.h);
        float pad = 7.0f;
        float x0 = static_cast<float>(width_) - mapW - pad - 18.0f;
        float y0 = static_cast<float>(height_) - mapH - pad - 18.0f;

        pushRect(x0 - pad, y0 - pad, mapW + pad * 2.0f, mapH + pad * 2.0f, {0.0f, 0.0f, 0.0f, 0.32f});
        pushRect(x0 - 1.0f, y0 - 1.0f, mapW + 2.0f, mapH + 2.0f, {0.53f, 0.46f, 0.31f, 0.24f});
        pushRect(x0, y0, mapW, mapH, {0.025f, 0.022f, 0.017f, 0.44f});
        auto mapTileX = [&](int tileX) {
            return x0 + static_cast<float>(maze_.w - 1 - tileX) * cell;
        };

        for (int y = 0; y < maze_.h; ++y) {
            for (int x = 0; x < maze_.w; ++x) {
                if (!maze_.IsOpen(x, y)) continue;
                XMFLOAT4 color{0.74f, 0.66f, 0.47f, 0.36f};
                Tile t{x, y};
                if (t == maze_.start) color = {0.25f, 0.80f, 0.42f, 0.68f};
                else if (t == maze_.exit) color = {1.0f, 0.26f, 0.12f, 0.76f};
                float px = mapTileX(x);
                float py = y0 + static_cast<float>(y) * cell;
                float inset = std::max(0.18f, cell * 0.10f);
                pushRect(px + inset, py + inset, std::max(0.6f, cell - inset * 2.0f), std::max(0.6f, cell - inset * 2.0f), color);
            }
        }

        auto markTile = [&](Tile t, XMFLOAT4 color, float scale) {
            if (!maze_.InBounds(t.x, t.y)) return;
            float size = std::max(3.0f, cell * scale);
            float px = mapTileX(t.x) + cell * 0.5f - size * 0.5f;
            float py = y0 + (static_cast<float>(t.y) + 0.5f) * cell - size * 0.5f;
            pushRect(px, py, size, size, color);
        };
        markTile(CameraTile(), {0.20f, 0.72f, 1.0f, 0.82f}, 1.70f);
        markTile(MonsterTile(), {0.88f, 0.04f, 0.05f, 0.64f}, 1.45f);

        DrawOverlayVertices(verts);
    }

    void Render() {
        lastPresentCompleted_ = false;
        if (!rtv_ || !dsv_) {
            lastPresentCompleted_ = true;
            return;
        }
        StartupProfile renderProfile(L"Render");
        bool postAvailable = sceneColorRtv_ && sceneColorSrv_ && postVertexShader_ && postPixelShader_ && postSampler_;
        ID3D11RenderTargetView* sceneTarget = postAvailable ? sceneColorRtv_.Get() : rtv_.Get();

        float clear[4] = {0.004f, 0.004f, 0.004f, 1.0f};
        if (monsterPreview_) {
            if (monsterPreviewView_ == MonsterPreviewView::Orbit) {
                clear[0] = clear[1] = clear[2] = 0.93f;
            } else {
                clear[0] = clear[1] = clear[2] = 0.0f;
            }
        }
        context_->ClearRenderTargetView(rtv_.Get(), clear);
        if (postAvailable) {
            context_->ClearRenderTargetView(sceneColorRtv_.Get(), clear);
        }
        context_->ClearDepthStencilView(dsv_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        renderProfile.Mark(L"ClearTargets");

        D3D11_VIEWPORT vp{};
        vp.Width = static_cast<float>(width_);
        vp.Height = static_cast<float>(height_);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        context_->RSSetViewports(1, &vp);

        float deathProgress = deathActive_ ? Clamp01(deathTimer_ / 4.25f) : 0.0f;
        float deathFadeProgress = deathActive_ ? Clamp01((deathTimer_ - 0.10f) / 3.55f) : 0.0f;
        float deathFocus = deathActive_ ? SmoothStep(0.0f, 0.24f, deathTimer_) : 0.0f;
        XMFLOAT3 f = Forward();
        float stepPhase = stepPhase_;
        float runCameraMotion = 1.0f + runIntensity_ * 1.05f + runEffort_ * 1.25f;
        float sideSway = std::sin(stepPhase) * settings_.sideSwayAmount * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR right = XMVectorSet(std::cos(bodyYaw_), 0.0f, -std::sin(bodyYaw_), 0.0f);
        XMVECTOR eye = XMLoadFloat3(&camera_) + right * sideSway;
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        float bobPitch = std::sin(stepPhase * 0.5f) * 0.020f * runCameraMotion * (1.0f - deathFocus);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, lookPitch_ + bobPitch, f.z, 0.0f));
        XMVECTOR viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        if (monsterPreview_ && monsterPreviewView_ == MonsterPreviewView::Top) {
            XMVECTOR topTarget = XMVectorSet(monster_.x, 1.20f, monster_.z, 0.0f);
            viewDir = XMVector3Normalize(topTarget - eye);
            worldUp = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        if (deathActive_) {
            float jitterStrength = (1.0f - SmoothStep(0.74f, 1.0f, deathProgress)) * (0.010f + deathProgress * 0.025f);
            float jitterX = (std::sin(time_ * 31.0f) + std::sin(time_ * 57.0f) * 0.45f) * jitterStrength;
            float jitterY = (std::sin(time_ * 43.0f) + std::sin(time_ * 71.0f) * 0.35f) * jitterStrength;
            viewDir = XMVector3Normalize(viewDir + viewRight * jitterX + worldUp * jitterY);
            viewRight = XMVector3Normalize(XMVector3Cross(worldUp, viewDir));
        }
        float roll = deathActive_
            ? (std::sin(time_ * 18.0f) * 0.075f + std::sin(time_ * 37.0f) * 0.045f) * SmoothStep(0.06f, 0.42f, deathProgress)
            : stumbleYawOffset_ * 0.12f * SmoothStep(0.0f, 0.35f, stumbleTimer_);
        XMVECTOR up = XMVector3Normalize(worldUp * std::cos(roll) + viewRight * std::sin(roll));
        XMVECTOR at = eye + viewDir;
        XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
        float aspect = static_cast<float>(std::max<LONG>(1, width_)) / static_cast<float>(std::max<LONG>(1, height_));
        float fovDegrees = monsterPreview_ ? 48.0f : 70.0f - dangerLevel_ * 3.5f;
        if (gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView) {
            fovDegrees = 86.0f;
        }
        if (deathActive_) {
            fovDegrees = Lerp(70.0f, 29.0f, SmoothStep(0.05f, 0.66f, deathProgress));
        }
        float exitStepStart = std::max(0.05f, settings_.exitDoorOpenSeconds * 0.68f);
        float exitStepEnd = exitStepStart + settings_.exitStepSeconds;
        float exitFade = exitTransitionActive_ ? SmoothStep(exitStepEnd - settings_.exitFadeSeconds * 0.64f, exitStepEnd + settings_.exitFadeSeconds * 0.36f, exitTransitionTimer_) : 0.0f;
        float fadeIn = (fadeInTimer_ > 0.0f && settings_.fadeInSeconds > 0.001f)
            ? 1.0f - SmoothStep(0.0f, settings_.fadeInSeconds, settings_.fadeInSeconds - fadeInTimer_)
            : 0.0f;
        float transitionFade = std::max(exitFade, fadeIn);
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, 42.0f);

        XMFLOAT3 flashlightForward = FlashlightForward();
        XMVECTOR lightDir = XMVector3Normalize(XMVectorSet(flashlightForward.x, flashlightForward.y, flashlightForward.z, 0.0f));
        XMVECTOR lightPos = eye + viewRight * 0.16f - up * 0.18f + viewDir * 0.08f;
        float shadowDistance = settings_.flashlightShadowDistanceMeters;
        float coneHalfRadians = std::clamp(settings_.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneOuter = std::cos(coneHalfRadians);
        float coneInner = std::cos(std::max(3.0f * kPi / 180.0f, coneHalfRadians * 0.36f));
        float shadowFov = std::clamp(settings_.flashlightConeDegrees + 8.0f, 36.0f, 150.0f) * kPi / 180.0f;
        XMMATRIX lightView = XMMatrixLookAtLH(lightPos, lightPos + lightDir, up);
        XMMATRIX lightProj = XMMatrixPerspectiveFovLH(shadowFov, 1.0f, 0.06f, shadowDistance);
        XMMATRIX lightViewProj = lightView * lightProj;
        XMFLOAT3 lightPosFloat{};
        XMFLOAT3 lightDirFloat{};
        XMStoreFloat3(&lightPosFloat, lightPos);
        XMStoreFloat3(&lightDirFloat, lightDir);

        SceneConstants cb{};
        XMStoreFloat4x4(&cb.viewProj, view * proj);
        XMStoreFloat4x4(&cb.lightViewProj, lightViewProj);
        XMFLOAT3 eyePos{};
        XMStoreFloat3(&eyePos, eye);
        float flashlightIntensity = settings_.flashlightIntensity * DreadFlashlightMultiplier();
        if (monsterPreview_) flashlightIntensity = 1.45f;
        cb.cameraPosTime = {eyePos.x, eyePos.y, eyePos.z, time_};
        cb.cameraDirAspect = {lightDirFloat.x, lightDirFloat.y, lightDirFloat.z, aspect};
        cb.lighting0 = {
            flashlightIntensity,
            settings_.flashlightAttenuation,
            settings_.ambientLight,
            std::max(2.0f, settings_.lampSpacing)
        };
        if (monsterPreview_) {
            cb.lighting0.y = 0.030f;
            cb.lighting0.z = 0.22f;
        }
        if (gEffectDebugViewer) {
            cb.lighting0.x = std::clamp(cb.lighting0.x, 0.55f, 0.92f);
            cb.lighting0.y = std::max(cb.lighting0.y, 0.085f);
            cb.lighting0.z = kEffectDebugAmbientLight;
        } else if (gBloodDebugEveryWall || settings_.bloodStudyView) {
            cb.lighting0.x = std::max(cb.lighting0.x, 1.85f);
            cb.lighting0.y = std::min(cb.lighting0.y, 0.040f);
            cb.lighting0.z = std::max(cb.lighting0.z, 0.42f);
        }
        cb.lighting1 = {
            settings_.lampIntensity,
            settings_.lampOnRatio,
            settings_.lampFlickerRatio,
            settings_.brokenZoneRatio
        };
        if (gEffectDebugViewer) {
            cb.lighting1.x = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? kEffectDebugLampIntensity : 0.0f;
            cb.lighting1.y = (gDebugSliceEffect == DebugSliceEffect::CeilingLamps) ? 1.0f : 0.0f;
            cb.lighting1.z = 0.0f;
            cb.lighting1.w = (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) ? 1.0f : 0.0f;
        } else if (gBloodDebugEveryWall || settings_.bloodStudyView) {
            cb.lighting1.x = std::max(cb.lighting1.x, 2.35f);
            cb.lighting1.y = 1.0f;
            cb.lighting1.z = 0.0f;
            cb.lighting1.w = 0.0f;
        }
        cb.fog0 = {
            settings_.fogStartMeters,
            settings_.fogEndMeters,
            settings_.fogDarkness,
            0.0f
        };
        if (monsterPreview_) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        if (gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView) {
            cb.fog0 = {1000.0f, 1001.0f, 0.0f, 0.0f};
        }
        cb.ao0 = {
            settings_.cornerAOIntensity,
            settings_.cornerAORadius,
            settings_.floorCeilingAOIntensity,
            maze_.TileAverage()
        };
        cb.post0 = {
            settings_.exposure,
            settings_.gamma,
            deathActive_ ? 1.0f : dangerLevel_,
            deathFadeProgress
        };
        cb.post1 = {
            cameraMotionBlur_.x,
            cameraMotionBlur_.y,
            std::clamp(settings_.bloomAmount, 0.0f, 2.0f),
            std::clamp(settings_.lensDirtAmount, 0.0f, 2.0f)
        };
        if (monsterPreview_) {
            cb.post0 = {1.0f, 2.2f, 0.0f, 0.0f};
            cb.post1 = {0.0f, 0.0f, 0.0f, 0.0f};
        }
        cb.shadow0 = {
            lightPosFloat.x,
            lightPosFloat.y,
            lightPosFloat.z,
            settings_.flashlightShadows ? settings_.flashlightShadowStrength : 0.0f
        };
        cb.shadow1 = {
            lightDirFloat.x,
            lightDirFloat.y,
            lightDirFloat.z,
            settings_.flashlightShadowBias
        };
        cb.shadow2 = {
            1.0f / static_cast<float>(std::max<UINT>(1, shadowMapSize_)),
            shadowDistance,
            coneOuter,
            coneInner
        };
        cb.maze0 = {
            -static_cast<float>(maze_.w) * maze_.tileW * 0.5f,
            -static_cast<float>(maze_.h) * maze_.tileD * 0.5f,
            maze_.tileW,
            maze_.tileD
        };
        cb.maze1 = {
            static_cast<float>(maze_.w),
            static_cast<float>(maze_.h),
            settings_.wallHeightMeters,
            maze_.TileAverage()
        };
        cb.texture0 = {
            settings_.wallTextureMeters,
            settings_.floorTextureMeters,
            settings_.ceilingTextureMeters,
            0.0f
        };
        cb.transition0 = {
            transitionFade,
            exitTransitionActive_ ? exitDoorAngle_ : 0.0f,
            0.0f,
            0.0f
        };
        if (gEffectDebugViewer && DebugSliceEffectIsWater(gDebugSliceEffect)) {
            cb.transition0.w = 1.0f + DebugSliceLoopPhase();
        } else if (!gEffectDebugViewer) {
            cb.transition0.w = 0.0f;
        }
        float fleshAmount = 0.0f;
        if (fleshFlickerTimer_ > 0.0f && fleshFlickerDuration_ > 0.001f) {
            float elapsed = fleshFlickerDuration_ - fleshFlickerTimer_;
            float envelope = SmoothStep(0.0f, 0.08f, elapsed) * (1.0f - SmoothStep(fleshFlickerDuration_ - 0.20f, fleshFlickerDuration_, elapsed));
            float strobe = ((std::sin(elapsed * 34.0f) + std::sin(elapsed * 71.0f) * 0.55f + std::sin(elapsed * 113.0f) * 0.28f) > 0.16f) ? 1.0f : 0.0f;
            fleshAmount = envelope * strobe * settings_.fleshFlickerIntensity;
        }
        if (settings_.fleshAlwaysOn) fleshAmount = std::max(fleshAmount, settings_.fleshFlickerIntensity);
        cb.horror0 = {
            Clamp01(fleshAmount),
            std::clamp(settings_.bloodWetness, 0.0f, 3.0f),
            std::clamp(settings_.fleshWetness, 0.0f, 4.0f),
            std::clamp(settings_.fleshParallaxScale, 0.0f, 0.32f)
        };
        float bloodStreamCount = static_cast<float>(std::clamp(settings_.bloodStreamCount, 4, 32));
        float bloodStreamThickness = std::clamp(settings_.bloodStreamThickness, 0.10f, 2.0f);
        float bloodShaderQuality = std::clamp(settings_.bloodShaderQuality, 0.25f, 1.0f);
        float bloodWorldAmount = 0.0f;
        if (bloodWorldFlickerTimer_ > 0.0f && bloodWorldFlickerDuration_ > 0.001f) {
            float elapsed = bloodWorldFlickerDuration_ - bloodWorldFlickerTimer_;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(bloodWorldFlickerDuration_ - 0.18f, bloodWorldFlickerDuration_, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f + std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            bloodWorldAmount = envelope * strobe * settings_.bloodWorldFlickerIntensity;
        }
        if (settings_.bloodWorldAlwaysOn && settings_.bloodWorldCoverage > 0.001f) {
            bloodWorldAmount = std::max(bloodWorldAmount, settings_.bloodWorldFlickerIntensity);
            if (bloodWorldActivationTime_ < -900.0f) bloodWorldActivationTime_ = time_ - 46.0f;
        }
        cb.blood0 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood1 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood2 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood3 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood4 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood5 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood6 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood7 = {0.0f, 0.0f, 0.0f, 0.0f};
        cb.blood8 = {0.0f, 0.0f, 0.0f, 0.0f};
        if (gBloodDebugEveryWall) {
            float debugClock = DebugSliceClockSeconds();
            float cycleAge = std::fmod(std::max(0.0f, debugClock), std::max(0.1f, settings_.effectBloodLoopSeconds));
            float debugAge = std::min(cycleAge, settings_.effectBloodFullSpreadAge);
            cb.blood0 = {0.0f, 0.0f, time_ - debugAge, 1.0f};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (settings_.bloodStudyView) {
            cb.blood0 = {0.0f, 0.0f, -40.0f, 1.0f};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (bloodWorldAmount > 0.001f && settings_.bloodWorldCoverage > 0.001f) {
            cb.blood0 = {camera_.x, camera_.z, bloodWorldActivationTime_, Clamp01(bloodWorldAmount)};
            cb.blood1 = {bloodStreamCount, kEffectBloodRevealRadius, bloodStreamThickness, bloodShaderQuality};
        } else if (!bloodRevealRegions_.empty()) {
            std::vector<const BloodRevealRegion*> regions;
            regions.reserve(bloodRevealRegions_.size());
            for (const BloodRevealRegion& region : bloodRevealRegions_) {
                if (region.radius > 0.001f && region.activationTime > -999000.0f) {
                    regions.push_back(&region);
                }
            }
            std::sort(regions.begin(), regions.end(), [this](const BloodRevealRegion* a, const BloodRevealRegion* b) {
                float adx = a->center.x - camera_.x;
                float adz = a->center.z - camera_.z;
                float bdx = b->center.x - camera_.x;
                float bdz = b->center.z - camera_.z;
                return adx * adx + adz * adz < bdx * bdx + bdz * bdz;
            });
            if (!regions.empty()) {
                const BloodRevealRegion& primary = *regions.front();
                cb.blood0 = {primary.center.x, primary.center.z, primary.activationTime, 1.0f};
                cb.blood1 = {bloodStreamCount, std::max(1.0f, primary.radius), bloodStreamThickness, bloodShaderQuality};
                auto assignRegion = [&](size_t slot, const BloodRevealRegion& region) {
                    XMFLOAT4 value{
                        region.center.x,
                        region.center.z,
                        region.activationTime,
                        std::max(1.0f, region.radius)
                    };
                    switch (slot) {
                    case 2: cb.blood2 = value; break;
                    case 3: cb.blood3 = value; break;
                    case 4: cb.blood4 = value; break;
                    case 5: cb.blood5 = value; break;
                    case 6: cb.blood6 = value; break;
                    case 7: cb.blood7 = value; break;
                    case 8: cb.blood8 = value; break;
                    default: break;
                    }
                };
                size_t slot = 2;
                for (size_t i = 1; i < regions.size() && slot <= 8; ++i, ++slot) {
                    assignRegion(slot, *regions[i]);
                }
            }
        }
        cb.air0 = {
            airFocusDistance_,
            std::clamp(settings_.airParticleBlur, 0.0f, 3.0f),
            std::clamp(settings_.airParticleSize, 0.20f, 4.0f),
            settings_.airParticles ? std::clamp(settings_.airParticleDensity, 0.0f, 4.0f) : 0.0f
        };
        cb.exitLight0 = {
            exitSignLightPos_.x,
            exitSignLightPos_.y,
            exitSignLightPos_.z,
            exitSignLightStrength_
        };
        float monsterFogRadius = std::max(maze_.TileAverage() * 2.60f, 5.80f);
        float monsterFogStrength = (monsterPreview_ || gEffectDebugViewer || gBloodDebugEveryWall || settings_.bloodStudyView)
            ? 0.0f
            : 0.72f;
        cb.monsterFog0 = {
            monster_.x,
            monster_.z,
            monsterFogRadius,
            monsterFogStrength
        };
        bool fleshLightingSuppressed =
            (settings_.fleshAlwaysOn && settings_.fleshFlickerIntensity > 0.001f) ||
            (fleshFlickerTimer_ > 0.0f && fleshFlickerDuration_ > 0.001f);
        if (fleshLightingSuppressed) {
            // Shader uses transition0.z to keep the entire flesh event flashlight-only, including strobe gaps.
            cb.transition0.z = 1.0f;
            cb.lighting0.z = 0.0f;
            cb.lighting1.x = 0.0f;
            cb.lighting1.y = 0.0f;
            cb.lighting1.z = 0.0f;
        }
        bool useFleshTessellation = cb.horror0.x > 0.01f &&
            featureLevel_ >= D3D_FEATURE_LEVEL_11_0 &&
            hullShader_ && domainShader_ &&
            cb.horror0.w > 0.001f;
        std::array<XMFLOAT4, 2> sparkLights = fleshLightingSuppressed
            ? std::array<XMFLOAT4, 2>{XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}, XMFLOAT4{0.0f, 0.0f, 0.0f, 0.0f}}
            : ActiveSparkLights();
        cb.sparkLight0 = sparkLights[0];
        cb.sparkLight1 = sparkLights[1];

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        float blendFactor[4] = {};

        UpdateDynamicGeometry();
        renderProfile.Mark(L"UpdateDynamicGeometry");

        if (shadowDsv_ && settings_.flashlightShadows && settings_.flashlightShadowStrength > 0.001f) {
            renderProfile.Mark(L"BeginShadowPass");
            ID3D11ShaderResourceView* nullSrvs[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
            context_->PSSetShaderResources(0, 7, nullSrvs);
            context_->ClearDepthStencilView(shadowDsv_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
            context_->OMSetRenderTargets(0, nullptr, shadowDsv_.Get());

            D3D11_VIEWPORT shadowVp{};
            shadowVp.Width = static_cast<float>(shadowMapSize_);
            shadowVp.Height = static_cast<float>(shadowMapSize_);
            shadowVp.MinDepth = 0.0f;
            shadowVp.MaxDepth = 1.0f;
            context_->RSSetViewports(1, &shadowVp);

            SceneConstants shadowCb = cb;
            XMStoreFloat4x4(&shadowCb.viewProj, lightViewProj);
            UploadSceneConstants(shadowCb);

            context_->IASetInputLayout(inputLayout_.Get());
            context_->IASetPrimitiveTopology(useFleshTessellation
                ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
                : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
            context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
            context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
            context_->HSSetShader(useFleshTessellation ? hullShader_.Get() : nullptr, nullptr, 0);
            context_->DSSetShader(useFleshTessellation ? domainShader_.Get() : nullptr, nullptr, 0);
            context_->PSSetShader(shadowPixelShader_.Get(), nullptr, 0);
            context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->HSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            context_->DSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
            if (useFleshTessellation) {
                ID3D11ShaderResourceView* dsSrvs[] = {nullptr, normalSrv_.Get(), nullptr, nullptr, nullptr, nullptr, nullptr};
                context_->DSSetShaderResources(0, 7, dsSrvs);
                context_->DSSetSamplers(0, 1, sampler_.GetAddressOf());
            }
            context_->RSSetState(shadowRasterState_.Get());
            context_->OMSetDepthStencilState(depthState_.Get(), 0);
            context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            if (!monsterPreview_) {
                UINT shadowStaticIndexCount = staticWaterStartIndex_ > 0 ? staticWaterStartIndex_ :
                    (staticTransparentStartIndex_ > 0 ? staticTransparentStartIndex_ : indexCount_);
                if (shadowStaticIndexCount > 0) {
                    StartupProfileLine(L"Render before ShadowStatic DrawIndexed");
                    context_->DrawIndexed(shadowStaticIndexCount, 0, 0);
                    renderProfile.Mark(L"ShadowStatic");
                }
                if (staticPropShadowIndexCount_ > 0) {
                    context_->PSSetShader(nullptr, nullptr, 0);
                    StartupProfileLine(L"Render before StaticPropShadow DrawIndexed");
                    context_->DrawIndexed(staticPropShadowIndexCount_, staticPropShadowStartIndex_, 0);
                    renderProfile.Mark(L"StaticPropShadow");
                    context_->PSSetShader(shadowPixelShader_.Get(), nullptr, 0);
                }
            }

            if (dynamicOpaqueVertexCount_ > 0) {
                context_->HSSetShader(nullptr, nullptr, 0);
                context_->DSSetShader(nullptr, nullptr, 0);
                context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                context_->IASetVertexBuffers(0, 1, dynamicBuffer_.GetAddressOf(), &stride, &offset);
                StartupProfileLine(L"Render before ShadowDynamic Draw");
                context_->Draw(dynamicOpaqueVertexCount_, 0);
                renderProfile.Mark(L"ShadowDynamic");
            }
        }

        UploadSceneConstants(cb);
        renderProfile.Mark(L"UploadSceneConstants");
        UploadLampDamageTexture();
        renderProfile.Mark(L"UploadLampDamageTexture");

        ID3D11ShaderResourceView* srvs[] = {albedoSrv_.Get(), normalSrv_.Get(), shadowSrv_.Get(), mazeSrv_.Get(), materialPropsSrv_.Get(), flashlightPatternSrv_.Get(), lampDamageSrv_.Get()};
        ID3D11SamplerState* samplers[] = {sampler_.Get(), shadowSampler_.Get()};
        context_->OMSetRenderTargets(1, &sceneTarget, dsv_.Get());
        context_->RSSetViewports(1, &vp);
        context_->IASetInputLayout(inputLayout_.Get());
        context_->IASetPrimitiveTopology(useFleshTessellation
            ? D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST
            : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
        context_->HSSetShader(useFleshTessellation ? hullShader_.Get() : nullptr, nullptr, 0);
        context_->DSSetShader(useFleshTessellation ? domainShader_.Get() : nullptr, nullptr, 0);
        context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
        context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->HSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->DSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
        context_->PSSetShaderResources(0, 7, srvs);
        if (useFleshTessellation) {
            context_->DSSetShaderResources(0, 7, srvs);
            context_->DSSetSamplers(0, 1, sampler_.GetAddressOf());
        }
        context_->PSSetSamplers(0, 2, samplers);
        context_->RSSetState(rasterState_.Get());
        context_->OMSetDepthStencilState(depthState_.Get(), 0);

        if (!monsterPreview_) {
            context_->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
            context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
            context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            UINT opaqueIndexCount = std::min(floorCeilingStartIndex_, indexCount_);
            if (opaqueIndexCount > 0) {
                StartupProfileLine(L"Render before MainOpaque DrawIndexed");
                context_->DrawIndexed(opaqueIndexCount, 0, 0);
                renderProfile.Mark(L"MainOpaque");
            }
            if (floorCeilingIndexCount_ > 0) {
                StartupProfileLine(L"Render before FloorCeiling DrawIndexed");
                context_->DrawIndexed(floorCeilingIndexCount_, floorCeilingStartIndex_, 0);
                renderProfile.Mark(L"FloorCeiling");
            }
            if (staticWaterIndexCount_ > 0) {
                context_->OMSetDepthStencilState(depthLessState_.Get(), 0);
                context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
                StartupProfileLine(L"Render before StaticWater DrawIndexed");
                context_->DrawIndexed(staticWaterIndexCount_, staticWaterStartIndex_, 0);
                renderProfile.Mark(L"StaticWater");
                context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            }
            if (staticTransparentIndexCount_ > 0) {
                context_->OMSetDepthStencilState(depthReadOnlyState_.Get(), 0);
                context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
                StartupProfileLine(L"Render before StaticTransparent DrawIndexed");
                context_->DrawIndexed(staticTransparentIndexCount_, staticTransparentStartIndex_, 0);
                renderProfile.Mark(L"StaticTransparent");
                context_->OMSetDepthStencilState(depthState_.Get(), 0);
                context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            }
        }

        if (dynamicOpaqueVertexCount_ > 0 || dynamicTransparentVertexCount_ > 0) {
            context_->HSSetShader(nullptr, nullptr, 0);
            context_->DSSetShader(nullptr, nullptr, 0);
            context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            context_->IASetVertexBuffers(0, 1, dynamicBuffer_.GetAddressOf(), &stride, &offset);
        }
        if (dynamicOpaqueVertexCount_ > 0) {
            context_->OMSetDepthStencilState(depthState_.Get(), 0);
            context_->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
            StartupProfileLine(L"Render before DynamicOpaque Draw");
            context_->Draw(dynamicOpaqueVertexCount_, 0);
            renderProfile.Mark(L"DynamicOpaque");
        }
        if (dynamicTransparentVertexCount_ > 0) {
            context_->OMSetDepthStencilState(depthReadOnlyState_.Get(), 0);
            context_->OMSetBlendState(alphaBlend_.Get(), blendFactor, 0xffffffff);
            StartupProfileLine(L"Render before DynamicTransparent Draw");
            context_->Draw(dynamicTransparentVertexCount_, dynamicOpaqueVertexCount_);
            renderProfile.Mark(L"DynamicTransparent");
        }

        ID3D11ShaderResourceView* nullSrvs[] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
        context_->PSSetShaderResources(0, 7, nullSrvs);
        context_->DSSetShaderResources(0, 7, nullSrvs);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        if (postAvailable) {
            StartupProfileLine(L"Render before DrawPostProcess");
            DrawPostProcess();
            renderProfile.Mark(L"PostProcess");
        }

        if (!monsterPreview_) {
            StartupProfileLine(L"Render before DrawMapOverlay");
            DrawMapOverlay();
            renderProfile.Mark(L"MapOverlay");
            StartupProfileLine(L"Render before DrawDreadMeterOverlay");
            DrawDreadMeterOverlay();
            renderProfile.Mark(L"DreadOverlay");
            StartupProfileLine(L"Render before DrawGameHudOverlay");
            DrawGameHudOverlay();
            renderProfile.Mark(L"GameHudOverlay");
        }

        context_->PSSetShaderResources(0, 7, nullSrvs);
        context_->DSSetShaderResources(0, 7, nullSrvs);
        context_->HSSetShader(nullptr, nullptr, 0);
        context_->DSSetShader(nullptr, nullptr, 0);
        if (presentEnabled_) {
            StartupProfileLine(L"Render before Present");
            HRESULT presentHr = swapChain_->Present(presentSyncInterval_, presentFlags_);
            lastPresentCompleted_ = presentHr != DXGI_ERROR_WAS_STILL_DRAWING;
            renderProfile.Mark(L"Present");
        } else {
            StartupProfileLine(L"Render skipping Present");
            lastPresentCompleted_ = true;
            renderProfile.Mark(L"NoPresent");
        }
    }
};

enum class RunMode {
    Fullscreen,
    Preview,
    Configure,
    SelfTest,
    MonsterPreview,
    MonsterPreviewFront,
    MonsterPreviewSide,
    MonsterPreviewLeftSide,
    MonsterPreviewTop,
    BloodDebug,
    GenerateIni
};

enum class GameState {
    MainMenu,
    PlayGame,
    DebugScene,
    Settings,
    Exit
};

struct App {
    Renderer renderer;
    bool preview = false;
    bool gameShell = false;
    bool rendererInitialized = false;
    bool gameRunStarted = false;
    bool gameDebugActive = false;
    HINSTANCE gameInstance = nullptr;
    GameState gameState = GameState::MainMenu;
    bool gameMouseCaptured = false;
    bool gameRecenteringMouse = false;
    POINT gameMouseCenter{};
    float gameMouseDeltaX = 0.0f;
    float gameMouseDeltaY = 0.0f;
    HWND gameTitle = nullptr;
    HWND gameSinglePlayer = nullptr;
    HWND gameSettings = nullptr;
    HWND gameDebug = nullptr;
    HWND gameBack = nullptr;
    HWND gameExit = nullptr;
    HWND gameConfig = nullptr;
    GameState gameSettingsReturnState = GameState::MainMenu;
    bool firstMouse = true;
    POINT initialMouse{};
    HWND hwnd = nullptr;
    HWND debugPrevEffect = nullptr;
    HWND debugNextEffect = nullptr;
    HWND debugSize = nullptr;
    HWND debugReset = nullptr;
    HWND debugPrevProp = nullptr;
    HWND debugNextProp = nullptr;
    HWND debugSettings = nullptr;
    HWND loadingOverlay = nullptr;
    bool loadingWarmupPending = false;
    ULONGLONG loadingWarmupStart = 0;
    int loadingWarmupAttempts = 0;
    bool quitting = false;

    struct CloneOutput {
        HWND hwnd = nullptr;
        HWND loadingOverlay = nullptr;
        Renderer renderer;
        bool loadingWarmupPending = false;
        ULONGLONG loadingWarmupStart = 0;
        int loadingWarmupAttempts = 0;
    };
    std::vector<std::unique_ptr<CloneOutput>> clones;
};

App* gApp = nullptr;

App::CloneOutput* CloneForWindow(HWND hwnd) {
    if (!gApp) return nullptr;
    for (auto& clone : gApp->clones) {
        if (clone && clone->hwnd == hwnd) return clone.get();
    }
    return nullptr;
}

constexpr int kDebugPrevEffectId = 5101;
constexpr int kDebugNextEffectId = 5102;
constexpr int kDebugSizeId = 5103;
constexpr int kDebugResetId = 5104;
constexpr int kDebugPrevPropId = 5105;
constexpr int kDebugNextPropId = 5106;
constexpr int kDebugSettingsId = 5107;
constexpr int kGameSinglePlayerId = 5201;
constexpr int kGameSettingsId = 5202;
constexpr int kGameDebugId = 5203;
constexpr int kGameBackId = 5204;
constexpr int kGameExitId = 5205;
constexpr UINT kGameConfigClosedMessage = WM_APP + 31;

DebugSliceEffect StepDebugSliceEffect(DebugSliceEffect effect, int delta) {
    int count = static_cast<int>(DebugSliceEffect::Count);
    int index = (static_cast<int>(effect) + delta) % count;
    if (index < 0) index += count;
    return static_cast<DebugSliceEffect>(index);
}

void UpdateDebugSliceControls(HWND hwnd) {
    if (!gApp || !gEffectDebugViewer) return;
    wchar_t title[160]{};
    if (gDebugSliceEffect == DebugSliceEffect::Props) {
        swprintf_s(title, L"Backrooms Maze Effect Slice Debug - Props - %s (%d/%d)",
            DebugPropName(gDebugPropIndex), WrapDebugPropIndex(gDebugPropIndex) + 1, kDebugPropCount);
    } else {
        swprintf_s(title, L"Backrooms Maze Effect Slice Debug - %s - %dx%d",
            DebugSliceEffectName(gDebugSliceEffect), gDebugSliceTiles, gDebugSliceTiles);
    }
    SetWindowTextW(hwnd, title);
    if (gApp->debugSize) {
        wchar_t sizeText[48]{};
        swprintf_s(sizeText, L"Grid: %dx%d", gDebugSliceTiles, gDebugSliceTiles);
        SetWindowTextW(gApp->debugSize, sizeText);
    }
    if (gApp->debugPrevProp) EnableWindow(gApp->debugPrevProp, TRUE);
    if (gApp->debugNextProp) EnableWindow(gApp->debugNextProp, TRUE);
}

void RedrawDebugSliceControls() {
    if (!gApp || !gEffectDebugViewer) return;
    HWND controls[] = {
        gApp->debugPrevEffect,
        gApp->debugNextEffect,
        gApp->debugSize,
        gApp->debugReset,
        gApp->debugPrevProp,
        gApp->debugNextProp,
        gApp->debugSettings
    };
    for (HWND control : controls) {
        if (!control) continue;
        SetWindowPos(control, HWND_TOP, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        RedrawWindow(control, nullptr, nullptr,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
    }
}

enum class ConfigDialogMode {
    Full,
    Game,
    Debug
};
void ShowConfig(HWND owner, ConfigDialogMode mode = ConfigDialogMode::Full);
HWND CreateEmbeddedConfig(HWND parent, ConfigDialogMode mode);
HWND CreateGameSettingsPanel(HWND parent);
HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance);
void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete);
void LoadingProgressCallback(void* context, const StartupProgressUpdate& update);

void LayoutGameControls(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int w = std::max(1L, rc.right - rc.left);
    int h = std::max(1L, rc.bottom - rc.top);
    int centerX = w / 2;
    int buttonW = 220;
    int buttonH = 38;
    int gap = 12;
    int top = std::max(90, h / 2 - 120);
    if (gApp->gameTitle) MoveWindow(gApp->gameTitle, centerX - 240, top - 76, 480, 42, TRUE);
    if (gApp->gameSinglePlayer) MoveWindow(gApp->gameSinglePlayer, centerX - buttonW / 2, top, buttonW, buttonH, TRUE);
    if (gApp->gameSettings) MoveWindow(gApp->gameSettings, centerX - buttonW / 2, top + (buttonH + gap), buttonW, buttonH, TRUE);
    if (gApp->gameDebug) MoveWindow(gApp->gameDebug, centerX - buttonW / 2, top + (buttonH + gap) * 2, buttonW, buttonH, TRUE);
    if (gApp->gameExit) MoveWindow(gApp->gameExit, centerX - buttonW / 2, top + (buttonH + gap) * 3, buttonW, buttonH, TRUE);
    if (gApp->gameBack) MoveWindow(gApp->gameBack, 12, 10, 104, 28, TRUE);
}

void SetGameMenuVisible(bool visible) {
    if (!gApp || !gApp->gameShell) return;
    int show = visible ? SW_SHOW : SW_HIDE;
    HWND controls[] = {
        gApp->gameTitle,
        gApp->gameSinglePlayer,
        gApp->gameSettings,
        gApp->gameDebug,
        gApp->gameExit
    };
    for (HWND control : controls) {
        if (control) ShowWindow(control, show);
    }
}

void UpdateGameMenuLabels() {
    if (!gApp || !gApp->gameShell) return;
    bool canResume = gApp->gameRunStarted && !gApp->gameDebugActive;
    if (gApp->gameSinglePlayer) {
        SetWindowTextW(gApp->gameSinglePlayer, canResume ? L"Resume" : L"Single Player");
    }
}

void SetDebugControlsVisible(bool visible) {
    if (!gApp) return;
    int show = visible ? SW_SHOW : SW_HIDE;
    HWND controls[] = {
        gApp->debugPrevEffect,
        gApp->debugNextEffect,
        gApp->debugSize,
        gApp->debugReset,
        gApp->debugPrevProp,
        gApp->debugNextProp
    };
    for (HWND control : controls) {
        if (control) ShowWindow(control, show);
    }
}

void SetGameCursorVisible(bool visible) {
    if (visible) {
        while (ShowCursor(TRUE) < 0) {}
    } else {
        while (ShowCursor(FALSE) >= 0) {}
    }
}

void ReleaseGameMouse() {
    if (!gApp) return;
    ClipCursor(nullptr);
    if (gApp->gameMouseCaptured) {
        ReleaseCapture();
    }
    SetGameCursorVisible(true);
    gApp->gameMouseCaptured = false;
}

void CaptureGameMouse(HWND hwnd) {
    if (!gApp || !gApp->gameShell || !hwnd) return;
    RECT rc{};
    GetClientRect(hwnd, &rc);
    POINT tl{rc.left, rc.top};
    POINT br{rc.right, rc.bottom};
    ClientToScreen(hwnd, &tl);
    ClientToScreen(hwnd, &br);
    RECT clip{tl.x, tl.y, br.x, br.y};
    ClipCursor(&clip);
    if (!gApp->gameMouseCaptured) {
        SetCapture(hwnd);
        SetGameCursorVisible(false);
    }
    gApp->gameMouseCaptured = true;
    gApp->gameMouseCenter = {(rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2};
    POINT center = gApp->gameMouseCenter;
    ClientToScreen(hwnd, &center);
    gApp->gameRecenteringMouse = true;
    SetCursorPos(center.x, center.y);
}

bool EnsureGameRenderer(HWND hwnd, RendererRuntimeMode mode) {
    if (!gApp || !gApp->gameShell) return false;
    if (gApp->rendererInitialized) return true;
    gApp->renderer.SetRuntimeMode(mode);
    if (!gApp->loadingOverlay) {
        gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance);
    }
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay, L"Loading level", L"Preparing renderer and maze.", false);
        UpdateWindow(gApp->loadingOverlay);
    }
    StartupProgressSink loadingProgress{LoadingProgressCallback, gApp->loadingOverlay};
    if (!gApp->renderer.Initialize(hwnd, nullptr, false, MonsterPreviewView::Orbit,
            gApp->loadingOverlay ? &loadingProgress : nullptr)) {
        if (gApp->loadingOverlay) {
            DestroyWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        MessageBoxW(hwnd, L"Direct3D initialization failed.", L"Backrooms Maze Game", MB_OK | MB_ICONERROR);
        return false;
    }
    gApp->rendererInitialized = true;
    if (gApp->loadingOverlay) {
        SetLoadingOverlayStatus(gApp->loadingOverlay, L"Ready", L"Entering maze.", true);
        DestroyWindow(gApp->loadingOverlay);
        gApp->loadingOverlay = nullptr;
    }
    return true;
}

void EnterGameMainMenu(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    ReleaseGameMouse();
    gApp->gameState = GameState::MainMenu;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    SetGameMenuVisible(true);
    UpdateGameMenuLabels();
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    SetWindowTextW(hwnd, L"Backrooms Maze");
}

void EnterGamePlay(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    if (!EnsureGameRenderer(hwnd, RendererRuntimeMode::PlayableGame)) return;
    if (!gApp->gameRunStarted || gApp->gameDebugActive) {
        if (!gApp->loadingOverlay) {
            gApp->loadingOverlay = CreateLoadingOverlay(hwnd, gApp->gameInstance);
        }
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Loading level", L"Generating maze and scene geometry.", false);
            UpdateWindow(gApp->loadingOverlay);
        }
        gApp->renderer.RestartGameRun();
        if (gApp->loadingOverlay) {
            SetLoadingOverlayStatus(gApp->loadingOverlay, L"Ready", L"Entering maze.", true);
            DestroyWindow(gApp->loadingOverlay);
            gApp->loadingOverlay = nullptr;
        }
        gApp->gameRunStarted = true;
    } else {
        gApp->renderer.SetRuntimeMode(RendererRuntimeMode::PlayableGame);
    }
    gApp->gameState = GameState::PlayGame;
    gApp->gameDebugActive = false;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    CaptureGameMouse(hwnd);
    SetWindowTextW(hwnd, L"Backrooms Maze - Single Player");
}

void EnterGameDebug(HWND hwnd) {
    if (!gApp || !gApp->gameShell) return;
    gEffectDebugViewer = true;
    gDebugSliceEffect = DebugSliceEffect::Blood;
    gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    gBloodDebugEveryWall = true;
    if (!EnsureGameRenderer(hwnd, RendererRuntimeMode::DebugViewer)) return;
    gApp->renderer.EnterDebugViewer(gDebugSliceEffect, gDebugSliceTiles);
    gApp->gameState = GameState::DebugScene;
    gApp->gameDebugActive = true;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(true);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_SHOW);
    ReleaseGameMouse();
    UpdateDebugSliceControls(hwnd);
    RedrawDebugSliceControls();
}

void EnterGameSettings(HWND hwnd, ConfigDialogMode mode, GameState returnState) {
    if (!gApp || !gApp->gameShell) return;
    ReleaseGameMouse();
    if (gApp->gameConfig) {
        DestroyWindow(gApp->gameConfig);
        gApp->gameConfig = nullptr;
    }
    gApp->gameSettingsReturnState = returnState;
    gApp->gameState = GameState::Settings;
    SetGameMenuVisible(false);
    SetDebugControlsVisible(false);
    if (gApp->gameBack) ShowWindow(gApp->gameBack, SW_HIDE);
    gApp->gameConfig = mode == ConfigDialogMode::Game
        ? CreateGameSettingsPanel(hwnd)
        : CreateEmbeddedConfig(hwnd, mode);
    if (gApp->gameConfig) {
        RECT rc{};
        GetClientRect(hwnd, &rc);
        MoveWindow(gApp->gameConfig, 0, 0, std::max(1L, rc.right - rc.left), std::max(1L, rc.bottom - rc.top), TRUE);
        ShowWindow(gApp->gameConfig, SW_SHOW);
        SetFocus(gApp->gameConfig);
        SetWindowTextW(hwnd, mode == ConfigDialogMode::Debug ? L"Backrooms Maze - Debug Settings" : L"Backrooms Maze - Settings");
    } else if (returnState == GameState::DebugScene) {
        EnterGameDebug(hwnd);
    } else {
        EnterGameMainMenu(hwnd);
    }
}

GameInputSnapshot CollectGameInput() {
    GameInputSnapshot input{};
    if (!gApp || gApp->gameState != GameState::PlayGame) return input;
    auto down = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
    input.moveX = (down('D') ? 1.0f : 0.0f) + (down('A') ? -1.0f : 0.0f);
    input.moveZ = (down('W') ? 1.0f : 0.0f) + (down('S') ? -1.0f : 0.0f);
    input.lookDeltaX = gApp->gameMouseDeltaX;
    input.lookDeltaY = gApp->gameMouseDeltaY;
    gApp->gameMouseDeltaX = 0.0f;
    gApp->gameMouseDeltaY = 0.0f;
    input.jump = down(VK_SPACE);
    input.sprint = down(VK_SHIFT);
    input.crouch = down(VK_CONTROL) || down('C');
    input.interact = down('E');
    input.pause = down(VK_ESCAPE);
    return input;
}

constexpr int kLoadingOverlayTimerId = 6101;
const wchar_t* kLoadingOverlayClass = L"BackroomsMazeLoadingOverlay";

struct LoadingOverlayState {
    std::wstring phase = L"Starting renderer";
    std::wstring detail = L"";
    int current = 0;
    int total = 1;
    int shaderDone = 0;
    int shaderTotal = 0;
    int shaderCompiled = 0;
    int shaderCached = 0;
    int frame = 0;
};

LoadingOverlayState* LoadingState(HWND hwnd) {
    return reinterpret_cast<LoadingOverlayState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

void DrawLoadingSpinner(HDC hdc, int cx, int cy, int radius, int frame) {
    constexpr int dots = 12;
    for (int i = 0; i < dots; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(dots)) * kPi * 2.0f;
        int age = (i - frame) % dots;
        if (age < 0) age += dots;
        int intensity = 72 + (dots - age) * 13;
        intensity = std::clamp(intensity, 72, 228);
        int x = cx + static_cast<int>(std::cos(angle) * radius);
        int y = cy + static_cast<int>(std::sin(angle) * radius);
        int dotRadius = std::max(2, radius / 6);
        HBRUSH brush = CreateSolidBrush(RGB(intensity, intensity - 8, intensity / 2));
        HGDIOBJ oldBrush = SelectObject(hdc, brush);
        HPEN pen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        Ellipse(hdc, x - dotRadius, y - dotRadius, x + dotRadius + 1, y + dotRadius + 1);
        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
        DeleteObject(brush);
    }
}

void DrawLoadingOverlay(HWND hwnd, HDC hdc) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    int width = std::max(1, static_cast<int>(rc.right - rc.left));
    int height = std::max(1, static_cast<int>(rc.bottom - rc.top));
    LoadingOverlayState* state = LoadingState(hwnd);

    HBRUSH bg = CreateSolidBrush(RGB(8, 8, 6));
    FillRect(hdc, &rc, bg);
    DeleteObject(bg);

    int contentW = std::min(560, std::max(180, width - 48));
    int left = (width - contentW) / 2;
    int top = std::max(18, height / 2 - 82);
    int spinnerRadius = std::clamp(std::min(width, height) / 18, 12, 24);
    int spinnerCx = left + spinnerRadius + 8;
    int spinnerCy = top + spinnerRadius + 4;
    DrawLoadingSpinner(hdc, spinnerCx, spinnerCy, spinnerRadius, state ? state->frame : 0);

    SetBkMode(hdc, TRANSPARENT);
    HFONT titleFont = CreateFontW(-22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT detailFont = CreateFontW(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    int textLeft = spinnerCx + spinnerRadius + 18;
    RECT titleRect{textLeft, top - 1, left + contentW, top + 31};
    SetTextColor(hdc, RGB(245, 235, 184));
    HGDIOBJ oldFont = SelectObject(hdc, titleFont);
    const std::wstring title = state && !state->phase.empty() ? state->phase : L"Loading";
    DrawTextW(hdc, title.c_str(), -1, &titleRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    RECT detailRect{textLeft, top + 31, left + contentW, top + 58};
    SelectObject(hdc, detailFont);
    SetTextColor(hdc, RGB(184, 178, 137));
    const std::wstring detail = state && !state->detail.empty() ? state->detail : L"Preparing startup tasks.";
    DrawTextW(hdc, detail.c_str(), -1, &detailRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    int barTop = top + 75;
    RECT barRect{left, barTop, left + contentW, barTop + 14};
    HBRUSH barBg = CreateSolidBrush(RGB(34, 34, 27));
    FillRect(hdc, &barRect, barBg);
    DeleteObject(barBg);
    FrameRect(hdc, &barRect, reinterpret_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));

    int current = state ? state->current : 0;
    int total = std::max(1, state ? state->total : 1);
    float fraction = Clamp01(static_cast<float>(current) / static_cast<float>(total));
    RECT fillRect = barRect;
    fillRect.right = fillRect.left + static_cast<int>((barRect.right - barRect.left) * fraction);
    InflateRect(&fillRect, -1, -1);
    if (fillRect.right > fillRect.left) {
        HBRUSH fill = CreateSolidBrush(RGB(215, 188, 72));
        FillRect(hdc, &fillRect, fill);
        DeleteObject(fill);
    }

    std::wostringstream summary;
    summary << L"Startup " << current << L"/" << total;
    if (state && state->shaderTotal > 0) {
        summary << L"    Shaders " << state->shaderDone << L"/" << state->shaderTotal
                << L" (compiled " << state->shaderCompiled << L", cached " << state->shaderCached << L")";
    }
    RECT summaryRect{left, barTop + 22, left + contentW, barTop + 48};
    SetTextColor(hdc, RGB(132, 128, 102));
    DrawTextW(hdc, summary.str().c_str(), -1, &summaryRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);

    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);
    DeleteObject(detailFont);
}

LRESULT CALLBACK LoadingOverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCCREATE: {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return TRUE;
    }
    case WM_CREATE:
        SetTimer(hwnd, kLoadingOverlayTimerId, 120, nullptr);
        return 0;
    case WM_TIMER:
        if (wParam == kLoadingOverlayTimerId) {
            if (LoadingOverlayState* state = LoadingState(hwnd)) {
                state->frame = (state->frame + 1) % 12;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawLoadingOverlay(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_NCDESTROY:
        KillTimer(hwnd, kLoadingOverlayTimerId);
        delete LoadingState(hwnd);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void RegisterLoadingOverlayClass(HINSTANCE hInstance) {
    static bool registered = false;
    if (registered) return;
    WNDCLASSW wc{};
    wc.lpfnWndProc = LoadingOverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kLoadingOverlayClass;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);
    registered = true;
}

void ResizeLoadingOverlay(HWND parent, HWND overlay) {
    if (!parent || !overlay) return;
    RECT rc{};
    GetClientRect(parent, &rc);
    MoveWindow(overlay, 0, 0,
        std::max(1, static_cast<int>(rc.right - rc.left)),
        std::max(1, static_cast<int>(rc.bottom - rc.top)), TRUE);
}

HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance) {
    RegisterLoadingOverlayClass(hInstance);
    auto* state = new LoadingOverlayState();
    RECT rc{};
    GetClientRect(parent, &rc);
    HWND overlay = CreateWindowExW(0, kLoadingOverlayClass, nullptr, WS_CHILD | WS_VISIBLE,
        0, 0, std::max(1, static_cast<int>(rc.right - rc.left)),
        std::max(1, static_cast<int>(rc.bottom - rc.top)),
        parent, nullptr, hInstance, state);
    if (!overlay) {
        delete state;
        return nullptr;
    }
    SetWindowPos(overlay, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    UpdateWindow(overlay);
    return overlay;
}

void SetLoadingOverlayProgress(HWND overlay, const StartupProgressUpdate& update) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (!state) return;
    state->phase = update.phase ? update.phase : L"Loading";
    state->detail = update.detail ? update.detail : L"";
    state->current = std::clamp(update.current, 0, std::max(1, update.total));
    state->total = std::max(1, update.total);
    state->shaderDone = std::clamp(update.shaderDone, 0, std::max(0, update.shaderTotal));
    state->shaderTotal = std::max(0, update.shaderTotal);
    state->shaderCompiled = std::max(0, update.shaderCompiled);
    state->shaderCached = std::max(0, update.shaderCached);
    state->frame = (state->frame + 1) % 12;
    RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete) {
    if (!overlay) return;
    LoadingOverlayState* state = LoadingState(overlay);
    if (!state) return;
    int total = std::max(state->total + 1, state->current + 1);
    state->phase = phase ? phase : L"Loading";
    state->detail = detail ? detail : L"";
    state->total = std::max(1, total);
    state->current = complete ? state->total : std::max(0, state->total - 1);
    state->frame = (state->frame + 1) % 12;
    RedrawWindow(overlay, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

void LoadingProgressCallback(void* context, const StartupProgressUpdate& update) {
    SetLoadingOverlayProgress(static_cast<HWND>(context), update);
}

void QuitScreensaver(HWND hwnd) {
    if (gApp && !gApp->preview && !gApp->quitting) {
        gApp->quitting = true;
        for (auto& clone : gApp->clones) {
            if (clone && clone->hwnd && IsWindow(clone->hwnd)) {
                DestroyWindow(clone->hwnd);
            }
        }
        if (gApp->hwnd && IsWindow(gApp->hwnd)) {
            DestroyWindow(gApp->hwnd);
        } else if (hwnd) {
            DestroyWindow(hwnd);
        }
        return;
    }
    DestroyWindow(hwnd);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        return 0;
    case WM_SIZE:
        if (gApp) {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            if (hwnd == gApp->hwnd) {
                if (gApp->loadingOverlay) ResizeLoadingOverlay(hwnd, gApp->loadingOverlay);
                if (gApp->gameConfig) MoveWindow(gApp->gameConfig, 0, 0, std::max(1, w), std::max(1, h), TRUE);
                gApp->renderer.Resize(w, h);
                if (gApp->gameShell) {
                    LayoutGameControls(hwnd);
                    if (gApp->gameMouseCaptured) CaptureGameMouse(hwnd);
                }
            } else if (App::CloneOutput* clone = CloneForWindow(hwnd)) {
                if (clone->loadingOverlay) ResizeLoadingOverlay(hwnd, clone->loadingOverlay);
                clone->renderer.Resize(w, h);
            }
        }
        return 0;
    case kGameConfigClosedMessage:
        if (gApp && gApp->gameShell && hwnd == gApp->hwnd) {
            if (gApp->gameSettingsReturnState == GameState::DebugScene) {
                EnterGameDebug(hwnd);
            } else {
                EnterGameMainMenu(hwnd);
            }
            return 0;
        }
        break;
    case WM_SETCURSOR:
        if (gApp && gApp->gameShell && gApp->gameState == GameState::PlayGame) {
            SetCursor(nullptr);
            return TRUE;
        }
        if (gApp && gApp->gameShell) {
            SetCursor(LoadCursorW(nullptr, IDC_ARROW));
            return TRUE;
        }
        if (gApp && !gApp->preview) {
            SetCursor(nullptr);
            return TRUE;
        }
        break;
    case WM_MOUSEMOVE:
        if (gApp && gApp->gameShell && gApp->gameState == GameState::PlayGame && hwnd == gApp->hwnd) {
            POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (gApp->gameRecenteringMouse) {
                gApp->gameRecenteringMouse = false;
                return 0;
            }
            gApp->gameMouseDeltaX += static_cast<float>(p.x - gApp->gameMouseCenter.x);
            gApp->gameMouseDeltaY += static_cast<float>(p.y - gApp->gameMouseCenter.y);
            POINT center = gApp->gameMouseCenter;
            ClientToScreen(hwnd, &center);
            gApp->gameRecenteringMouse = true;
            SetCursorPos(center.x, center.y);
            return 0;
        }
        if (gApp && !gApp->preview) {
            POINT p{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            if (gApp->firstMouse) {
                gApp->initialMouse = p;
                gApp->firstMouse = false;
            } else {
                int dx = p.x - gApp->initialMouse.x;
                int dy = p.y - gApp->initialMouse.y;
                if (dx * dx + dy * dy > 36) QuitScreensaver(hwnd);
            }
        }
        return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (gApp && gApp->gameShell) {
            if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && wParam == VK_ESCAPE &&
                (gApp->gameState == GameState::PlayGame || gApp->gameState == GameState::DebugScene)) {
                EnterGameMainMenu(hwnd);
            } else if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && wParam == VK_ESCAPE &&
                gApp->gameState == GameState::Settings && gApp->gameConfig) {
                DestroyWindow(gApp->gameConfig);
            } else if ((msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) && wParam == VK_ESCAPE &&
                gApp->gameState == GameState::MainMenu && gApp->gameRunStarted && !gApp->gameDebugActive) {
                EnterGamePlay(hwnd);
            }
            return 0;
        }
        if (gApp && !gApp->preview) QuitScreensaver(hwnd);
        return 0;
    case WM_COMMAND:
        if (gApp && gApp->gameShell && hwnd == gApp->hwnd) {
            int id = LOWORD(wParam);
            if (id == kGameSinglePlayerId) {
                EnterGamePlay(hwnd);
                return 0;
            }
            if (id == kGameSettingsId) {
                EnterGameSettings(hwnd, ConfigDialogMode::Game, GameState::MainMenu);
                return 0;
            }
            if (id == kGameDebugId) {
                EnterGameDebug(hwnd);
                return 0;
            }
            if (id == kGameBackId) {
                if (gApp->gameState == GameState::Settings && gApp->gameConfig) {
                    DestroyWindow(gApp->gameConfig);
                    return 0;
                }
                EnterGameMainMenu(hwnd);
                return 0;
            }
            if (id == kDebugSettingsId) {
                EnterGameSettings(hwnd, ConfigDialogMode::Debug, GameState::DebugScene);
                return 0;
            }
            if (id == kGameExitId) {
                ReleaseGameMouse();
                DestroyWindow(hwnd);
                return 0;
            }
        }
        if (gApp && gEffectDebugViewer && hwnd == gApp->hwnd) {
            int id = LOWORD(wParam);
            if (id == kDebugPrevEffectId || id == kDebugNextEffectId || id == kDebugSizeId || id == kDebugResetId ||
                id == kDebugPrevPropId || id == kDebugNextPropId) {
                if (id == kDebugPrevEffectId) {
                    gDebugSliceEffect = StepDebugSliceEffect(gDebugSliceEffect, -1);
                } else if (id == kDebugNextEffectId) {
                    gDebugSliceEffect = StepDebugSliceEffect(gDebugSliceEffect, 1);
                } else if (id == kDebugSizeId) {
                    gDebugSliceTiles = gDebugSliceTiles >= 5 ? 1 : gDebugSliceTiles + 1;
                } else if (id == kDebugPrevPropId || id == kDebugNextPropId) {
                    if (gDebugSliceEffect != DebugSliceEffect::Props) {
                        gDebugSliceEffect = DebugSliceEffect::Props;
                    }
                    gDebugPropIndex = WrapDebugPropIndex(gDebugPropIndex + (id == kDebugPrevPropId ? -1 : 1));
                } else if (id == kDebugResetId) {
                    gApp->renderer.ResetDebugSliceAnimation();
                    UpdateDebugSliceControls(hwnd);
                    RedrawDebugSliceControls();
                    return 0;
                }
                gApp->renderer.ConfigureDebugSlice(gDebugSliceEffect, gDebugSliceTiles);
                UpdateDebugSliceControls(hwnd);
                RedrawDebugSliceControls();
                return 0;
            }
        }
        break;
    case WM_ACTIVATEAPP:
        if (gApp && gApp->gameShell) {
            if (wParam == FALSE) ReleaseGameMouse();
            else if (gApp->gameState == GameState::PlayGame) CaptureGameMouse(gApp->hwnd);
            return 0;
        }
        if (gApp && !gApp->preview && wParam == FALSE) QuitScreensaver(hwnd);
        return 0;
    case WM_DESTROY:
        if (gApp && gApp->gameShell) ReleaseGameMouse();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

std::wstring Lower(std::wstring s) {
    std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return s;
}

uintptr_t ParseHandle(const wchar_t* s) {
    if (!s) return 0;
    while (*s == L':' || *s == L' ') ++s;
    return static_cast<uintptr_t>(_wcstoui64(s, nullptr, 0));
}

RunMode ParseMode(int argc, wchar_t** argv, HWND& previewParent) {
    for (int i = 1; i < argc; ++i) {
        std::wstring arg = Lower(argv[i]);
        if (arg.rfind(L"/c", 0) == 0 || arg.rfind(L"-c", 0) == 0) return RunMode::Configure;
        if (arg == L"/selftest" || arg == L"-selftest") return RunMode::SelfTest;
        if (arg == L"/makeini" || arg == L"-makeini") return RunMode::GenerateIni;
        if (arg == L"/monsterpreviewfront" || arg == L"-monsterpreviewfront") return RunMode::MonsterPreviewFront;
        if (arg == L"/monsterpreviewside" || arg == L"-monsterpreviewside") return RunMode::MonsterPreviewSide;
        if (arg == L"/monsterpreviewleft" || arg == L"-monsterpreviewleft") return RunMode::MonsterPreviewLeftSide;
        if (arg == L"/monsterpreviewtop" || arg == L"-monsterpreviewtop") return RunMode::MonsterPreviewTop;
        if (arg == L"/monsterpreview" || arg == L"-monsterpreview") return RunMode::MonsterPreview;
        if (arg == L"/blooddebug" || arg == L"-blooddebug" ||
            arg == L"/effectdebug" || arg == L"-effectdebug") return RunMode::BloodDebug;
        if (arg.rfind(L"/p", 0) == 0 || arg.rfind(L"-p", 0) == 0) {
            uintptr_t handle = 0;
            if (arg.size() > 2) handle = ParseHandle(arg.c_str() + 2);
            if (!handle && i + 1 < argc) handle = ParseHandle(argv[++i]);
            previewParent = reinterpret_cast<HWND>(handle);
            return RunMode::Preview;
        }
        if (arg.rfind(L"/s", 0) == 0 || arg.rfind(L"-s", 0) == 0) return RunMode::Fullscreen;
    }
    return RunMode::Configure;
}

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
        L"Manual player control tuning. Key rebinding is planned after this settings split.",
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
    s.mazeSeed = static_cast<uint32_t>(std::clamp(ParseConfigInt(state, L"Maze", L"RandomSeed", static_cast<int>(s.mazeSeed)), 0, std::numeric_limits<int>::max()));
    s.mapOverlay = ParseConfigInt(state, L"Maze", L"MapOverlay", s.mapOverlay ? 1 : 0) != 0;
    s.tileWidthMeters = std::clamp(ParseConfigFloat(state, L"Maze", L"TileWidthMeters", s.tileWidthMeters), 1.2f, 8.0f);
    s.tileLengthMeters = std::clamp(ParseConfigFloat(state, L"Maze", L"TileLengthMeters", s.tileLengthMeters), 1.2f, 8.0f);
    s.wallHeightMeters = std::clamp(ParseConfigFloat(state, L"Maze", L"WallHeightMeters", s.wallHeightMeters), 1.8f, 8.0f);
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
    s.bloodWorldFlickerMinSeconds = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerMinSeconds", s.bloodWorldFlickerMinSeconds), 3.0f, 600.0f);
    s.bloodWorldFlickerMaxSeconds = std::max(s.bloodWorldFlickerMinSeconds, std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerMaxSeconds", s.bloodWorldFlickerMaxSeconds), 3.0f, 900.0f));
    s.bloodWorldFlickerDuration = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerDuration", s.bloodWorldFlickerDuration), 0.15f, 8.0f);
    s.bloodWorldFlickerIntensity = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"BloodWorldFlickerIntensity", s.bloodWorldFlickerIntensity), 0.0f, 2.0f);
    s.bloodStudyView = ParseConfigInt(state, L"Atmosphere", L"BloodStudyView", s.bloodStudyView ? 1 : 0) != 0;
    s.fleshFlicker = ParseConfigInt(state, L"Atmosphere", L"FleshFlicker", s.fleshFlicker ? 1 : 0) != 0;
    s.fleshAlwaysOn = ParseConfigInt(state, L"Atmosphere", L"FleshAlwaysOn", s.fleshAlwaysOn ? 1 : 0) != 0;
    s.fleshWetness = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshWetness", s.fleshWetness), 0.0f, 4.0f);
    s.fleshParallaxScale = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshParallaxScale", s.fleshParallaxScale), 0.0f, 0.32f);
    s.fleshFlickerMinSeconds = std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshFlickerMinSeconds", s.fleshFlickerMinSeconds), 3.0f, 600.0f);
    s.fleshFlickerMaxSeconds = std::max(s.fleshFlickerMinSeconds, std::clamp(ParseConfigFloat(state, L"Atmosphere", L"FleshFlickerMaxSeconds", s.fleshFlickerMaxSeconds), 3.0f, 900.0f));
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
    s.monsterKillDistance = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterKillDistance", s.monsterKillDistance), 0.2f, 4.0f);
    s.monsterVisibleDistance = std::clamp(ParseConfigFloat(state, L"Monster", L"MonsterVisibleDistance", s.monsterVisibleDistance), 1.0f, 60.0f);
    s.monsterSkullMesh = ParseConfigString(state, L"Monster", L"SkullMesh", s.monsterSkullMesh.c_str());
    s.monsterAltSkullMesh = ParseConfigString(state, L"Monster", L"AlternateSkullMesh", s.monsterAltSkullMesh.c_str());
    s.monsterAltSkullChance = std::clamp(ParseConfigFloat(state, L"Monster", L"AlternateSkullChance", s.monsterAltSkullChance), 0.0f, 1.0f);
    s.monsterSkullMaxTriangles = std::clamp(ParseConfigInt(state, L"Monster", L"SkullMaxTriangles", s.monsterSkullMaxTriangles), 0, 60000);
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

#include "game/game_settings_panel.inl"

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

struct PlaybackMonitorRect {
    RECT rc{};
    bool primary = false;
};

BOOL CALLBACK EnumPlaybackMonitorProc(HMONITOR monitor, HDC, LPRECT, LPARAM lParam) {
    auto* monitors = reinterpret_cast<std::vector<PlaybackMonitorRect>*>(lParam);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) return TRUE;
    RECT rc = info.rcMonitor;
    if (rc.right <= rc.left || rc.bottom <= rc.top) return TRUE;
    monitors->push_back({rc, (info.dwFlags & MONITORINFOF_PRIMARY) != 0});
    return TRUE;
}

std::vector<PlaybackMonitorRect> EnumeratePlaybackMonitors() {
    std::vector<PlaybackMonitorRect> monitors;
    EnumDisplayMonitors(nullptr, nullptr, EnumPlaybackMonitorProc, reinterpret_cast<LPARAM>(&monitors));
    if (monitors.empty()) {
        RECT rc{
            GetSystemMetrics(SM_XVIRTUALSCREEN),
            GetSystemMetrics(SM_YVIRTUALSCREEN),
            GetSystemMetrics(SM_XVIRTUALSCREEN) + GetSystemMetrics(SM_CXVIRTUALSCREEN),
            GetSystemMetrics(SM_YVIRTUALSCREEN) + GetSystemMetrics(SM_CYVIRTUALSCREEN)
        };
        monitors.push_back({rc, true});
    }
    std::stable_sort(monitors.begin(), monitors.end(), [](const PlaybackMonitorRect& a, const PlaybackMonitorRect& b) {
        if (a.primary != b.primary) return a.primary;
        if (a.rc.top != b.rc.top) return a.rc.top < b.rc.top;
        return a.rc.left < b.rc.left;
    });
    return monitors;
}

int RunScreensaver(HINSTANCE hInstance, RunMode mode, HWND previewParent) {
    bool monsterPreviewMode =
        mode == RunMode::MonsterPreview ||
        mode == RunMode::MonsterPreviewFront ||
        mode == RunMode::MonsterPreviewSide ||
        mode == RunMode::MonsterPreviewLeftSide ||
        mode == RunMode::MonsterPreviewTop;
    bool bloodDebugMode = mode == RunMode::BloodDebug;
    gEffectDebugViewer = bloodDebugMode;
    if (bloodDebugMode) {
        gDebugSliceEffect = DebugSliceEffect::Blood;
        gDebugSliceTiles = std::clamp(gDebugSliceTiles, 1, 5);
    }
    gBloodDebugEveryWall = gEffectDebugViewer &&
        (gDebugSliceEffect == DebugSliceEffect::Blood || DebugSliceEffectIsWater(gDebugSliceEffect));
    bool diagnosticWindowMode = monsterPreviewMode || bloodDebugMode;
    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit;
    if (mode == RunMode::MonsterPreviewFront) monsterPreviewView = MonsterPreviewView::Front;
    else if (mode == RunMode::MonsterPreviewSide) monsterPreviewView = MonsterPreviewView::Side;
    else if (mode == RunMode::MonsterPreviewLeftSide) monsterPreviewView = MonsterPreviewView::LeftSide;
    else if (mode == RunMode::MonsterPreviewTop) monsterPreviewView = MonsterPreviewView::Top;

    const wchar_t* cls = L"BackroomsMazeScreensaverWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    App app;
    app.preview = mode == RunMode::Preview || diagnosticWindowMode;
    gApp = &app;

    std::vector<PlaybackMonitorRect> playbackMonitors;
    if (mode == RunMode::Fullscreen) {
        playbackMonitors = EnumeratePlaybackMonitors();
    }

    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_TOPMOST;
    HWND parent = nullptr;
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (!playbackMonitors.empty()) {
        const RECT& rc = playbackMonitors.front().rc;
        x = rc.left;
        y = rc.top;
        w = std::max(1L, rc.right - rc.left);
        h = std::max(1L, rc.bottom - rc.top);
    }

    if (diagnosticWindowMode) {
        style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
        exStyle = 0;
        parent = nullptr;
        x = 80;
        y = 80;
        w = 1100;
        h = 820;
    } else if (mode == RunMode::Preview && previewParent) {
        RECT rc{};
        GetClientRect(previewParent, &rc);
        x = 0;
        y = 0;
        w = std::max(1L, rc.right - rc.left);
        h = std::max(1L, rc.bottom - rc.top);
        style = WS_CHILD | WS_VISIBLE;
        exStyle = 0;
        parent = previewParent;
    }

    HWND hwnd = CreateWindowExW(exStyle, cls, L"Backrooms Maze", style, x, y, w, h, parent, nullptr, hInstance, nullptr);
    if (!hwnd) return 1;
    app.hwnd = hwnd;

    if (!app.preview && playbackMonitors.size() > 1) {
        for (size_t i = 1; i < playbackMonitors.size(); ++i) {
            const RECT& rc = playbackMonitors[i].rc;
            auto clone = std::make_unique<App::CloneOutput>();
            clone->hwnd = CreateWindowExW(exStyle, cls, L"Backrooms Maze", style,
                rc.left, rc.top,
                std::max(1L, rc.right - rc.left),
                std::max(1L, rc.bottom - rc.top),
                nullptr, nullptr, hInstance, nullptr);
            if (clone->hwnd) {
                app.clones.push_back(std::move(clone));
            }
        }
    }

    if (gEffectDebugViewer) {
        gApp->debugPrevEffect = CreateWindowW(L"BUTTON", L"< Effect", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            12, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevEffectId)), hInstance, nullptr);
        gApp->debugNextEffect = CreateWindowW(L"BUTTON", L"Effect >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            110, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextEffectId)), hInstance, nullptr);
        gApp->debugSize = CreateWindowW(L"BUTTON", L"Grid: 3x3", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            210, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSizeId)), hInstance, nullptr);
        gApp->debugReset = CreateWindowW(L"BUTTON", L"Reset anim", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            322, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugResetId)), hInstance, nullptr);
        gApp->debugPrevProp = CreateWindowW(L"BUTTON", L"< Prop", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            434, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevPropId)), hInstance, nullptr);
        gApp->debugNextProp = CreateWindowW(L"BUTTON", L"Prop >", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            526, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextPropId)), hInstance, nullptr);
        UpdateDebugSliceControls(hwnd);
        RedrawDebugSliceControls();
    }

    app.loadingOverlay = CreateLoadingOverlay(hwnd, hInstance);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    for (auto& clone : app.clones) {
        if (!clone || !clone->hwnd) continue;
        clone->loadingOverlay = CreateLoadingOverlay(clone->hwnd, hInstance);
        ShowWindow(clone->hwnd, SW_SHOW);
        UpdateWindow(clone->hwnd);
    }
    if (!app.preview) ShowCursor(FALSE);

    Settings fullscreenSettings;
    const Settings* rendererSettings = nullptr;
    if (mode == RunMode::Fullscreen) {
        fullscreenSettings = LoadSettings();
        fullscreenSettings.mazeSeed = ResolveRuntimeSeed(fullscreenSettings.mazeSeed);
        rendererSettings = &fullscreenSettings;
    }

    StartupProgressSink loadingProgress{LoadingProgressCallback, app.loadingOverlay};
    if (!app.renderer.Initialize(hwnd, rendererSettings, monsterPreviewMode, monsterPreviewView,
            app.loadingOverlay ? &loadingProgress : nullptr)) {
        if (app.loadingOverlay) {
            DestroyWindow(app.loadingOverlay);
            app.loadingOverlay = nullptr;
        }
        MessageBoxW(hwnd, L"Direct3D initialization failed.", L"Backrooms Maze", MB_OK | MB_ICONERROR);
        DestroyWindow(hwnd);
        if (!app.preview) ShowCursor(TRUE);
        return 1;
    }
    for (size_t i = 0; i < app.clones.size(); ++i) {
        auto& clone = app.clones[i];
        if (!clone || !clone->hwnd) continue;
        if (app.loadingOverlay) {
            std::wstringstream detail;
            detail << L"Preparing cloned display " << (i + 2) << L"/" << (app.clones.size() + 1) << L".";
            SetLoadingOverlayStatus(app.loadingOverlay, L"Preparing displays", detail.str().c_str(), false);
        }
        StartupProgressSink cloneProgress{LoadingProgressCallback, clone->loadingOverlay};
        if (!clone->renderer.Initialize(clone->hwnd, rendererSettings, false, MonsterPreviewView::Orbit,
                clone->loadingOverlay ? &cloneProgress : nullptr)) {
            if (clone->loadingOverlay) {
                DestroyWindow(clone->loadingOverlay);
                clone->loadingOverlay = nullptr;
            }
            MessageBoxW(hwnd, L"Direct3D initialization failed on a cloned display.", L"Backrooms Maze", MB_OK | MB_ICONERROR);
            QuitScreensaver(hwnd);
            if (!app.preview) ShowCursor(TRUE);
            return 1;
        }
    }
    app.loadingWarmupPending = app.loadingOverlay != nullptr;
    if (app.loadingWarmupPending) {
        SetLoadingOverlayStatus(app.loadingOverlay, L"Warming first frame",
            L"Starting the first GPU frame.", false);
    }
    for (auto& clone : app.clones) {
        if (clone && clone->loadingOverlay) {
            clone->loadingWarmupPending = true;
            SetLoadingOverlayStatus(clone->loadingOverlay, L"Warming first frame",
                L"Starting the first GPU frame.", false);
        }
    }

    MSG msg{};
    bool running = true;
    ULONGLONG playbackLastTicks = GetTickCount64();
    auto warmupOutput = [](Renderer& renderer, HWND owner, HWND& overlay,
                           bool& pending, ULONGLONG& start, int& attempts) -> bool {
        if (!pending) return false;
        if (start == 0) start = GetTickCount64();
        renderer.SetPresentSyncInterval(0);
        renderer.SetPresentFlags(DXGI_PRESENT_DO_NOT_WAIT);
        renderer.TickFixed(0.0f);
        renderer.SetPresentFlags(0);
        renderer.SetPresentSyncInterval(1);
        ++attempts;
        ULONGLONG warmupElapsed = GetTickCount64() - start;
        if (!renderer.LastPresentCompleted() && attempts < 3 && warmupElapsed < 1500) {
            Sleep(50);
            return true;
        }
        if (overlay) {
            SetLoadingOverlayStatus(overlay, L"Ready", L"Entering maze.", true);
            DestroyWindow(overlay);
            overlay = nullptr;
        }
        pending = false;
        InvalidateRect(owner, nullptr, FALSE);
        UpdateWindow(owner);
        return false;
    };

    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (running) {
            bool hadWarmup = app.loadingWarmupPending;
            bool warmupStillPending = warmupOutput(app.renderer, hwnd, app.loadingOverlay,
                app.loadingWarmupPending, app.loadingWarmupStart, app.loadingWarmupAttempts);
            for (auto& clone : app.clones) {
                if (!clone || !clone->hwnd) continue;
                hadWarmup = hadWarmup || clone->loadingWarmupPending;
                warmupStillPending = warmupOutput(clone->renderer, clone->hwnd, clone->loadingOverlay,
                    clone->loadingWarmupPending, clone->loadingWarmupStart, clone->loadingWarmupAttempts) || warmupStillPending;
            }
            if (hadWarmup) {
                playbackLastTicks = GetTickCount64();
                if (warmupStillPending) continue;
                continue;
            }
            ULONGLONG now = GetTickCount64();
            float dt = std::min(0.05f, static_cast<float>(now - playbackLastTicks) / 1000.0f);
            playbackLastTicks = now;
            if (app.clones.empty()) {
                app.renderer.Tick();
            } else {
                app.renderer.TickFixed(dt);
                for (auto& clone : app.clones) {
                    if (clone && clone->hwnd) clone->renderer.TickFixed(dt);
                }
            }
            RedrawDebugSliceControls();
            Sleep(1);
        }
    }

    if (!app.preview) ShowCursor(TRUE);
    gApp = nullptr;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    return static_cast<int>(msg.wParam);
}

int RunSelfTest(HINSTANCE hInstance) {
    const wchar_t* cls = L"BackroomsMazeScreensaverSelfTest";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    auto appHolder = std::make_unique<App>();
    App& app = *appHolder;
    app.preview = true;
    gApp = &app;
    HWND hwnd = CreateWindowExW(0, cls, L"Backrooms Maze Self Test", WS_POPUP, 0, 0, 320, 180, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) {
        gApp = nullptr;
        return 2;
    }
    bool ok = app.renderer.Initialize(hwnd);
    StartupProfileLine(L"SelfTest after Initialize");
    app.renderer.SetPresentSyncInterval(0);
    app.renderer.SetPresentEnabled(false);
    StartupProfileLine(L"SelfTest before DestroyWindow");
    DestroyWindow(hwnd);
    StartupProfileLine(L"SelfTest after DestroyWindow");
    gApp = nullptr;
    appHolder.release();
    StartupProfileLine(L"SelfTest before return");
    return ok ? 0 : 3;
}

void ApplyDefaultGuiFont(HWND hwnd) {
    if (!hwnd) return;
    HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

int RunGame(HINSTANCE hInstance) {
    const wchar_t* cls = L"BackroomsMazeGameWindow";
    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = cls;
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    App app;
    app.preview = true;
    app.gameShell = true;
    app.gameInstance = hInstance;
    gApp = &app;

    Settings launchSettings = LoadSettings();
    int w = std::clamp(launchSettings.gameResolutionWidth, 640, 7680);
    int h = std::clamp(launchSettings.gameResolutionHeight, 360, 4320);
    int x = std::max(0, (GetSystemMetrics(SM_CXSCREEN) - w) / 2);
    int y = std::max(0, (GetSystemMetrics(SM_CYSCREEN) - h) / 2);
    DWORD style = launchSettings.gameFullscreen
        ? (WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS)
        : (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    if (launchSettings.gameFullscreen) {
        x = 0;
        y = 0;
        w = GetSystemMetrics(SM_CXSCREEN);
        h = GetSystemMetrics(SM_CYSCREEN);
    }
    HWND hwnd = CreateWindowExW(0, cls, L"Backrooms Maze", style,
        x, y, w, h, nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) {
        gApp = nullptr;
        return 1;
    }
    app.hwnd = hwnd;

    app.gameTitle = CreateWindowW(L"STATIC", L"Backrooms Maze", WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, 10, 10, hwnd, nullptr, hInstance, nullptr);
    app.gameSinglePlayer = CreateWindowW(L"BUTTON", L"Single Player", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameSinglePlayerId)), hInstance, nullptr);
    app.gameSettings = CreateWindowW(L"BUTTON", L"Settings", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameSettingsId)), hInstance, nullptr);
    app.gameDebug = CreateWindowW(L"BUTTON", L"Debug", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameDebugId)), hInstance, nullptr);
    app.gameExit = CreateWindowW(L"BUTTON", L"Exit", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameExitId)), hInstance, nullptr);
    app.gameBack = CreateWindowW(L"BUTTON", L"Back", WS_CHILD | BS_PUSHBUTTON,
        0, 0, 10, 10, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGameBackId)), hInstance, nullptr);

    app.debugPrevEffect = CreateWindowW(L"BUTTON", L"< Effect", WS_CHILD | BS_PUSHBUTTON,
        12, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevEffectId)), hInstance, nullptr);
    app.debugNextEffect = CreateWindowW(L"BUTTON", L"Effect >", WS_CHILD | BS_PUSHBUTTON,
        110, 10, 92, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextEffectId)), hInstance, nullptr);
    app.debugSize = CreateWindowW(L"BUTTON", L"Grid: 3x3", WS_CHILD | BS_PUSHBUTTON,
        210, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSizeId)), hInstance, nullptr);
    app.debugReset = CreateWindowW(L"BUTTON", L"Reset anim", WS_CHILD | BS_PUSHBUTTON,
        322, 10, 104, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugResetId)), hInstance, nullptr);
    app.debugPrevProp = CreateWindowW(L"BUTTON", L"< Prop", WS_CHILD | BS_PUSHBUTTON,
        434, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugPrevPropId)), hInstance, nullptr);
    app.debugNextProp = CreateWindowW(L"BUTTON", L"Prop >", WS_CHILD | BS_PUSHBUTTON,
        526, 10, 84, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugNextPropId)), hInstance, nullptr);
    app.debugSettings = CreateWindowW(L"BUTTON", L"Debug settings", WS_CHILD | BS_PUSHBUTTON,
        618, 10, 126, 28, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDebugSettingsId)), hInstance, nullptr);

    HWND controls[] = {
        app.gameTitle, app.gameSinglePlayer, app.gameSettings, app.gameDebug, app.gameExit, app.gameBack,
        app.debugPrevEffect, app.debugNextEffect, app.debugSize, app.debugReset, app.debugPrevProp, app.debugNextProp,
        app.debugSettings
    };
    for (HWND control : controls) ApplyDefaultGuiFont(control);

    LayoutGameControls(hwnd);
    SetDebugControlsVisible(false);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg{};
    bool running = true;
    ULONGLONG lastTicks = GetTickCount64();
    while (running) {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;

        ULONGLONG now = GetTickCount64();
        float dt = std::min(0.05f, static_cast<float>(now - lastTicks) / 1000.0f);
        lastTicks = now;

        if (app.rendererInitialized &&
            (app.gameState == GameState::PlayGame || app.gameState == GameState::DebugScene)) {
            if (app.gameState == GameState::PlayGame) {
                GameInputSnapshot input = CollectGameInput();
                app.renderer.SetGameInput(input);
            } else {
                app.renderer.SetGameInput({});
            }
            app.renderer.TickFixed(dt);
            if (app.gameState == GameState::DebugScene) RedrawDebugSliceControls();
        } else {
            Sleep(10);
        }
        Sleep(1);
    }

    ReleaseGameMouse();
    gApp = nullptr;
    gEffectDebugViewer = false;
    gBloodDebugEveryWall = false;
    return static_cast<int>(msg.wParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    SetProcessDPIAware();
#if defined(BACKROOMS_GAME_EXE)
    return RunGame(hInstance);
#else
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    HWND previewParent = nullptr;
    RunMode mode = ParseMode(argc, argv, previewParent);
    if (argv) LocalFree(argv);

    if (mode == RunMode::Configure) {
        ShowConfig(nullptr);
        return 0;
    }
    if (mode == RunMode::SelfTest) {
        return RunSelfTest(hInstance);
    }
    if (mode == RunMode::GenerateIni) {
        EnsureSettingsFile();
        return 0;
    }

    return RunScreensaver(hInstance, mode, previewParent);
#endif
}
