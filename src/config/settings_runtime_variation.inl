// Runtime seed resolution and bounded per-run settings variation.

uint32_t ResolveRuntimeSeed(uint32_t configuredSeed) {
    if (configuredSeed != 0) return configuredSeed;

    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    uint64_t mix = static_cast<uint64_t>(counter.QuadPart);
    mix ^= GetTickCount64() + 0x9e3779b97f4a7c15ull;
    mix ^= static_cast<uint64_t>(GetCurrentProcessId()) << 32;
    mix ^= static_cast<uint64_t>(GetCurrentThreadId()) << 16;
    mix ^= static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    mix ^= static_cast<uint64_t>(reinterpret_cast<uintptr_t>(&mix));

    uint64_t osRandom[2]{};
    if (SystemFunction036(osRandom, sizeof(osRandom))) {
        mix ^= osRandom[0];
        mix ^= osRandom[1] + 0x9e3779b97f4a7c15ull + (mix << 6) + (mix >> 2);
    }

    std::random_device rd;
    mix ^= static_cast<uint64_t>(rd()) << 1;
    mix ^= static_cast<uint64_t>(rd()) << 33;

    mix ^= mix >> 33;
    mix *= 0xff51afd7ed558ccdull;
    mix ^= mix >> 33;
    mix *= 0xc4ceb9fe1a85ec53ull;
    mix ^= mix >> 33;
    uint32_t seed = static_cast<uint32_t>(mix ^ (mix >> 32));
    return seed != 0 ? seed : 1u;
}

float RuntimeSigned(uint32_t seed, int salt) {
    return Rand01(salt, salt * 37 + 19, seed ^ 0x6d2b79f5u) * 2.0f - 1.0f;
}

float JitterScaled(uint32_t seed, int salt, float value, float amount, float maxFraction, float minValue, float maxValue) {
    if (amount <= 0.0001f || std::abs(value) <= 0.0001f) return std::clamp(value, minValue, maxValue);
    float scale = 1.0f + RuntimeSigned(seed, salt) * amount * maxFraction;
    return std::clamp(value * scale, minValue, maxValue);
}

int JitterInt(uint32_t seed, int salt, int value, float amount, float maxFraction, int minValue, int maxValue) {
    if (value == 0) return std::clamp(value, minValue, maxValue);
    float jittered = JitterScaled(seed, salt, static_cast<float>(value), amount, maxFraction,
        static_cast<float>(minValue), static_cast<float>(maxValue));
    return std::clamp(static_cast<int>(std::round(jittered)), minValue, maxValue);
}

int JitterIntRange(uint32_t seed, int salt, int value, int range, float amount, int minValue, int maxValue) {
    if (amount <= 0.0001f || range <= 0) return std::clamp(value, minValue, maxValue);
    float delta = RuntimeSigned(seed, salt) * amount * static_cast<float>(range);
    return std::clamp(value + static_cast<int>(std::round(delta)), minValue, maxValue);
}

