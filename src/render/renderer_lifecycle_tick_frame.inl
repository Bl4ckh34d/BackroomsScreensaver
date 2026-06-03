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
