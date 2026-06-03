    // Renderer startup, shutdown, settings bootstrap, and audio preparation.

    ~Renderer() {
        audioRuntime_.engine.Shutdown();
    }

    void ApplyBenchmarkDemoSettings(Settings& target) const {
        target.mazeWidth = 75;
        target.mazeHeight = 75;
        target.roomCount = 0;
        target.mazeSeed = 0xB345D123u;
        target.runVariation = 0.0f;
        target.mapOverlay = false;
        target.debugAiMapOverlay = false;
        target.lampOnRatio = 1.0f;
        target.lampFlickerRatio = 0.10f;
        target.brokenZoneRatio = 0.0f;
        target.sparkParticles = true;
        target.sparkEmitterRatio = 0.35f;
        target.sparkBurstMinSeconds = 0.20f;
        target.sparkBurstMaxSeconds = 0.42f;
        target.sparkMaxParticles = 1200;
        target.airParticles = true;
        target.airParticleDensity = 4.0f;
        target.airParticleSize = 1.0f;
        target.airParticleBlur = 1.0f;
        target.paperDensity = 4.0f;
        target.hallwayPaperRunDensity = 4.0f;
        target.chairDensity = 4.0f;
        target.metalCabinetDensity = 4.0f;
        target.waterDamageEnabled = true;
        target.waterDamageDensity = 4.0f;
        target.bloodSplatterDensity = 4.0f;
        target.bloodWorldFlicker = false;
        target.bloodWorldAlwaysOn = false;
        target.bloodWorldCoverage = 0.0f;
        target.bloodWorldFlickerMinSeconds = 1500.0f;
        target.bloodWorldFlickerMaxSeconds = 4800.0f;
        target.bloodWorldFlickerDuration = 0.35f;
        target.bloodWorldFlickerIntensity = 0.0f;
        target.fleshFlicker = false;
        target.fleshAlwaysOn = false;
        target.fleshFlickerMinSeconds = 1500.0f;
        target.fleshFlickerMaxSeconds = 4800.0f;
        target.fleshFlickerDuration = 0.35f;
        target.fleshFlickerIntensity = 0.0f;
        target.jumpscareFrequency = 0.0f;
        target.flashlightIntensity = std::max(target.flashlightIntensity, 1.0f);
        target.flashlightShadows = true;
        target.flashlightShadowDistanceMeters = std::max(target.flashlightShadowDistanceMeters, 18.0f);
        target.fogStartMeters = 0.0f;
        target.fogEndMeters = 36.0f;
        target.fogDarkness = 1.0f;
        target.monsterIgnorePlayer = true;
        target.debugInvincible = true;
    }
