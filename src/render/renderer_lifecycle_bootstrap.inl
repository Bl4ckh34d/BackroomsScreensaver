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
        target.lampFlickerRatio = 0.18f;
        target.brokenZoneRatio = 0.10f;
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
        target.waterDamageDensity = 3.25f;
        target.bloodSplatterDensity = 3.25f;
        target.bloodWetness = std::max(target.bloodWetness, 0.995f);
        target.bloodShaderQuality = 1.0f;
        target.bloodWorldFlicker = true;
        target.bloodWorldAlwaysOn = true;
        target.bloodWorldCoverage = 0.58f;
        target.bloodWorldFlickerMinSeconds = 8.0f;
        target.bloodWorldFlickerMaxSeconds = 16.0f;
        target.bloodWorldFlickerDuration = 1.4f;
        target.bloodWorldFlickerIntensity = 0.55f;
        target.fleshFlicker = true;
        target.fleshAlwaysOn = false;
        target.fleshFlickerMinSeconds = 10.0f;
        target.fleshFlickerMaxSeconds = 18.0f;
        target.fleshFlickerDuration = 1.1f;
        target.fleshFlickerIntensity = 0.45f;
        target.jumpscareFrequency = 1.0f;
        target.brokenLampScaresEnabled = true;
        target.airVentScaresEnabled = true;
        target.flashlightIntensity = std::max(target.flashlightIntensity, 1.0f);
        target.flashlightShadows = true;
        target.flashlightShadowDistanceMeters = std::max(target.flashlightShadowDistanceMeters, 18.0f);
        target.lensDirtAmount = std::max(target.lensDirtAmount, 0.48f);
        target.fogStartMeters = 0.0f;
        target.fogEndMeters = 36.0f;
        target.fogDarkness = 1.0f;
        target.monsterIgnorePlayer = true;
        target.debugInvincible = true;
    }
