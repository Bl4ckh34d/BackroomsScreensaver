    bool Initialize(HWND hwnd, const Settings* settingsOverride = nullptr, bool monsterPreview = false,
                    MonsterPreviewView monsterPreviewView = MonsterPreviewView::Orbit,
                    const StartupProgressSink* startupProgress = nullptr) {
        struct StartupProgressScope {
            Renderer* renderer = nullptr;
            ~StartupProgressScope() {
                if (renderer) renderer->startupRuntime_.sink = nullptr;
            }
        } progressScope{this};
        startupRuntime_.sink = startupProgress;
        startupRuntime_.step = 0;
        startupRuntime_.total = kStartupProgressPreShaderSteps + 9 + kStartupProgressPostShaderSteps;
        startupRuntime_.fineCurrent = 0;
        startupRuntime_.fineTotal = startupRuntime_.total * kStartupProgressUnitsPerStep;
        startupRuntime_.shaderDone = 0;
        startupRuntime_.shaderTotal = 0;
        startupRuntime_.shaderCompiled = 0;
        startupRuntime_.shaderCached = 0;
        startupRuntime_.lastInitializeError.clear();
        ReportStartupActivity(L"Starting renderer", L"Reading settings and preparing Direct3D.");

        StartupProfile profile(L"Initialize");
        settingsRuntime_.live = settingsOverride ? *settingsOverride : LoadSettings();
        if (BenchmarkDemoEnabled()) ApplyBenchmarkDemoSettings(settingsRuntime_.live);
        PrepareAudio(settingsRuntime_.live);
        monsterPreview_.active = monsterPreview;
        monsterPreview_.view = monsterPreviewView;
        sessionRuntime_.mode = monsterPreview_.active ? RendererRuntimeMode::Preview :
            (gEffectDebugViewer ? RendererRuntimeMode::DebugViewer : sessionRuntime_.mode);
        profile.Mark(L"LoadSettings");
        ReportStartupStep(L"Settings loaded", L"Creating Direct3D device.");
        hostRuntime_.hwnd = hwnd;
        RECT rc{};
        GetClientRect(hostRuntime_.hwnd, &rc);
        hostRuntime_.width = std::max(1L, rc.right - rc.left);
        hostRuntime_.height = std::max(1L, rc.bottom - rc.top);

        DXGI_SWAP_CHAIN_DESC scd{};
        scd.BufferCount = 2;
        scd.BufferDesc.Width = static_cast<UINT>(hostRuntime_.width);
        scd.BufferDesc.Height = static_cast<UINT>(hostRuntime_.height);
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = hostRuntime_.hwnd;
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
            &scd, &d3dRuntime_.swapChain, &d3dRuntime_.device, &d3dRuntime_.featureLevel, &d3dRuntime_.context);
        if (FAILED(hr) && settingsRuntime_.live.allowWarpFallback) {
            hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                levels, ARRAYSIZE(levels), D3D11_SDK_VERSION,
                &scd, &d3dRuntime_.swapChain, &d3dRuntime_.device, &d3dRuntime_.featureLevel, &d3dRuntime_.context);
        }
        if (FAILED(hr)) {
            startupRuntime_.lastInitializeError = L"CreateDeviceAndSwapChain failed: HRESULT 0x" + [&]() {
                std::wstringstream ss;
                ss << std::hex << static_cast<unsigned long>(hr);
                return ss.str();
            }();
            return false;
        }
        profile.Mark(L"CreateDeviceAndSwapChain");
        startupRuntime_.shaderTotal = d3dRuntime_.featureLevel >= D3D_FEATURE_LEVEL_11_0 ? 9 : 7;
        startupRuntime_.total = kStartupProgressPreShaderSteps + startupRuntime_.shaderTotal + kStartupProgressPostShaderSteps;
        startupRuntime_.fineTotal = startupRuntime_.total * kStartupProgressUnitsPerStep;
        ReportStartupStep(L"Direct3D device ready", L"Creating render targets.");

        if (!CreateBackBuffer()) {
            startupRuntime_.lastInitializeError = L"CreateBackBuffer failed.";
            return false;
        }
        profile.Mark(L"CreateBackBuffer");
        ReportStartupStep(L"Back buffer ready", L"Checking shader cache.");
        ReportStartupActivity(L"Loading shaders", ShaderProgressDetail(L"Checking shader cache", nullptr, nullptr, false));
        if (!CreateShaders()) {
            startupRuntime_.lastInitializeError = L"CreateShaders failed.";
            return false;
        }
        profile.Mark(L"CreateShaders");
        ReportStartupActivity(L"Shaders ready", L"Creating render states.");
        if (!CreateStates()) {
            startupRuntime_.lastInitializeError = L"CreateStates failed.";
            return false;
        }
        profile.Mark(L"CreateStates");
        ReportStartupStep(L"Render states ready", L"Allocating shadow map.");
        if (!CreateShadowResources()) {
            startupRuntime_.lastInitializeError = L"CreateShadowResources failed.";
            return false;
        }
        profile.Mark(L"CreateShadowResources");
        ReportStartupStep(L"Shadow resources ready", L"Building material textures.");
        ReportStartupActivity(L"Loading textures", L"Checking texture cache.");
        if (!CreateTextures()) {
            startupRuntime_.lastInitializeError = L"CreateTextures failed.";
            return false;
        }
        profile.Mark(L"CreateTextures");
        ReportStartupStep(L"Textures ready", L"Loading flashlight pattern.");
        if (!CreateFlashlightPatternTexture()) {
            startupRuntime_.lastInitializeError = L"CreateFlashlightPatternTexture failed.";
            return false;
        }
        profile.Mark(L"CreateFlashlightPatternTexture");
        ReportStartupStep(L"Flashlight pattern ready", L"Creating constant buffers.");
        if (!CreateConstantBuffer()) {
            startupRuntime_.lastInitializeError = L"CreateConstantBuffer failed.";
            return false;
        }
        profile.Mark(L"CreateConstantBuffer");
        ReportStartupStep(L"GPU buffers ready", sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? L"Loading menu meshes."
            : L"Loading monster mesh.");

        sessionRuntime_.SeedRuntime(ResolveRuntimeSeed(settingsRuntime_.live.mazeSeed));
        ApplyRuntimeVariation(settingsRuntime_.live, sessionRuntime_.runtimeSeed);
        sessionRuntime_.gameplaySettings = settingsRuntime_.live;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) ApplyMainMenuSettings();
        if (gEffectDebugViewer) ApplyDebugSliceSettings();
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            LoadMenuPropMeshes();
            profile.Mark(L"LoadMenuPropMeshes");
            ReportStartupStep(L"Menu meshes ready", L"Generating menu layout.");
        } else {
            EnsureFullSceneAssets();
            profile.Mark(L"LoadSceneAssets");
            ReportStartupStep(L"Scene meshes ready", L"Generating maze layout.");
        }
        profile.Mark(L"LoadPropMeshes");
        GameWorldMazeGenerationRequest mazeRequest{};
        mazeRequest.layout = MakeMazeLayoutSpec(settingsRuntime_.live);
        mazeRequest.generation = MakeMazeGenerationSpec(settingsRuntime_.live);
        mazeRequest.runtimeSeed = sessionRuntime_.runtimeSeed;
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            mazeRequest.kind = GameWorldMazeGenerationKind::MainMenu;
        } else if (gEffectDebugViewer) {
            mazeRequest.kind = GameWorldMazeGenerationKind::DebugSlice;
            mazeRequest.debugSliceTiles = gDebugSliceTiles;
        } else if (gBloodDebugEveryWall) {
            mazeRequest.kind = GameWorldMazeGenerationKind::BloodDebugCorridor;
        } else if (BenchmarkDemoEnabled()) {
            mazeRequest.kind = GameWorldMazeGenerationKind::BenchmarkDemo;
        } else {
            mazeRequest.kind = GameWorldMazeGenerationKind::Standard;
        }
        gameWorld_.GenerateMaze(mazeRequest);
        profile.Mark(L"GenerateMaze");
        ReportStartupStep(L"Maze generated", L"Uploading maze mask.");
        if (!CreateMazeMaskTexture()) {
            startupRuntime_.lastInitializeError = L"CreateMazeMaskTexture failed.";
            return false;
        }
        profile.Mark(L"CreateMazeMaskTexture");
        ReportStartupStep(L"Maze mask ready", L"Building maze geometry.");
        ResetSimulation();
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            menuRuntime_.darkLayerOneRun = false;
        }
        CreateMazeMesh();
        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) SetupMainMenuScene();
        SetupPersistentAudioEmitters();
        ResetDebugSliceLoopState();
        profile.Mark(L"CreateMazeMesh");
        ReportStartupStep(L"Ready", L"Entering maze.");
        timeRuntime_.lastTicks = GetTickCount64();
        debugRuntime_.bloodDebugStartTicks = timeRuntime_.lastTicks;
        return true;
    }
