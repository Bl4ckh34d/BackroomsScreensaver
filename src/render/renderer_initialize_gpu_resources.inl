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
