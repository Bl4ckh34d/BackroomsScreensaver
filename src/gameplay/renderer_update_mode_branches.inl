        viewRuntime_.fadeInTimer = std::max(0.0f, viewRuntime_.fadeInTimer - dt);
        if (sessionRuntime_.IsMainMenu()) {
            UpdateMainMenuScene(dt);
            return;
        }
        if (monsterPreview_.active) {
            UpdateMonsterHeadAnimation(dt, false);
            SetMonsterPreviewCamera(timeRuntime_.time);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (gEffectDebugViewer) {
            ApplyDebugSliceCamera();
            UpdateDebugSliceLoop(dt);
            UpdateDebugMonsterWalk(dt);
            if (gDebugSliceEffect == DebugSliceEffect::AirVents) {
                UpdateSteamAndDrops(dt);
            } else if (gDebugSliceEffect == DebugSliceEffect::BrokenLamps) {
                UpdateSparks(dt);
            }
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            return;
        }
        if (settingsRuntime_.live.bloodStudyView) {
            ApplyBloodStudyCamera();
            scareRuntime_.bloodWorldActivationTime = timeRuntime_.time - 46.0f;
            UpdateSparks(dt);
            UpdateSteamAndDrops(dt);
            UpdateFlashlightAim(dt);
            UpdateDreadMeterDisplay(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            return;
        }
        if (benchmarkRuntime_.active || BenchmarkDemoEnabled()) {
            benchmarkRuntime_.active = true;
            benchmarkRuntime_.timer += dt;
            UpdateScareEvents(dt);
            UpdateSparks(dt);
            UpdateSteamAndDrops(dt);
            UpdateBrokenRuntimeLampSparks(dt, 3.0f, 9.5f, 0.16f);
            if (MonsterActiveForCurrentMode()) {
                UpdateMonsterHeadAnimation(dt, false);
            }
            viewRuntime_.dangerLevel = 0.40f + std::sin(timeRuntime_.time * 0.73f) * 0.12f;
            viewRuntime_.dreadLevel = 0.45f + std::cos(timeRuntime_.time * 0.41f) * 0.10f;
            UpdateDreadMeterDisplay(dt);
            ApplyBenchmarkDemoCamera(benchmarkRuntime_.timer);
            UpdateFlashlightAim(dt);
            UpdateAirParticles(dt);
            UpdateAirParticleFocus(dt);
            float duration = BenchmarkDemoDurationSeconds();
            if (duration > 0.0f && benchmarkRuntime_.timer >= duration && hostRuntime_.hwnd) {
                PostMessageW(hostRuntime_.hwnd, WM_CLOSE, 0, 0);
            }
            return;
        }
