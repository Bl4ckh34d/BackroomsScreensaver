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

#include "debug/effect_debug.inl"

#include "render/render_types.inl"

#include "core/math_utils.inl"

#include "config/settings.inl"

#include "maze/maze.inl"

enum class MonsterPreviewView {
    Orbit,
    Front,
    Side,
    LeftSide,
    Top
};

struct MenuPlaquePlacement {
    XMFLOAT3 center{};
    XMFLOAT3 right{1.0f, 0.0f, 0.0f};
    XMFLOAT3 inward{0.0f, 0.0f, -1.0f};
    float halfW = 0.72f;
    float halfH = 0.122f;
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
        ReportStartupStep(L"GPU buffers ready", runtimeMode_ == RendererRuntimeMode::MainMenu
            ? L"Loading menu meshes."
            : L"Loading monster mesh.");

        runtimeSeed_ = ResolveRuntimeSeed(settings_.mazeSeed);
        ApplyRuntimeVariation(settings_, runtimeSeed_);
        gameplaySettings_ = settings_;
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) ApplyMainMenuSettings();
        if (gEffectDebugViewer) ApplyDebugSliceSettings();
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            LoadMenuPropMeshes();
            profile.Mark(L"LoadMenuPropMeshes");
            ReportStartupStep(L"Menu meshes ready", L"Generating menu layout.");
        } else {
            EnsureFullSceneAssets();
            profile.Mark(L"LoadSceneAssets");
            ReportStartupStep(L"Scene meshes ready", L"Generating maze layout.");
        }
        profile.Mark(L"LoadPropMeshes");
        maze_.rng.seed(runtimeSeed_);
        rng_.seed(runtimeSeed_ ^ 0x9e3779b9u);

        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.exit = {maze_.w - 2, maze_.h - 2};
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            maze_.GenerateMenuRoom();
        } else if (gEffectDebugViewer) {
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
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) SetupMainMenuScene();
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

    void EnterMainMenuScene() {
        runtimeMode_ = RendererRuntimeMode::MainMenu;
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        settings_ = gameplaySettings_;
        ApplyMainMenuSettings();
        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.GenerateMenuRoom();
        CreateMazeMaskTexture();
        ResetSimulation();
        CreateMazeMesh();
        SetupMainMenuScene();
        InvalidateRect(hwnd_, nullptr, FALSE);
    }

    void SetMenuInteraction(float pointerX, float pointerY, bool buttonHover, bool exitHover, bool singlePlayerHover) {
        menuPointerTargetX_ = Clamp01(pointerX);
        menuPointerTargetY_ = Clamp01(pointerY);
        menuButtonHover_ = buttonHover;
        menuExitHover_ = exitHover;
        menuSinglePlayerHover_ = singlePlayerHover;
        menuHoverButtonIndex_ = -1;
        if (buttonHover) {
            if (singlePlayerHover) menuHoverButtonIndex_ = 0;
        }
    }

    void TriggerMainMenuLampBurst() {
        menuLampBurstPending_ = true;
    }

    bool MenuButtonScreenRect(int index, RECT& out) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu || index < 0 || index >= 3) return false;
        MenuPlaquePlacement plaque = MenuButtonPlacement(index);
        return ProjectMenuQuadToScreen(plaque.center, plaque.right, {0.0f, 1.0f, 0.0f}, plaque.halfW, plaque.halfH, out);
    }

    bool MenuExitDoorScreenRect(RECT& out) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return false;
        XMFLOAT3 center = Add3(exitDoorCenter_, Scale3(exitDoorNormal_, 0.035f));
        return ProjectMenuQuadToScreen(center, exitDoorRight_, {0.0f, 1.0f, 0.0f}, 0.71f, 1.12f, out);
    }

    void SetMenuHoverButtonIndex(int index) {
        menuHoverButtonIndex_ = index;
        menuButtonHover_ = index >= 0;
        menuSinglePlayerHover_ = index == 0;
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
            target.mapOverlay = settings.mapOverlay;
            target.debugAiMapOverlay = settings.debugAiMapOverlay;
            target.exposure = settings.exposure;
            target.bloomAmount = settings.bloomAmount;
            target.motionBlurAmount = settings.motionBlurAmount;
            target.airParticleDensity = settings.airParticleDensity;
            target.mouseSensitivity = settings.mouseSensitivity;
            target.invertMouseY = settings.invertMouseY;
            target.gameKeyBindings = settings.gameKeyBindings;
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
        EnsureFullSceneAssets();
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
        EnsureFullSceneAssets();
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

    void EnsureFullSceneAssets() {
        if (!monsterMeshLoaded_ || skullMesh_.empty()) {
            LoadMonsterSkullMesh();
        }
        if (!propMeshesLoaded_) {
            LoadPropMeshes();
        }
    }

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
    bool monsterMeshLoaded_ = false;
    bool propMeshesLoaded_ = false;
    bool menuPropMeshesLoaded_ = false;
    RendererRuntimeMode runtimeMode_ = RendererRuntimeMode::ScreensaverAutopilot;
    GameInputSnapshot gameInput_{};
    Settings gameplaySettings_{};
    float menuPointerX_ = 0.5f;
    float menuPointerY_ = 0.5f;
    float menuPointerTargetX_ = 0.5f;
    float menuPointerTargetY_ = 0.5f;
    float menuBaseYaw_ = 0.0f;
    float menuBasePitch_ = 0.0f;
    float menuDoorOpen_ = 0.0f;
    float menuBloodAmount_ = 0.0f;
    bool menuButtonHover_ = false;
    bool menuExitHover_ = false;
    bool menuSinglePlayerHover_ = false;
    bool menuLampBurstPending_ = false;
    int menuHoverButtonIndex_ = -1;
    float playerHealth_ = 100.0f;
    float playerStamina_ = 100.0f;
    float playerVerticalOffset_ = 0.0f;
    float playerVerticalVelocity_ = 0.0f;
    float playerStaminaRegenDelay_ = 0.0f;
    float playerNoiseRadiusMeters_ = 0.0f;
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
    bool monsterHeardPlayerNow_ = false;
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

    void ApplyMainMenuSettings() {
        settings_.mazeWidth = 3;
        settings_.mazeHeight = 3;
        settings_.wallHeightMeters = std::max(settings_.wallHeightMeters, 2.85f);
        settings_.mapOverlay = false;
        settings_.debugAiMapOverlay = false;
        settings_.chairDensity = 0.0f;
        settings_.paperDensity = 0.0f;
        settings_.hallwayPaperRunDensity = 0.0f;
        settings_.metalCabinetDensity = 0.0f;
        settings_.waterDamageDensity = 0.0f;
        settings_.lampOnRatio = 1.0f;
        settings_.lampSpacing = std::max(settings_.tileWidthMeters, settings_.tileLengthMeters) * 2.4f;
        settings_.lampIntensity = std::max(settings_.lampIntensity, 1.05f);
        settings_.lampFlickerRatio = 0.0f;
        settings_.brokenZoneRatio = 0.0f;
        settings_.ambientLight = std::max(settings_.ambientLight, 0.105f);
        settings_.flashlightIntensity = std::max(settings_.flashlightIntensity, 1.85f);
        settings_.flashlightAttenuation = std::min(settings_.flashlightAttenuation, 0.070f);
        settings_.flashlightConeDegrees = std::max(settings_.flashlightConeDegrees, 92.0f);
        settings_.airParticles = true;
        settings_.airParticleDensity = std::max(0.32f, settings_.airParticleDensity * 0.55f);
        settings_.sparkParticles = true;
        settings_.fadeInSeconds = 0.0f;
        settings_.bloodWorldCoverage = std::max(settings_.bloodWorldCoverage, 0.45f);
        settings_.bloodWorldAlwaysOn = false;
        settings_.bloodWorldFlickerIntensity = 0.0f;
        settings_.fogStartMeters = std::max(settings_.fogStartMeters, 4.2f);
        settings_.fogEndMeters = std::max(settings_.fogEndMeters, 9.5f);
    }

    MenuPlaquePlacement MenuButtonPlacement(int index) const {
        XMFLOAT3 c = maze_.WorldCenter(maze_.start, 0.0f);
        const float northWallZ = c.z + maze_.tileD * 0.5f - 0.034f;
        MenuPlaquePlacement plaque{};
        plaque.halfW = std::min(maze_.tileW * 0.86f, 1.42f);
        plaque.halfH = 0.168f;
        plaque.center = {c.x + maze_.tileW * 0.58f, 1.73f - static_cast<float>(index) * 0.39f, northWallZ};
        plaque.right = {1.0f, 0.0f, 0.0f};
        plaque.inward = {0.0f, 0.0f, -1.0f};
        return plaque;
    }

    bool ProjectMenuQuadToScreen(XMFLOAT3 center, XMFLOAT3 right, XMFLOAT3 up, float halfW, float halfH, RECT& out) const {
        if (width_ <= 0 || height_ <= 0) return false;
        right = Normalize3(right, {1.0f, 0.0f, 0.0f});
        up = Normalize3(up, {0.0f, 1.0f, 0.0f});

        XMFLOAT3 f = Forward();
        XMVECTOR eye = XMLoadFloat3(&camera_);
        XMVECTOR worldUp = XMVectorSet(0, 1, 0, 0);
        XMVECTOR viewDir = XMVector3Normalize(XMVectorSet(f.x, lookPitch_, f.z, 0.0f));
        XMMATRIX view = XMMatrixLookAtLH(eye, eye + viewDir, worldUp);
        float aspect = static_cast<float>(std::max<LONG>(1, width_)) / static_cast<float>(std::max<LONG>(1, height_));
        float fovDegrees = runtimeMode_ == RendererRuntimeMode::MainMenu ? 84.0f : 70.0f;
        XMMATRIX proj = XMMatrixPerspectiveFovLH(fovDegrees * kPi / 180.0f, aspect, 0.045f, 42.0f);
        XMMATRIX viewProj = view * proj;

        auto p = [&](float x, float y) {
            return Add3(center, Add3(Scale3(right, x * halfW), Scale3(up, y * halfH)));
        };
        std::array<XMFLOAT3, 4> corners = {{
            p(-1.0f, -1.0f),
            p( 1.0f, -1.0f),
            p( 1.0f,  1.0f),
            p(-1.0f,  1.0f)
        }};

        float minX = static_cast<float>(width_);
        float minY = static_cast<float>(height_);
        float maxX = 0.0f;
        float maxY = 0.0f;
        int visible = 0;
        for (const XMFLOAT3& corner : corners) {
            XMVECTOR clip = XMVector3TransformCoord(XMLoadFloat3(&corner), viewProj);
            XMFLOAT3 ndc{};
            XMStoreFloat3(&ndc, clip);
            if (ndc.z < 0.0f || ndc.z > 1.0f) continue;
            float sx = (ndc.x * 0.5f + 0.5f) * static_cast<float>(width_);
            float sy = (0.5f - ndc.y * 0.5f) * static_cast<float>(height_);
            minX = std::min(minX, sx);
            minY = std::min(minY, sy);
            maxX = std::max(maxX, sx);
            maxY = std::max(maxY, sy);
            ++visible;
        }
        if (visible < 3) return false;

        constexpr LONG pad = 4;
        out.left = std::clamp(static_cast<LONG>(std::floor(minX)) - pad, 0L, static_cast<LONG>(width_));
        out.top = std::clamp(static_cast<LONG>(std::floor(minY)) - pad, 0L, static_cast<LONG>(height_));
        out.right = std::clamp(static_cast<LONG>(std::ceil(maxX)) + pad, 0L, static_cast<LONG>(width_));
        out.bottom = std::clamp(static_cast<LONG>(std::ceil(maxY)) + pad, 0L, static_cast<LONG>(height_));
        return out.right > out.left && out.bottom > out.top;
    }

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
        const char* version = "BackroomsMazeTextureCacheV11";
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

    #include "render/renderer_mesh_loading.inl"

    #include "render/renderer_shaders.inl"

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

    #include "render/renderer_textures.inl"

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

    #include "render/renderer_maze_mesh.inl"

    #include "monster/monster_ai.inl"

    #include "gameplay/scare_effect_events.inl"

    #include "gameplay/renderer_update.inl"

    #include "render/renderer_dynamic_geometry.inl"

    #include "render/renderer_overlays.inl"

    #include "render/renderer_present.inl"
};

#include "app/app_state.inl"

enum class ConfigDialogMode {
    Full,
    Game,
    Debug
};
void ShowConfig(HWND owner, ConfigDialogMode mode = ConfigDialogMode::Full);
HWND CreateEmbeddedConfig(HWND parent, ConfigDialogMode mode);
#if defined(BACKROOMS_GAME_EXE)
HWND CreateGameSettingsPanel(HWND parent);
#endif
HWND CreateLoadingOverlay(HWND parent, HINSTANCE hInstance, bool brandedSplash = false);
void SetLoadingOverlayStatus(HWND overlay, const wchar_t* phase, const wchar_t* detail, bool complete);
void LoadingProgressCallback(void* context, const StartupProgressUpdate& update);

#if defined(BACKROOMS_GAME_EXE)
#include "game/game_shell.inl"
#endif

#include "platform/loading_overlay.inl"

#include "platform/window_proc.inl"

#include "config/config_dialog.inl"

#if defined(BACKROOMS_GAME_EXE)
#include "game/game_app.inl"
#else
#include "screensaver/screensaver_app.inl"
#endif

} // namespace

#include "app/entry_point.inl"