void ApplyRuntimeVariation(Settings& s, uint32_t seed) {
    float v = Clamp01(s.runVariation);
    if (v <= 0.0001f) return;
    constexpr float kUserFraction = 0.15f;

    s.roomCount = JitterIntRange(seed, 101, s.roomCount, s.roomCountRange, v, 0, 80);
    s.roomMinRadius = JitterIntRange(seed, 102, s.roomMinRadius, s.roomMinRadiusRange, v, 1, 12);
    s.roomMaxRadius = JitterIntRange(seed, 103, s.roomMaxRadius, s.roomMaxRadiusRange, v, s.roomMinRadius, 16);

    s.flashlightIntensity = JitterScaled(seed, 201, s.flashlightIntensity, v, 0.18f, 0.0f, 10.0f);
    s.flashlightAttenuation = JitterScaled(seed, 202, s.flashlightAttenuation, v, 0.22f, 0.001f, 2.0f);
    s.flashlightConeDegrees = JitterScaled(seed, 203, s.flashlightConeDegrees, v, 0.10f, 20.0f, 140.0f);
    s.ambientLight = JitterScaled(seed, 204, s.ambientLight, v, 0.45f, 0.0f, 1.0f);
    s.lampIntensity = JitterScaled(seed, 205, s.lampIntensity, v, 0.38f, 0.0f, 10.0f);
    s.lampOnRatio = JitterScaled(seed, 207, s.lampOnRatio, v, 0.65f, 0.0f, 1.0f);
    s.lampFlickerRatio = JitterScaled(seed, 208, s.lampFlickerRatio, v, 0.55f, 0.0f, 1.0f);
    s.brokenZoneRatio = JitterScaled(seed, 209, s.brokenZoneRatio, v, 0.55f, 0.0f, 1.0f);
    s.fogStartMeters = JitterScaled(seed, 211, s.fogStartMeters, v, 0.22f, 0.0f, 200.0f);
    s.fogEndMeters = std::max(s.fogStartMeters + 0.1f, JitterScaled(seed, 212, s.fogEndMeters, v, 0.22f, 0.1f, 300.0f));
    s.fogDarkness = JitterScaled(seed, 213, s.fogDarkness, v, 0.28f, 0.0f, 1.0f);
    s.cornerAOIntensity = JitterScaled(seed, 214, s.cornerAOIntensity, v, 0.24f, 0.0f, 1.0f);
    s.exposure = JitterScaled(seed, 215, s.exposure, v, 0.12f, 0.1f, 8.0f);
    s.motionBlurAmount = JitterScaled(seed, 216, s.motionBlurAmount, v, 0.10f, 0.0f, 2.0f);
    s.bloomAmount = JitterScaled(seed, 217, s.bloomAmount, v, 0.10f, 0.0f, 2.0f);
    s.lensDirtAmount = JitterScaled(seed, 218, s.lensDirtAmount, v, 0.10f, 0.0f, 2.0f);

    s.walkSpeed = JitterScaled(seed, 301, s.walkSpeed, v, kUserFraction, 0.1f, 8.0f);
    s.roomSpeed = JitterScaled(seed, 302, s.roomSpeed, v, kUserFraction, 0.1f, 8.0f);
    s.runSpeed = JitterScaled(seed, 303, s.runSpeed, v, kUserFraction, 0.1f, 12.0f);
    s.turnLookAheadTiles = JitterScaled(seed, 304, s.turnLookAheadTiles, v, kUserFraction, 0.0f, 8.0f);
    s.roomLookAheadTiles = JitterScaled(seed, 305, s.roomLookAheadTiles, v, kUserFraction, 0.0f, 10.0f);
    s.roomPauseChance = JitterScaled(seed, 306, s.roomPauseChance, v, kUserFraction, 0.0f, 1.0f);
    s.junctionScanChance = JitterScaled(seed, 307, s.junctionScanChance, v, kUserFraction, 0.0f, 1.0f);
    s.scanAngleDegrees = JitterScaled(seed, 308, s.scanAngleDegrees, v, kUserFraction, 0.0f, 160.0f);
    float lookBackMin = JitterScaled(seed, 309, s.lookBackMinSeconds, v, kUserFraction, 2.0f, 300.0f);
    float lookBackMax = JitterScaled(seed, 310, s.lookBackMaxSeconds, v, kUserFraction, 2.0f, 300.0f);
    s.lookBackMinSeconds = std::min(lookBackMin, lookBackMax);
    s.lookBackMaxSeconds = std::max(lookBackMin, lookBackMax);
    s.headBobAmount = JitterScaled(seed, 311, s.headBobAmount, v, kUserFraction, 0.0f, 0.4f);
    s.sideSwayAmount = JitterScaled(seed, 312, s.sideSwayAmount, v, kUserFraction, 0.0f, 0.3f);
    s.junctionScanBaseSeconds = JitterScaled(seed, 313, s.junctionScanBaseSeconds, v, kUserFraction, 0.0f, 4.0f);
    s.junctionScanBranchSeconds = JitterScaled(seed, 314, s.junctionScanBranchSeconds, v, kUserFraction, 0.0f, 3.0f);
    s.flashlightSwayAmount = JitterScaled(seed, 315, s.flashlightSwayAmount, v, kUserFraction, 0.0f, 4.0f);
    s.flashlightFollowSpeed = JitterScaled(seed, 316, s.flashlightFollowSpeed, v, kUserFraction, 0.1f, 4.0f);
    s.flashlightPanicDartAmount = JitterScaled(seed, 317, s.flashlightPanicDartAmount, v, kUserFraction, 0.0f, 4.0f);

    s.paperDensity = JitterScaled(seed, 401, s.paperDensity, v, kUserFraction, 0.0f, 4.0f);
    s.hallwayPaperRunDensity = JitterScaled(seed, 402, s.hallwayPaperRunDensity, v, kUserFraction, 0.0f, 4.0f);
    s.chairDensity = JitterScaled(seed, 403, s.chairDensity, v, kUserFraction, 0.0f, 4.0f);
    s.waterDamageDensity = JitterScaled(seed, 406, s.waterDamageDensity, v, kUserFraction, 0.0f, 4.0f);
    s.waterDamageEnabled = false;
    s.waterDamageDensity = 0.0f;
    s.bloodSplatterDensity = 0.0f;
    s.metalCabinetDensity = JitterScaled(seed, 407, s.metalCabinetDensity, v, kUserFraction, 0.0f, 4.0f);
    s.jumpscareFrequency = JitterScaled(seed, 408, s.jumpscareFrequency, v, kUserFraction, 0.0f, 1.0f);
    s.sparkEmitterRatio = JitterScaled(seed, 409, s.sparkEmitterRatio, v, kUserFraction, 0.0f, 1.0f);
    float sparkBurstMin = JitterScaled(seed, 410, s.sparkBurstMinSeconds, v, kUserFraction, 0.05f, 60.0f);
    float sparkBurstMax = JitterScaled(seed, 411, s.sparkBurstMaxSeconds, v, kUserFraction, 0.05f, 60.0f);
    s.sparkBurstMinSeconds = std::min(sparkBurstMin, sparkBurstMax);
    s.sparkBurstMaxSeconds = std::max(sparkBurstMin, sparkBurstMax);
    s.sparkMaxParticles = JitterInt(seed, 412, s.sparkMaxParticles, v, kUserFraction, 0, 1200);
    s.sparkSize = JitterScaled(seed, 413, s.sparkSize, v, kUserFraction, 0.1f, 5.0f);
    s.airParticleDensity = JitterScaled(seed, 414, s.airParticleDensity, v, kUserFraction, 0.0f, 4.0f);
    s.airParticleSize = JitterScaled(seed, 415, s.airParticleSize, v, kUserFraction, 0.20f, 4.0f);
    s.airParticleBlur = JitterScaled(seed, 416, s.airParticleBlur, v, kUserFraction, 0.0f, 3.0f);

    s.dreadDecayPerSecond = JitterScaled(seed, 501, s.dreadDecayPerSecond, v, 0.30f, 0.0f, 1.0f);
    s.dreadMonsterDistance = JitterScaled(seed, 502, s.dreadMonsterDistance, v, 0.22f, 1.0f, 60.0f);
    s.dreadMonsterGainPerSecond = JitterScaled(seed, 503, s.dreadMonsterGainPerSecond, v, 0.28f, 0.0f, 3.0f);
    s.dreadJumpscareGain = JitterScaled(seed, 504, s.dreadJumpscareGain, v, 0.30f, 0.0f, 1.0f);
    s.dreadFleshGain = JitterScaled(seed, 505, s.dreadFleshGain, v, 0.30f, 0.0f, 1.0f);
    s.dreadWalkSpeedBoost = JitterScaled(seed, 506, s.dreadWalkSpeedBoost, v, 0.24f, 0.0f, 2.0f);
    s.dreadRunSpeedBoost = JitterScaled(seed, 507, s.dreadRunSpeedBoost, v, 0.24f, 0.0f, 2.0f);
    s.dreadFlashlightFlicker = JitterScaled(seed, 508, s.dreadFlashlightFlicker, v, 0.30f, 0.0f, 3.0f);

    s.monsterScale = JitterScaled(seed, 601, s.monsterScale, v, 0.14f, 0.25f, 4.0f);
    s.monsterSpeed = JitterScaled(seed, 602, s.monsterSpeed, v, 0.22f, 0.1f, 4.0f);
    s.monsterSprintSpeed = JitterScaled(seed, 603, s.monsterSprintSpeed, v, 0.22f, 0.1f, 4.0f);
    s.monsterKillDistance = JitterScaled(seed, 604, s.monsterKillDistance, v, 0.10f, 0.2f, 4.0f);
    s.monsterVisibleDistance = JitterScaled(seed, 605, s.monsterVisibleDistance, v, 0.25f, 1.0f, 60.0f);
}
