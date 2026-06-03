// Monster hearing and sound-alert helper functions. 
// Included inside Renderer's private section from monster_ai.inl.

    void AlertMonsterToSound(const XMFLOAT3& pos) {
        if (MonsterIgnoresPlayer()) return;
        Tile sound = gameWorld_.maze.TileFromWorld(pos.x, pos.z);
        if (!ValidMonsterTile(sound)) return;
        if (!MonsterGoalFarEnough(sound)) return;
        gameWorld_.monster.soundTile = sound;
        gameWorld_.monster.hasSound = true;
        gameWorld_.monster.hasLastKnown = false;
        gameWorld_.monster.chasingVisible = false;
        gameWorld_.monster.searchTimer = 0.0f;
        gameWorld_.monster.roamTimer = 0.0f;
        SetMonsterGoal(sound, true);
    }

    void AlertMonsterToPlayerTrigger(const XMFLOAT3& fallbackPos) {
        if (MonsterIgnoresPlayer()) return;
        Tile player = CameraTile();
        if (ValidMonsterTile(player)) {
            XMFLOAT3 ping = gameWorld_.maze.WorldCenter(player, 0.0f);
            AlertMonsterToSound(ping);
        } else {
            AlertMonsterToSound(fallbackPos);
        }
    }

    int MonsterSoundWallBlocksBetween(XMFLOAT3 from, XMFLOAT3 to) const {
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

    void ProcessPlayerAudibleSoundsForMonster(std::vector<PlayerAudibleSoundPulse>& soundPulses, bool seesPlayer, bool ignorePlayer) {
        if (ignorePlayer) return;
        for (PlayerAudibleSoundPulse& pulse : soundPulses) {
            if (pulse.processedByMonster || pulse.radius <= 0.01f) continue;
            pulse.processedByMonster = true;
            float dx = gameWorld_.monster.position.x - pulse.pos.x;
            float dz = gameWorld_.monster.position.z - pulse.pos.z;
            if (dx * dx + dz * dz > pulse.radius * pulse.radius) continue;
            if (!MonsterGoalFarEnough(gameWorld_.maze.TileFromWorld(pulse.pos.x, pulse.pos.z))) continue;
            if (!MonsterPassesSoundObstructionChance(pulse)) continue;
            pulse.heardByMonster = true;
            gameWorld_.monster.heardPlayerNow = true;
            if (!seesPlayer) {
                AlertMonsterToSound(pulse.pos);
            }
        }
    }
