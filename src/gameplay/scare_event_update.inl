// Scare event trigger update coordinator. 
// Included inside Renderer's private section from scare_effect_events.inl.

    void TriggerVisionFlashJumpscare(bool bloodWorld) {
        scareRuntime_.visionFlashDuration = 0.16f;
        scareRuntime_.visionFlashTimer = scareRuntime_.visionFlashDuration;
        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, bloodWorld ? 0.82f : 0.74f);
        gameWorld_.QueueAudioEvent(GameAudioEvent::OneShot(
            GameSound::VisionFlash,
            AudioBus::Effects,
            gameWorld_.player.position,
            bloodWorld ? 1.08f : 0.98f,
            false).WithCategory(GameAudioEventCategory::Scare));
    }

    void UpdateScareEvents(float dt) {
        scareRuntime_.scareCooldown = std::max(0.0f, scareRuntime_.scareCooldown - dt);
        scareRuntime_.fleshFlickerTimer = std::max(0.0f, scareRuntime_.fleshFlickerTimer - dt);
        scareRuntime_.bloodWorldFlickerTimer = std::max(0.0f, scareRuntime_.bloodWorldFlickerTimer - dt);
        float scareFrequency = JumpscareFrequency();
        float scareScale = ScareCooldownScale();
        auto customScareAllowed = [&](int index) {
            PlayableCustomScareGate gate = gameWorld_.CustomScareGateFor(index);
            if (!gate.allowed) return false;
            return !gate.requiresRoll || RandRange(0.0f, 1.0f) <= gate.chance;
        };
        if (scareFrequency <= 0.001f) {
            scareRuntime_.fleshFlickerTimer = 0.0f;
            scareRuntime_.fleshFlickerCooldown = 1000000.0f;
            scareRuntime_.bloodWorldFlickerTimer = 0.0f;
            scareRuntime_.bloodWorldFlickerCooldown = 1000000.0f;
            return;
        }
        if (IsThreatVisible() || ChasePanicActive()) {
            scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, 0.80f);
            viewRuntime_.ventReactionTimer = 0.0f;
            return;
        }
        if (sessionRuntime_.mode == RendererRuntimeMode::PlayableGame && sessionRuntime_.input.crouch) {
            scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, 0.35f);
            viewRuntime_.ventReactionTimer = 0.0f;
            return;
        }
        bool fleshWorldEnabled = settingsRuntime_.live.fleshFlicker;
        bool bloodWorldEnabled = settingsRuntime_.live.bloodWorldFlicker && settingsRuntime_.live.bloodWorldCoverage > 0.001f;
        if (!fleshWorldEnabled) {
            scareRuntime_.fleshFlickerTimer = 0.0f;
        }
        if (!bloodWorldEnabled) {
            scareRuntime_.bloodWorldFlickerTimer = 0.0f;
        }
        if (!fleshWorldEnabled && !bloodWorldEnabled) {
            scareRuntime_.fleshFlickerCooldown = 1000000.0f;
            scareRuntime_.bloodWorldFlickerCooldown = 1000000.0f;
        } else {
            scareRuntime_.bloodWorldFlickerCooldown = std::max(0.0f, scareRuntime_.bloodWorldFlickerCooldown - dt);
            scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
            if (scareRuntime_.bloodWorldFlickerCooldown <= 0.0f && scareRuntime_.fleshFlickerTimer <= 0.0f &&
                scareRuntime_.bloodWorldFlickerTimer <= 0.0f && scareRuntime_.scareCooldown <= 0.0f) {
                bool bloodAllowed = bloodWorldEnabled && customScareAllowed(3);
                bool fleshAllowed = fleshWorldEnabled && customScareAllowed(4);
                bool triggerBloodWorld = bloodAllowed && (!fleshAllowed || RandRange(0.0f, 1.0f) < 0.50f);
                if (!triggerBloodWorld) {
                    if (!fleshAllowed) {
                        scareRuntime_.bloodWorldFlickerCooldown = RandRange(1.0f, 3.0f);
                        scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
                    } else {
                        scareRuntime_.fleshFlickerDuration = settingsRuntime_.live.fleshFlickerDuration;
                        scareRuntime_.fleshFlickerTimer = scareRuntime_.fleshFlickerDuration;
                        TriggerVisionFlashJumpscare(false);
                        scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, scareRuntime_.fleshFlickerDuration + RandRange(8.0f, 18.0f) * scareScale);
                        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.62f);
                        AddDread(settingsRuntime_.live.dreadFleshGain);
                    }
                } else {
                    scareRuntime_.bloodWorldFlickerDuration = settingsRuntime_.live.bloodWorldFlickerDuration;
                    scareRuntime_.bloodWorldFlickerTimer = scareRuntime_.bloodWorldFlickerDuration;
                    TriggerVisionFlashJumpscare(true);
                    scareRuntime_.bloodWorldActivationTime = timeRuntime_.time;
                    scareRuntime_.scareCooldown = std::max(scareRuntime_.scareCooldown, scareRuntime_.bloodWorldFlickerDuration + RandRange(9.0f, 20.0f) * scareScale);
                    viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.72f);
                    AddDread(std::max(settingsRuntime_.live.dreadJumpscareGain * 0.90f, 0.30f));
                }
                float worldMinSeconds = std::min(
                    fleshWorldEnabled ? settingsRuntime_.live.fleshFlickerMinSeconds : settingsRuntime_.live.bloodWorldFlickerMinSeconds,
                    bloodWorldEnabled ? settingsRuntime_.live.bloodWorldFlickerMinSeconds : settingsRuntime_.live.fleshFlickerMinSeconds);
                float worldMaxSeconds = std::max(
                    fleshWorldEnabled ? settingsRuntime_.live.fleshFlickerMaxSeconds : settingsRuntime_.live.bloodWorldFlickerMaxSeconds,
                    bloodWorldEnabled ? settingsRuntime_.live.bloodWorldFlickerMaxSeconds : settingsRuntime_.live.fleshFlickerMaxSeconds);
                scareRuntime_.bloodWorldFlickerCooldown = RandRange(worldMinSeconds, std::max(worldMinSeconds, worldMaxSeconds));
                scareRuntime_.fleshFlickerCooldown = scareRuntime_.bloodWorldFlickerCooldown;
            }
        }
        if (gameWorld_.deathActive || gameWorld_.exitTransitionActive) return;

        Tile currentTile = CameraTile();
        bool enteredTile = !(currentTile == scareRuntime_.scareEventTile);
        if (enteredTile) {
            scareRuntime_.scareEventTile = currentTile;
        }

        if (settingsRuntime_.live.brokenLampScaresEnabled) {
            for (SparkEmitter& emitter : effectRuntime_.sparkEmitters) {
                if (emitter.triggered) continue;
                if (!PointWithinHorizontalRange(emitter.pos, gameWorld_.maze.TileAverage() * 3.1f)) continue;
                if (!ScareSourceAhead(emitter.pos,
                    gameWorld_.maze.TileMinimum() * 0.78f,
                    gameWorld_.maze.TileAverage() * 2.65f,
                    4,
                    0.10f)) continue;
                float sensory = std::max(ScareSensoryWeight(emitter.pos, 8.5f, 0.80f, 2.35f), 0.72f);
                if (scareRuntime_.scareCooldown <= 0.0f && sensory > 0.0f && customScareAllowed(0)) {
                    emitter.triggered = true;
                    float intensity = PickBrokenLampSparkIntensity();
                    gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                        emitter.pos,
                        SparkHearingRadius(intensity),
                        1.42f,
                        GameAudioEventCategory::Scare));
                    if (!BreakNearestRuntimeLampAt(emitter.pos, gameWorld_.maze.TileMinimum() * 0.46f)) {
                        SpawnSparkBurst(emitter, intensity);
                        ScheduleSparkChain(emitter.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale, PickBrokenLampChainBursts());
                    }
                    scareRuntime_.scareCooldown = RandRange(9.0f, 18.0f) * scareScale;
                    viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.42f + sensory * 0.43f);
                    AddDread(settingsRuntime_.live.dreadJumpscareGain *
                        Clamp01(intensity / std::max(0.1f, settingsRuntime_.live.effectBrokenLampSparkIntensityMax)) * sensory);
                    if (sensory > 0.55f) {
                        viewRuntime_.stumbleTimer = std::max(viewRuntime_.stumbleTimer, 0.12f + sensory * 0.06f);
                        viewRuntime_.stumbleDuration = std::max(viewRuntime_.stumbleDuration, 0.16f + sensory * 0.06f);
                        viewRuntime_.stumbleYawOffset = RandRange(-0.18f, 0.18f) * sensory;
                    }
                }
            }
        }

        if (settingsRuntime_.live.airVentScaresEnabled) {
            for (SteamEmitter& emitter : effectRuntime_.steamEmitters) {
                if (emitter.triggered) continue;
                if (!PointWithinHorizontalRange(emitter.pos, gameWorld_.maze.TileAverage() * 3.25f)) continue;
                if (!ScareSourceAhead(emitter.pos,
                    gameWorld_.maze.TileMinimum() * 0.82f,
                    gameWorld_.maze.TileAverage() * 2.80f,
                    4,
                    0.08f)) continue;
                float sensory = std::max(ScareSensoryWeight(emitter.pos, 7.8f, 0.76f, 2.10f), 0.70f);
                if (sensory > 0.0f && scareRuntime_.scareCooldown <= 0.0f && customScareAllowed(1)) {
                    emitter.triggered = true;
                    gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                        emitter.pos,
                        AirVentHearingRadius(),
                        1.18f,
                        GameAudioEventCategory::Scare));
                    SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                    if (!emitter.panelDropped &&
                        RandRange(0.0f, 1.0f) < settingsRuntime_.live.effectAirVentPanelDropChance &&
                        SpawnVentDrop(emitter)) {
                        emitter.panelDropped = true;
                        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.34f + sensory * 0.41f);
                        AddDread(settingsRuntime_.live.dreadJumpscareGain * 0.78f * sensory);
                    }
                    scareRuntime_.scareCooldown = RandRange(10.0f, 21.0f) * scareScale;
                    AddDread(settingsRuntime_.live.dreadJumpscareGain * 0.62f * sensory);
                    viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.28f + sensory * 0.36f);
                    BeginVentReaction(emitter, sensory);
                }
            }
        }
    }
