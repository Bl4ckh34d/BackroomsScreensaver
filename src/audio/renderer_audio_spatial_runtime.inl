// Renderer audio occlusion, emitter setup, and per-frame update coordinator. 
// Included inside Renderer's private section from renderer_audio.inl.

    bool AudioRayClear(XMFLOAT3 from, XMFLOAT3 to) const {
        return AudioWallBlocksBetween(from, to) == 0;
    }

    int AudioWallBlocksBetween(XMFLOAT3 from, XMFLOAT3 to) const {
        Tile fromTile = gameWorld_.maze.TileFromWorld(from.x, from.z);
        Tile toTile = gameWorld_.maze.TileFromWorld(to.x, to.z);
        if (!gameWorld_.maze.InBounds(fromTile.x, fromTile.y) || !gameWorld_.maze.InBounds(toTile.x, toTile.y)) return 4;
        if (fromTile == toTile) return gameWorld_.maze.IsOpen(fromTile.x, fromTile.y) ? 0 : 1;
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        int steps = std::clamp(static_cast<int>(dist / std::max(0.05f, gameWorld_.maze.TileMinimum() * 0.12f)), 6, 160);
        int blocks = 0;
        Tile previous{-100000, -100000};
        for (int i = 1; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            Tile sample = gameWorld_.maze.TileFromWorld(from.x + dx * t, from.z + dz * t);
            if (!gameWorld_.maze.InBounds(sample.x, sample.y)) {
                ++blocks;
                if (blocks >= 8) return blocks;
                continue;
            }
            if (!gameWorld_.maze.IsOpen(sample.x, sample.y) && !(sample == previous)) {
                ++blocks;
                if (blocks >= 8) return blocks;
            }
            previous = sample;
        }
        return blocks;
    }

    float AudioOcclusionFor(XMFLOAT3 source) const {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu) return 0.0f;
        float dist = DistanceToPoint(source);
        if (dist > std::max(72.0f, gameWorld_.maze.TileAverage() * 32.0f)) return 0.0f;
        XMFLOAT3 listener{gameWorld_.player.position.x, 1.30f, gameWorld_.player.position.z};
        source.y = std::clamp(source.y, 0.15f, settingsRuntime_.live.wallHeightMeters - 0.08f);
        Tile listenerTile = gameWorld_.maze.TileFromWorld(listener.x, listener.z);
        Tile sourceTile = gameWorld_.maze.TileFromWorld(source.x, source.z);
        if (listenerTile == sourceTile) return 0.0f;

        XMFLOAT3 forward = Normalize3(Sub3(source, listener), {0.0f, 0.0f, 1.0f});
        XMFLOAT3 right = Normalize3(Cross3({0.0f, 1.0f, 0.0f}, forward), {1.0f, 0.0f, 0.0f});
        float radius = std::clamp(gameWorld_.maze.TileMinimum() * 0.16f, 0.12f, 0.30f);
        std::array<float, 5> offsets{{0.0f, -1.0f, 1.0f, -0.45f, 0.45f}};
        int minBlocks = 8;
        for (float offset : offsets) {
            XMFLOAT3 l = Add3(listener, Scale3(right, offset * radius));
            XMFLOAT3 s = Add3(source, Scale3(right, -offset * radius * 0.60f));
            minBlocks = std::min(minBlocks, AudioWallBlocksBetween(l, s));
        }
        return static_cast<float>(minBlocks);
    }

    void SetupPersistentAudioEmitters() {
        audioRuntime_.engine.StopAll();
        audioRuntime_.game.ResetForScene(
            RandRange(12.0f, 36.0f),
            RandRange(7.0f, 18.0f),
            RandRange(14.0f, 32.0f),
            gameWorld_.player.stepPhase);
        if (!audioRuntime_.ready) return;
        UpdatePersistentLampHumVoices(0.0f, true);
    }

    void UpdateAudio(float dt) {
        if (!audioRuntime_.ready) {
            DrainGameAudioEvents(false);
            return;
        }
        audioRuntime_.engine.ApplySettings(MakeAudioEngineSettings(settingsRuntime_.live));
        audioRuntime_.engine.SetListener(gameWorld_.player.position, DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch));
        DrainGameAudioEvents(true);
        UpdatePlayerAudibleSoundPulses(dt);
        UpdateMenuDoorAudio();
        UpdatePersistentLampHumVoices(dt);
        UpdateLampFlickerStarterClicks(dt);
        UpdateDelayedAudio(dt);
        UpdateWetDripAudio(dt);
        UpdateVentMonsterGroans(dt);

        UpdateMonsterVocalAudio(dt);

        bool footstepMoving = IsPlayableSimulationMode(sessionRuntime_.mode) && gameWorld_.player.smoothedMoveSpeed > 0.05f;
        if (audioRuntime_.game.ConsumeFootstepTrigger(footstepMoving, gameWorld_.player.stepPhase, kPi)) PlayFootstepSound();

        if (gameWorld_.exitTransitionActive) {
            if (!audioRuntime_.game.exitDoorOpenSoundPlayed && exitDoorPresentation_.angle > 0.02f) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorOpenCreak,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.90f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.exitDoorOpenSoundPlayed = true;
            }
            if (!audioRuntime_.game.exitDoorCloseCreakSoundPlayed && gameWorld_.exitTransitionTimer > settingsRuntime_.live.exitDoorOpenSeconds + settingsRuntime_.live.exitStepSeconds * 0.15f) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorCloseCreak,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.76f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.exitDoorCloseCreakSoundPlayed = true;
            }
            if (!audioRuntime_.game.exitDoorCloseSoundPlayed && gameWorld_.exitTransitionTimer > settingsRuntime_.live.exitDoorOpenSeconds + settingsRuntime_.live.exitStepSeconds * 0.55f) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::DoorCloseLock,
                    AudioBus::Effects,
                    exitDoorPresentation_.center,
                    0.78f,
                    true,
                    true).WithCategory(GameAudioEventCategory::Door));
                audioRuntime_.game.exitDoorCloseSoundPlayed = true;
            }
        }

        audioRuntime_.engine.Update(dt, [&](XMFLOAT3 pos) {
            return AudioOcclusionFor(pos);
        });
    }
