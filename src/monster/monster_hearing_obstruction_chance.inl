    bool MonsterPassesSoundObstructionChance(const PlayerAudibleSoundPulse& pulse) const {
        XMFLOAT3 monsterPos{gameWorld_.monster.position.x, 0.08f, gameWorld_.monster.position.z};
        int wallBlocks = MonsterSoundWallBlocksBetween(monsterPos, pulse.pos);
        float chance = std::pow(0.5f, static_cast<float>(std::clamp(wallBlocks, 0, 8)));
        if (chance >= 0.999f) return true;
        float ageT = pulse.life > 0.001f ? Clamp01(pulse.age / pulse.life) : 0.0f;
        float roll = Rand01(
            static_cast<int>(std::floor(pulse.pos.x * 31.0f + pulse.pos.z * 17.0f + pulse.radius * 11.0f)),
            static_cast<int>(std::floor(timeRuntime_.time * 60.0f + ageT * 997.0f)),
            sessionRuntime_.runtimeSeed ^ 0x4F1BBCDCu);
        return roll <= chance;
    }
