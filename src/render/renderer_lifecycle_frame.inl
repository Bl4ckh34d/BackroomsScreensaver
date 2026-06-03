    // Renderer resize, frame tick, and present controls.

    void Resize(int w, int h) {
        if (!d3dRuntime_.device || w <= 0 || h <= 0) return;
        hostRuntime_.width = w;
        hostRuntime_.height = h;
        d3dRuntime_.context->OMSetRenderTargets(0, nullptr, nullptr);
        renderTargetRuntime_.rtv.Reset();
        renderTargetRuntime_.dsv.Reset();
        renderTargetRuntime_.depth.Reset();
        renderTargetRuntime_.sceneColorSrv.Reset();
        renderTargetRuntime_.sceneColorRtv.Reset();
        renderTargetRuntime_.sceneColor.Reset();
        HRESULT hr = d3dRuntime_.swapChain->ResizeBuffers(0, static_cast<UINT>(w), static_cast<UINT>(h), DXGI_FORMAT_UNKNOWN, 0);
        if (SUCCEEDED(hr)) CreateBackBuffer();
    }

    void Tick() {
        ULONGLONG now = GetTickCount64();
        float dt = std::min(0.05f, static_cast<float>(now - timeRuntime_.lastTicks) / 1000.0f);
        timeRuntime_.lastTicks = now;
        TickFrame(dt);
    }

    void TickFixed(float dt) {
        timeRuntime_.lastTicks = GetTickCount64();
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
        timeRuntime_.time += dt;
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
                << gpuProfileRuntime_.frameCounter << L","
                << static_cast<int>(sessionRuntime_.mode) << L","
                << (profileEnd - profileStart) << L","
                << (profileAfterProgress - profileStart) << L","
                << (profileAfterBudget - profileAfterProgress) << L","
                << (profileAfterSimulation - profileAfterBudget) << L","
                << (profileAfterAudio - profileAfterSimulation) << L","
                << (profileEnd - profileAfterAudio) << L","
                << staticSceneGeometry_.indexCount << L","
                << staticSceneGeometry_.instancedIndexCount << L","
                << staticSceneGeometry_.instancedInstanceCount << L","
                << staticSceneGeometry_.floorCeilingIndexCount << L","
                << staticSceneGeometry_.waterIndexCount << L","
                << staticSceneGeometry_.transparentIndexCount << L","
                << dynamicGeometry_.opaqueVertexCount << L","
                << dynamicGeometry_.transparentVertexCount << L","
                << effectRuntime_.airParticles.size() << L","
                << effectRuntime_.sparks.size() << L","
                << effectRuntime_.steam.size() << L","
                << effectRuntime_.runtimeLamps.size();
            RuntimeProfileFrameLine(csv.str());
        }
    }

    void SetPresentSyncInterval(UINT syncInterval) {
        presentRuntime_.syncInterval = syncInterval;
    }

    void SetPresentFlags(UINT flags) {
        presentRuntime_.flags = flags;
    }

    void SetPresentEnabled(bool enabled) {
        presentRuntime_.enabled = enabled;
    }

    bool LastPresentCompleted() const {
        return presentRuntime_.lastCompleted;
    }
