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
