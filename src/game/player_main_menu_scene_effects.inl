        viewRuntime_.flashlightYaw += AngleWrap(targetYaw - viewRuntime_.flashlightYaw) * std::min(1.0f, dt * 15.0f);
        viewRuntime_.flashlightPitch += (std::clamp(targetPitch, -0.86f, 0.70f) - viewRuntime_.flashlightPitch) * std::min(1.0f, dt * 15.0f);

        float doorTarget = menuRuntime_.exitHover ? 1.0f : 0.0f;
        float doorResponse = doorTarget > menuRuntime_.doorOpen ? 2.35f : 5.8f;
        menuRuntime_.doorOpen += (doorTarget - menuRuntime_.doorOpen) * std::min(1.0f, dt * doorResponse);
        exitDoorPresentation_.angle = menuRuntime_.doorOpen * 1.38f;

        if (menuRuntime_.darkLayerOneRun && menuRuntime_.singlePlayerHover && !menuRuntime_.lampBurstPlayed) {
            menuRuntime_.lampBurstPending = true;
        }

        if (menuRuntime_.lampBurstPending) {
            menuRuntime_.lampBurstPending = false;
            if (menuRuntime_.lampBurstPlayed) {
                UpdateMainMenuAmbientSparks(dt);
                UpdateBrokenRuntimeLampSparks(dt, 1.65f, 5.8f, 0.22f);
                UpdateSparks(dt);
                UpdateAirParticles(dt);
                UpdateAirParticleFocus(dt);
                UpdateDreadMeterDisplay(dt);
                return;
            }
            menuRuntime_.lampBurstPlayed = true;
            if (!effectRuntime_.runtimeLamps.empty()) {
                RuntimeLampState* nearest = &effectRuntime_.runtimeLamps.front();
                XMFLOAT3 c = gameWorld_.maze.WorldCenter(gameWorld_.maze.start, 0.0f);
                float best = std::numeric_limits<float>::max();
                for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
                    float dx = lamp.pos.x - c.x;
                    float dz = lamp.pos.z - c.z;
                    float d2 = dx * dx + dz * dz;
                    if (d2 < best) {
                        best = d2;
                        nearest = &lamp;
                    }
                }
                if (nearest->broken) {
                    QueueLightBulbBreakSoundAt(nearest->pos, 1.0f);
                } else {
                    BreakRuntimeLamp(*nearest);
                }
            } else {
                XMFLOAT3 c = gameWorld_.maze.WorldCenter(gameWorld_.maze.start, settingsRuntime_.live.wallHeightMeters - 0.10f);
                QueueLightBulbBreakSoundAt(c, 1.0f);
                EmitSparkBurstAt(c, 3.1f);
                ScheduleSparkChain(c, 2.1f, 3);
            }
        }

        UpdateMainMenuAmbientSparks(dt);
        UpdateBrokenRuntimeLampSparks(dt, 1.65f, 5.8f, 0.22f);
        UpdateSparks(dt);
        UpdateAirParticles(dt);
        UpdateAirParticleFocus(dt);
        UpdateDreadMeterDisplay(dt);
