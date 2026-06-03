// Game audio event dispatch, player noise, hearing radii, and short one-shot queues. 
// Included inside Renderer's private section from renderer_audio.inl.

    void RecomputePlayerNoiseRadiusFromPulses() {
        gameWorld_.RecomputePlayerNoiseRadiusFromPulses();
    }

    void UpdatePlayerAudibleSoundPulses(float dt) {
        gameWorld_.AdvancePlayerSoundPulses(dt);
    }

    void EmitPlayerAudibleSound(XMFLOAT3 pos, float radius, float life = 0.90f) {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu || radius <= 0.01f) return;
        pos.y = 0.08f;
        gameWorld_.EmitPlayerSoundPulse(pos, radius, life, 18);
    }

    void EmitPlayerAudibleSoundAtCamera(float radius, float life = 0.90f) {
        EmitPlayerAudibleSound({gameWorld_.player.position.x, 0.08f, gameWorld_.player.position.z}, radius, life);
    }

    void DispatchGameAudioEvent(const GameAudioEvent& event, bool playbackEnabled = true) {
        if (event.kind == GameAudioEventKind::PlayerNoise) {
            EmitPlayerAudibleSound(event.pos, event.hearingRadius, event.hearingLife);
            return;
        }

        if (playbackEnabled && event.volume > 0.0f) {
            float occlusion = event.useOcclusion ? AudioOcclusionFor(event.pos) : 0.0f;
            occlusion = std::min(occlusion, event.occlusionLimit);
            audioRuntime_.engine.PlayRandom(event.sound, event.bus, event.pos, event.volume, event.spatial, occlusion);
        }
        if (event.hearingRadius > 0.0f) {
            EmitPlayerAudibleSound(event.pos, event.hearingRadius, event.hearingLife);
        }
    }

    void DrainGameAudioEvents(bool playbackEnabled = true) {
        std::vector<GameAudioEvent> events = gameWorld_.DrainAudioEvents();
        for (const GameAudioEvent& event : events) {
            DispatchGameAudioEvent(event, playbackEnabled);
        }
    }

    float TileHearingRadius(float tiles) const {
        return std::max(0.1f, gameWorld_.maze.TileAverage()) * std::max(0.0f, tiles);
    }

    float FootstepHearingRadius(float walkT, float runT, bool crouching, bool wetFootstep) const {
        float radius = 0.0f;
        if (crouching) {
            radius = TileHearingRadius(0.35f);
        } else {
            float runBlend = std::max(runT, gameWorld_.player.runEffort * 0.72f);
            if (runBlend > 0.01f) {
                radius = TileHearingRadius(Lerp(3.2f, 4.0f, runBlend));
            } else {
                radius = TileHearingRadius(Lerp(2.1f, 2.9f, walkT));
            }
        }
        return radius * (wetFootstep ? 1.25f : 1.0f);
    }

    float JumpscareHearingRadius(float scale = 1.0f) const {
        return std::max(0.1f, gameWorld_.maze.TileAverage()) * 4.85f * std::max(0.1f, scale);
    }

    float LightBulbBreakHearingRadius() const {
        return TileHearingRadius(28.0f);
    }

    float FlashlightClickHearingRadius() const {
        return std::max(0.55f, gameWorld_.maze.TileMinimum() * 0.62f);
    }

    float AirVentHearingRadius() const {
        return TileHearingRadius(5.0f);
    }

    float SparkHearingRadius(float intensity = 1.0f) const {
        return TileHearingRadius(Lerp(9.0f, 16.0f, Clamp01(intensity / std::max(0.1f, settingsRuntime_.live.effectBrokenLampSparkIntensityMax))));
    }
