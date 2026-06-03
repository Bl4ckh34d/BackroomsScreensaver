    void MarkWetCeilingDripEmitter(XMFLOAT3 pos, float seed) {
        pos.y = 0.10f;
        for (const WetDripEmitter& existing : effectRuntime_.wetDripEmitters) {
            float dx = existing.pos.x - pos.x;
            float dz = existing.pos.z - pos.z;
            if (dx * dx + dz * dz < 0.20f) return;
        }
        if (effectRuntime_.wetDripEmitters.size() >= 160) return;

        int sx = static_cast<int>(std::floor(pos.x * 41.0f + seed * 97.0f));
        int sz = static_cast<int>(std::floor(pos.z * 43.0f - seed * 83.0f));
        WetDripEmitter emitter{};
        emitter.pos = pos;
        emitter.interval = Lerp(1.0f / 3.0f, 2.0f, Rand01(sx, sz, sessionRuntime_.runtimeSeed ^ 0xD21F5u));
        emitter.timer = emitter.interval * Rand01(sx + 19, sz - 31, sessionRuntime_.runtimeSeed ^ 0x9E772u);
        emitter.volume = Lerp(0.24f, 0.38f, Rand01(sx - 7, sz + 23, sessionRuntime_.runtimeSeed ^ 0x512D9u));
        emitter.age = 0.0f;
        emitter.audibleDelay = Lerp(7.5f, 10.5f, Rand01(sx + 53, sz - 47, sessionRuntime_.runtimeSeed ^ 0x71A45u));
        effectRuntime_.wetDripEmitters.push_back(emitter);
    }

    void UpdateWetDripAudio(float dt) {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu) return;
        float audibleRange = std::max(18.0f, gameWorld_.maze.TileAverage() * 12.0f);
        for (WetDripEmitter& emitter : effectRuntime_.wetDripEmitters) {
            emitter.age += std::max(0.0f, dt);
            if (emitter.age < emitter.audibleDelay) continue;

            emitter.timer -= dt;
            if (emitter.timer > 0.0f) continue;
            if (DistanceToPoint(emitter.pos) <= audibleRange) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::WetCarpetCeilingDrip,
                    AudioBus::Effects,
                    emitter.pos,
                    emitter.volume * RandRange(0.90f, 1.10f),
                    true,
                    true).WithCategory(GameAudioEventCategory::WetDrip));
            }
            emitter.timer += std::max(1.0f / 3.0f, emitter.interval);
            if (emitter.timer <= 0.0f) emitter.timer = emitter.interval;
        }
    }
