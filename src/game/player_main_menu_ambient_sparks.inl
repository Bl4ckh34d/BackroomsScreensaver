    void UpdateMainMenuAmbientSparks(float dt) {
        if (menuRuntime_.darkLayerOneRun || !settingsRuntime_.live.sparkParticles) return;
        for (SparkEmitter& emitter : effectRuntime_.sparkEmitters) {
            emitter.cooldown -= std::max(0.0f, dt);
            if (emitter.cooldown > 0.0f) continue;
            float intensity = RandRange(0.35f, 0.92f);
            EmitSparkBurstAt(emitter.pos, intensity);
            QueueSparkSoundAt(emitter.pos, intensity);
            emitter.cooldown = RandRange(2.8f, 7.4f);
        }
    }
