// Spark burst, lamp damage, and broken-lamp scare helpers. 
// Included inside Renderer's private section from scare_effect_events.inl.

    void EmitSparkBurstAt(const XMFLOAT3& pos, float intensity = 1.0f) {
        if (!settingsRuntime_.live.sparkParticles || settingsRuntime_.live.sparkMaxParticles <= 0) return;
        QueueSparkSoundAt(pos, intensity);
        int count = static_cast<int>((5.0f + RandRange(0.0f, 8.9f)) * intensity);
        for (int i = 0; i < count && effectRuntime_.sparks.size() < static_cast<size_t>(settingsRuntime_.live.sparkMaxParticles); ++i) {
            float yaw = RandRange(-1.15f, 1.15f) + LampHash(pos.x, pos.z) * kPi;
            float speed = RandRange(0.35f, 1.25f) * (0.85f + intensity * 0.30f);
            SparkParticle sp{};
            sp.pos = pos;
            sp.vel = {std::sin(yaw) * speed, RandRange(0.15f, 0.92f) * (0.7f + intensity * 0.45f), std::cos(yaw) * speed};
            sp.age = 0.0f;
            sp.life = RandRange(0.55f, 1.35f) * (0.85f + intensity * 0.12f);
            sp.size = RandRange(0.018f, 0.040f) * settingsRuntime_.live.sparkSize * (0.9f + intensity * 0.12f);
            effectRuntime_.sparks.push_back(sp);
        }
        SparkFlash flash{};
        flash.pos = pos;
        flash.age = 0.0f;
        flash.life = std::clamp(0.16f + intensity * 0.035f, 0.16f, 0.42f);
        flash.intensity = std::clamp(intensity * 2.2f, 0.3f, 12.0f);
        effectRuntime_.sparkFlashes.push_back(flash);
    }

    void SpawnSparkBurst(const SparkEmitter& emitter, float intensity = 1.0f) {
        EmitSparkBurstAt(emitter.pos, intensity);
    }

    void ScheduleSparkChain(const XMFLOAT3& pos, float intensity, int bursts) {
        SparkChain chain{};
        chain.pos = pos;
        chain.timer = RandRange(0.055f, 0.16f);
        chain.intensity = intensity;
        chain.remaining = std::max(0, bursts);
        effectRuntime_.sparkChains.push_back(chain);
    }
