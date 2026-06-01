    ~Renderer() {
        audio_.Shutdown();
    }

    void ApplyBenchmarkDemoSettings(Settings& target) const {
        target.mazeWidth = 75;
        target.mazeHeight = 75;
        target.roomCount = 0;
        target.mazeSeed = 0xB345D123u;
        target.runVariation = 0.0f;
        target.mapOverlay = false;
        target.debugAiMapOverlay = false;
        target.lampOnRatio = 1.0f;
        target.lampFlickerRatio = 0.10f;
        target.brokenZoneRatio = 0.0f;
        target.sparkParticles = true;
        target.sparkEmitterRatio = 0.35f;
        target.sparkBurstMinSeconds = 0.20f;
        target.sparkBurstMaxSeconds = 0.42f;
        target.sparkMaxParticles = 1200;
        target.airParticles = true;
        target.airParticleDensity = 4.0f;
        target.airParticleSize = 1.0f;
        target.airParticleBlur = 1.0f;
        target.paperDensity = 4.0f;
        target.hallwayPaperRunDensity = 4.0f;
        target.chairDensity = 4.0f;
        target.metalCabinetDensity = 4.0f;
        target.waterDamageEnabled = true;
        target.waterDamageDensity = 4.0f;
        target.bloodSplatterDensity = 4.0f;
        target.bloodWorldFlicker = false;
        target.bloodWorldAlwaysOn = false;
        target.bloodWorldCoverage = 0.0f;
        target.bloodWorldFlickerMinSeconds = 1500.0f;
        target.bloodWorldFlickerMaxSeconds = 4800.0f;
        target.bloodWorldFlickerDuration = 0.35f;
        target.bloodWorldFlickerIntensity = 0.0f;
        target.fleshFlicker = false;
        target.fleshAlwaysOn = false;
        target.fleshFlickerMinSeconds = 1500.0f;
        target.fleshFlickerMaxSeconds = 4800.0f;
        target.fleshFlickerDuration = 0.35f;
        target.fleshFlickerIntensity = 0.0f;
        target.jumpscareFrequency = 0.0f;
        target.flashlightIntensity = std::max(target.flashlightIntensity, 1.0f);
        target.flashlightShadows = true;
        target.flashlightShadowDistanceMeters = std::max(target.flashlightShadowDistanceMeters, 18.0f);
        target.fogStartMeters = 0.0f;
        target.fogEndMeters = 36.0f;
        target.fogDarkness = 1.0f;
        target.monsterIgnorePlayer = true;
        target.debugInvincible = true;
    }

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
        startupProgressFineCurrent_ = 0;
        startupProgressFineTotal_ = startupProgressTotal_ * kStartupProgressUnitsPerStep;
        startupShaderDone_ = 0;
        startupShaderTotal_ = 0;
        startupShaderCompiled_ = 0;
        startupShaderCached_ = 0;
        lastInitializeError_.clear();
        ReportStartupActivity(L"Starting renderer", L"Reading settings and preparing Direct3D.");

        StartupProfile profile(L"Initialize");
        settings_ = settingsOverride ? *settingsOverride : LoadSettings();
        if (BenchmarkDemoEnabled()) ApplyBenchmarkDemoSettings(settings_);
        PrepareAudio(settings_);
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
        if (FAILED(hr)) {
            lastInitializeError_ = L"CreateDeviceAndSwapChain failed: HRESULT 0x" + [&]() {
                std::wstringstream ss;
                ss << std::hex << static_cast<unsigned long>(hr);
                return ss.str();
            }();
            return false;
        }
        profile.Mark(L"CreateDeviceAndSwapChain");
        startupShaderTotal_ = featureLevel_ >= D3D_FEATURE_LEVEL_11_0 ? 9 : 7;
        startupProgressTotal_ = kStartupProgressPreShaderSteps + startupShaderTotal_ + kStartupProgressPostShaderSteps;
        startupProgressFineTotal_ = startupProgressTotal_ * kStartupProgressUnitsPerStep;
        ReportStartupStep(L"Direct3D device ready", L"Creating render targets.");

        if (!CreateBackBuffer()) {
            lastInitializeError_ = L"CreateBackBuffer failed.";
            return false;
        }
        profile.Mark(L"CreateBackBuffer");
        ReportStartupStep(L"Back buffer ready", L"Checking shader cache.");
        ReportStartupActivity(L"Loading shaders", ShaderProgressDetail(L"Checking shader cache", nullptr, nullptr, false));
        if (!CreateShaders()) {
            lastInitializeError_ = L"CreateShaders failed.";
            return false;
        }
        profile.Mark(L"CreateShaders");
        ReportStartupActivity(L"Shaders ready", L"Creating render states.");
        if (!CreateStates()) {
            lastInitializeError_ = L"CreateStates failed.";
            return false;
        }
        profile.Mark(L"CreateStates");
        ReportStartupStep(L"Render states ready", L"Allocating shadow map.");
        if (!CreateShadowResources()) {
            lastInitializeError_ = L"CreateShadowResources failed.";
            return false;
        }
        profile.Mark(L"CreateShadowResources");
        ReportStartupStep(L"Shadow resources ready", L"Building material textures.");
        ReportStartupActivity(L"Loading textures", L"Checking texture cache.");
        if (!CreateTextures()) {
            lastInitializeError_ = L"CreateTextures failed.";
            return false;
        }
        profile.Mark(L"CreateTextures");
        ReportStartupStep(L"Textures ready", L"Loading flashlight pattern.");
        if (!CreateFlashlightPatternTexture()) {
            lastInitializeError_ = L"CreateFlashlightPatternTexture failed.";
            return false;
        }
        profile.Mark(L"CreateFlashlightPatternTexture");
        ReportStartupStep(L"Flashlight pattern ready", L"Creating constant buffers.");
        if (!CreateConstantBuffer()) {
            lastInitializeError_ = L"CreateConstantBuffer failed.";
            return false;
        }
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
        } else if (BenchmarkDemoEnabled()) {
            maze_.GenerateBenchmarkDemo();
        } else {
            maze_.Generate(settings_);
        }
        profile.Mark(L"GenerateMaze");
        ReportStartupStep(L"Maze generated", L"Uploading maze mask.");
        if (!CreateMazeMaskTexture()) {
            lastInitializeError_ = L"CreateMazeMaskTexture failed.";
            return false;
        }
        profile.Mark(L"CreateMazeMaskTexture");
        ReportStartupStep(L"Maze mask ready", L"Building maze geometry.");
        ResetSimulation();
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) {
            menuDarkLayerOneRun_ = false;
        }
        CreateMazeMesh();
        if (runtimeMode_ == RendererRuntimeMode::MainMenu) SetupMainMenuScene();
        SetupPersistentAudioEmitters();
        ResetDebugSliceLoopState();
        profile.Mark(L"CreateMazeMesh");
        ReportStartupStep(L"Ready", L"Entering maze.");
        lastTicks_ = GetTickCount64();
        bloodDebugStartTicks_ = lastTicks_;
        return true;
    }

    const std::wstring& LastInitializeError() const {
        return lastInitializeError_;
    }

    bool PrepareAudio(const Settings& settings) {
        audioReady_ = audio_.Initialize(settings);
        if (audioReady_ && !audioSamplesLoaded_) {
            audio_.LoadAll(settings);
            audioSamplesLoaded_ = true;
        }
        return audioReady_;
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
        const bool runtimeProfile = RuntimeProfileEnabled();
        double profileStart = 0.0;
        double profileAfterProgress = 0.0;
        double profileAfterBudget = 0.0;
        double profileAfterSimulation = 0.0;
        double profileAfterAudio = 0.0;
        if (runtimeProfile) profileStart = ProfileNowMs();
        time_ += dt;
        UpdatePlayableProgressionTimers(dt);
        if (runtimeProfile) profileAfterProgress = ProfileNowMs();
        UpdateAirParticlePerformanceBudget(dt);
        if (runtimeProfile) profileAfterBudget = ProfileNowMs();
        UpdateSimulation(dt);
        if (runtimeProfile) profileAfterSimulation = ProfileNowMs();
        UpdateAudio(dt);
        if (runtimeProfile) profileAfterAudio = ProfileNowMs();
        Render();
        if (runtimeProfile) {
            const double profileEnd = ProfileNowMs();
            std::wostringstream csv;
            csv << std::fixed << std::setprecision(3)
                << gpuProfileFrameCounter_ << L","
                << static_cast<int>(runtimeMode_) << L","
                << (profileEnd - profileStart) << L","
                << (profileAfterProgress - profileStart) << L","
                << (profileAfterBudget - profileAfterProgress) << L","
                << (profileAfterSimulation - profileAfterBudget) << L","
                << (profileAfterAudio - profileAfterSimulation) << L","
                << (profileEnd - profileAfterAudio) << L","
                << indexCount_ << L","
                << instancedIndexCount_ << L","
                << instancedInstanceCount_ << L","
                << floorCeilingIndexCount_ << L","
                << staticWaterIndexCount_ << L","
                << staticTransparentIndexCount_ << L","
                << dynamicOpaqueVertexCount_ << L","
                << dynamicTransparentVertexCount_ << L","
                << airParticles_.size() << L","
                << sparks_.size() << L","
                << steam_.size() << L","
                << runtimeLamps_.size();
            RuntimeProfileFrameLine(csv.str());
        }
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
        menuDarkLayerOneRun_ = false;
        maze_.w = settings_.mazeWidth;
        maze_.h = settings_.mazeHeight;
        maze_.tileW = settings_.tileWidthMeters;
        maze_.tileD = settings_.tileLengthMeters;
        maze_.GenerateMenuRoom();
        CreateMazeMaskTexture();
        ResetSimulation();
        CreateMazeMesh();
        SetupMainMenuScene();
        SetupPersistentAudioEmitters();
        InvalidateRect(hwnd_, nullptr, FALSE);
    }

    void EnterPausedMainMenuScene() {
        if (runtimeMode_ == RendererRuntimeMode::PlayableGame) {
            SavePlayableSnapshot();
        }
        EnterMainMenuScene();
    }

    bool RestorePausedGameRun() {
        if (!pausedPlayableSnapshot_) return false;
        RestorePlayableSnapshot();
        pausedPlayableSnapshot_.reset();
        return true;
    }

    void SetMenuInteraction(float pointerX, float pointerY, bool buttonHover, bool exitHover, bool singlePlayerHover) {
        if (menuStartTransitionActive_ || menuStartTransitionComplete_) return;
        menuPointerTargetX_ = Clamp01(pointerX);
        menuPointerTargetY_ = Clamp01(pointerY);
        menuButtonHover_ = buttonHover;
        menuExitHover_ = exitHover;
        menuSinglePlayerHover_ = buttonHover && singlePlayerHover;
        menuHoverButtonIndex_ = -1;
        if (buttonHover) {
            if (menuSinglePlayerHover_) menuHoverButtonIndex_ = 0;
        }
    }

    void TriggerMainMenuLampBurst() {
        if (!menuDarkLayerOneRun_) return;
        if (menuLampBurstPlayed_) return;
        menuLampBurstPending_ = true;
    }

    void BeginMainMenuStartTransition() {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return;
        menuStartTransitionActive_ = true;
        menuStartTransitionComplete_ = false;
        menuStartTransitionFromCustomView_ = menuCustomViewActive_ || menuCustomViewTarget_;
        menuStartTransitionTimer_ = 0.0f;
        menuStartTransitionFade_ = 0.0f;
        menuStartCamera_ = camera_;
        menuStartYaw_ = yaw_;
        menuStartPitch_ = lookPitch_;
        menuCustomViewTarget_ = false;
        menuSinglePlayerHover_ = true;
        menuButtonHover_ = false;
        menuHoverButtonIndex_ = -1;
        if (menuDarkLayerOneRun_ && !menuLampBurstPlayed_) menuLampBurstPending_ = true;
    }

    bool MainMenuStartTransitionComplete() const {
        return menuStartTransitionComplete_;
    }

    bool MenuButtonScreenRect(int index, RECT& out) const {
        if (menuStartTransitionActive_ || menuStartTransitionComplete_) return false;
        if (runtimeMode_ != RendererRuntimeMode::MainMenu || index < 0 || index >= menuButtonCount_) return false;
        MenuPlaquePlacement plaque = MenuButtonPlacement(index);
        return ProjectMenuQuadToScreen(plaque.center, plaque.right, {0.0f, 1.0f, 0.0f}, plaque.halfW, plaque.halfH, out);
    }

    bool CustomGamePanelScreenRect(RECT& out) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return false;
        MenuPlaquePlacement panel = MenuCustomPanelPlacement();
        return ProjectMenuQuadToScreen(panel.center, panel.right, {0.0f, 1.0f, 0.0f}, panel.halfW, panel.halfH, out);
    }

    bool CustomGameControlScreenRect(int control, RECT& out) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return false;
        MenuPlaquePlacement placement{};
        if (!CustomGameControlPlacement(static_cast<CustomGameMenuControl>(control), placement)) return false;
        return ProjectMenuQuadToScreen(placement.center, placement.right, {0.0f, 1.0f, 0.0f}, placement.halfW, placement.halfH, out);
    }

    bool MenuExitDoorScreenRect(RECT& out) const {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return false;
        XMFLOAT3 center = Add3(exitDoorCenter_, Scale3(exitDoorNormal_, 0.035f));
        return ProjectMenuQuadToScreen(center, exitDoorRight_, {0.0f, 1.0f, 0.0f}, 0.71f, 1.12f, out);
    }

    void SetMenuHoverButtonIndex(int index) {
        menuHoverButtonIndex_ = index;
        menuButtonHover_ = index >= 0;
        menuSinglePlayerHover_ = index >= 0 &&
            index < menuButtonCount_ &&
            menuButtonLabelRows_[static_cast<size_t>(index)] == 0;
    }

    void SetMenuResumeLabel(bool resume) {
        menuResumeLabel_ = resume;
    }

    void SetMenuButtonLayout(bool canResumeCurrent, bool canResumeSaved) {
        menuButtonCount_ = 0;
        auto add = [&](int labelRow) {
            if (menuButtonCount_ >= static_cast<int>(menuButtonLabelRows_.size())) return;
            menuButtonLabelRows_[static_cast<size_t>(menuButtonCount_++)] = labelRow;
        };
        if (canResumeCurrent) add(1);
        if (canResumeSaved) add(2);
        add(0);
        add(3);
        add(4);
        add(5);
        menuResumeLabel_ = canResumeCurrent;
    }

    void SetMainMenuCustomGameView(bool open) {
        if (runtimeMode_ != RendererRuntimeMode::MainMenu) return;
        if (open == menuCustomViewTarget_ && open == menuCustomViewActive_) return;
        menuCustomViewTarget_ = open;
        if (open) {
            menuCustomViewActive_ = true;
            menuCustomViewTimer_ = 0.0f;
            menuCustomStartCamera_ = camera_;
            menuCustomStartYaw_ = yaw_;
            menuCustomStartPitch_ = lookPitch_;
            menuButtonHover_ = false;
            menuExitHover_ = false;
            menuSinglePlayerHover_ = false;
            menuHoverButtonIndex_ = -1;
        } else {
            menuCustomReturnTimer_ = 0.0f;
            menuCustomReturnCamera_ = camera_;
            menuCustomReturnYaw_ = yaw_;
            menuCustomReturnPitch_ = lookPitch_;
        }
    }

    bool MainMenuCustomGameViewVisible() const {
        return runtimeMode_ == RendererRuntimeMode::MainMenu && (menuCustomViewActive_ || menuCustomViewTarget_);
    }

    void SetCustomGameMenuState(const CustomGameSpec& spec, int hoverControl, int selectedScare = -1) {
        int clampedHover = std::clamp(hoverControl, 0, static_cast<int>(CustomGameMenuControl::Back));
        int clampedSelectedScare = std::clamp(selectedScare, -2, CustomGameSpec::kScareTypeCount - 1);
        bool changed =
            customMenuSpec_.layer != spec.layer ||
            customMenuSpec_.mazeWidth != spec.mazeWidth ||
            customMenuSpec_.mazeHeight != spec.mazeHeight ||
            customMenuSpec_.roomCount != spec.roomCount ||
            customMenuSpec_.brokenLampScares != spec.brokenLampScares ||
            customMenuSpec_.airVentScares != spec.airVentScares ||
            customMenuSpec_.waterScares != spec.waterScares ||
            customMenuSpec_.bloodWorldScares != spec.bloodWorldScares ||
            customMenuSpec_.fleshWorldScares != spec.fleshWorldScares ||
            customMenuSpec_.omukadeBoss != spec.omukadeBoss ||
            customMenuSpec_.eightPages != spec.eightPages ||
            customMenuSpec_.mapDirtPercent != spec.mapDirtPercent ||
            customMenuSpec_.paperDensityPercent != spec.paperDensityPercent ||
            customMenuSpec_.propDensityPercent != spec.propDensityPercent ||
            customMenuSpec_.lampOnPercent != spec.lampOnPercent ||
            customMenuSpec_.lampFlickerPercent != spec.lampFlickerPercent ||
            customMenuSpec_.lampSparkPercent != spec.lampSparkPercent ||
            customMenuSpec_.fogStartMeters != spec.fogStartMeters ||
            customMenuSpec_.fogEndMeters != spec.fogEndMeters ||
            customMenuSpec_.fogDarknessPercent != spec.fogDarknessPercent ||
            customMenuSpec_.jumpscareChancePercent != spec.jumpscareChancePercent ||
            customMenuSpec_.jumpscareStartMinSeconds != spec.jumpscareStartMinSeconds ||
            customMenuSpec_.jumpscareStartMaxSeconds != spec.jumpscareStartMaxSeconds ||
            customMenuSpec_.scareChancePercent != spec.scareChancePercent ||
            customMenuSpec_.scareStartMinSeconds != spec.scareStartMinSeconds ||
            customMenuSpec_.scareStartMaxSeconds != spec.scareStartMaxSeconds ||
            customMenuHoverControl_ != clampedHover ||
            customMenuSelectedScare_ != clampedSelectedScare;
        customMenuSpec_ = spec;
        customMenuHoverControl_ = clampedHover;
        customMenuSelectedScare_ = clampedSelectedScare;
        customMenuTextureDirty_ = customMenuTextureDirty_ || changed;
    }

    void SetGameInput(const GameInputSnapshot& input) {
        gameInput_ = input;
    }

    bool LoadSavedGameRun() {
        return LoadSavedRunFromFile();
    }

    bool SavedGameRunExists() const {
        return SavedRunExists();
    }

    bool PlayableRunFinished() const {
        return runtimeMode_ == RendererRuntimeMode::PlayableGame && playableRun_.runFinished;
    }

    void DeleteSavedGameRun() {
        DeleteSavedRun();
    }

    void RestartCustomGameRun(const CustomGameSpec& customSpec) {
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        runtimeMode_ = RendererRuntimeMode::PlayableGame;
        EnsureFullSceneAssets();
        settings_ = gameplaySettings_;
        menuStartTransitionActive_ = false;
        menuStartTransitionComplete_ = false;
        menuStartTransitionFromCustomView_ = false;
        menuStartTransitionTimer_ = 0.0f;
        menuStartTransitionFade_ = 0.0f;
        menuCustomViewTarget_ = false;
        menuCustomViewActive_ = false;
        flashlightEnabled_ = true;
        previousFlashlightInput_ = false;
        BeginCustomPlayableRun(customSpec);
    }

    void ShowGameNotification(const std::wstring& text, float durationSeconds = 4.2f) {
        hudNotificationText_ = text;
        hudNotificationStartTime_ = time_;
        hudNotificationDuration_ = std::max(0.25f, durationSeconds);
        hudNotificationTextureDirty_ = true;
    }

    void ApplyGameSettings(const Settings& settings) {
        auto applyLive = [&](Settings& target) {
            target.gameFullscreen = settings.gameFullscreen;
            target.gameResolutionWidth = settings.gameResolutionWidth;
            target.gameResolutionHeight = settings.gameResolutionHeight;
            target.allowWarpFallback = settings.allowWarpFallback;
            target.mapOverlay = settings.mapOverlay;
            target.debugAiMapOverlay = settings.debugAiMapOverlay;
            target.debugInfiniteStamina = settings.debugInfiniteStamina;
            target.debugInvincible = settings.debugInvincible;
            target.monsterIgnorePlayer = settings.monsterIgnorePlayer;
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
        audio_.ApplySettings(settings_);
    }

    void EnableInfiniteStaminaCheat() {
        gameplaySettings_.debugInfiniteStamina = true;
        settings_.debugInfiniteStamina = true;
        playerStamina_ = 100.0f;
    }

    bool MonsterIgnoresPlayer() const {
        return !MonsterActiveForCurrentMode() || (settings_.monsterIgnorePlayer && IsPlayableSimulationMode(runtimeMode_));
    }

    bool MonsterActiveForCurrentMode() const {
        if (monsterPreview_ || gEffectDebugViewer) return true;
        if (runtimeMode_ == RendererRuntimeMode::PlayableGame) {
            return deathActive_ || (playableRun_.active && playableRun_.levelRunning && playableRun_.currentLevel.bossEncounter);
        }
        return true;
    }

    void RestartGameRun() {
        gEffectDebugViewer = false;
        gBloodDebugEveryWall = false;
        runtimeMode_ = RendererRuntimeMode::PlayableGame;
        EnsureFullSceneAssets();
        settings_ = gameplaySettings_;
        menuStartTransitionActive_ = false;
        menuStartTransitionComplete_ = false;
        menuStartTransitionFromCustomView_ = false;
        menuStartTransitionTimer_ = 0.0f;
        menuStartTransitionFade_ = 0.0f;
        flashlightEnabled_ = true;
        previousFlashlightInput_ = false;
        BeginPlayableRun();
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
        SetupPersistentAudioEmitters();
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
