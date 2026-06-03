// Air vent burst, drop, and reaction helpers. 
// Included inside Renderer's private section from scare_effect_events.inl.

    void SpawnSteamBurst(const SteamEmitter& emitter, float intensity = 1.0f) {
        QueueAirVentDustPuffAt(emitter.pos, intensity);
        int count = static_cast<int>(RandRange(12.0f, 26.0f) * intensity);
        for (int i = 0; i < count && effectRuntime_.steam.size() < 420; ++i) {
            float side = RandRange(-0.42f, 0.42f);
            float rise = RandRange(-0.08f, 0.18f);
            XMFLOAT3 right{emitter.dir.z, 0.0f, -emitter.dir.x};
            SteamParticle sp{};
            sp.pos = Add3(emitter.pos, Add3(Scale3(right, side), {0.0f, rise, 0.0f}));
            float spread = RandRange(-0.32f, 0.32f);
            sp.vel = {
                emitter.dir.x * RandRange(0.55f, 1.25f) * intensity + right.x * spread,
                RandRange(0.16f, 0.62f) * intensity,
                emitter.dir.z * RandRange(0.55f, 1.25f) * intensity + right.z * spread
            };
            sp.age = 0.0f;
            sp.life = RandRange(1.0f, 2.4f) * (0.85f + intensity * 0.18f);
            sp.size = RandRange(0.22f, 0.52f) * (0.85f + intensity * 0.25f);
            effectRuntime_.steam.push_back(sp);
        }
    }

    bool SpawnVentDrop(const SteamEmitter& emitter) {
        if (effectRuntime_.ventDrops.size() >= kMaxVentDrops) return false;
        VentDrop d{};
        d.pos = emitter.pos;
        d.vel = {emitter.dir.x * RandRange(0.45f, 0.95f), RandRange(0.22f, 0.75f), emitter.dir.z * RandRange(0.45f, 0.95f)};
        d.yaw = std::atan2(emitter.dir.x, emitter.dir.z) + RandRange(-0.4f, 0.4f);
        d.roll = RandRange(-0.35f, 0.35f);
        d.angular = RandRange(-4.8f, 4.8f);
        d.life = RandRange(2.5f, 4.6f);
        effectRuntime_.ventDrops.push_back(d);
        return true;
    }

    void BeginVentReaction(const SteamEmitter& emitter, float sensory) {
        if (IsThreatVisible() || ChasePanicActive()) return;
        viewRuntime_.ventReactionTarget = emitter.pos;
        viewRuntime_.ventReactionAway = Normalize3({gameWorld_.player.position.x - emitter.pos.x, 0.0f, gameWorld_.player.position.z - emitter.pos.z}, emitter.dir);
        viewRuntime_.ventReactionDuration = RandRange(1.35f, 1.85f) * Lerp(0.90f, 1.12f, Clamp01(sensory));
        viewRuntime_.ventReactionLookDelay = RandRange(0.14f, 0.38f) * Lerp(1.08f, 0.82f, Clamp01(sensory));
        viewRuntime_.ventReactionBackDuration = RandRange(0.58f, 0.96f);
        viewRuntime_.ventReactionScanSeed = RandRange(0.0f, kPi * 2.0f);
        viewRuntime_.ventReactionTimer = viewRuntime_.ventReactionDuration;
        cameraRuntime_.stopTimer = 0.0f;
        cameraRuntime_.headScanTimer = 0.0f;
        cameraRuntime_.junctionScanActive = false;
        cameraRuntime_.lookBack = false;
        viewRuntime_.propLookTimer = 0.0f;
        scareRuntime_.bloodFocusTimer = 0.0f;
        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.55f + sensory * 0.30f);
    }
