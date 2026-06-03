    XMFLOAT3 MonsterSoundOrigin() const {
        return MonsterFocusPoint();
    }

    XMFLOAT3 PickVentGroanEmitter() {
        if (effectRuntime_.steamEmitters.empty()) {
            XMFLOAT3 away = Normalize3({gameWorld_.monster.position.x - gameWorld_.player.position.x, 0.0f, gameWorld_.monster.position.z - gameWorld_.player.position.z}, {std::sin(gameWorld_.player.yaw), 0.0f, std::cos(gameWorld_.player.yaw)});
            return {gameWorld_.player.position.x + away.x * gameWorld_.maze.TileAverage() * 2.4f, settingsRuntime_.live.wallHeightMeters * 0.82f, gameWorld_.player.position.z + away.z * gameWorld_.maze.TileAverage() * 2.4f};
        }

        float tile = std::max(0.1f, gameWorld_.maze.TileAverage());
        float bestScore = -1000000.0f;
        const SteamEmitter* best = nullptr;
        for (const SteamEmitter& emitter : effectRuntime_.steamEmitters) {
            float dx = emitter.pos.x - gameWorld_.player.position.x;
            float dz = emitter.pos.z - gameWorld_.player.position.z;
            float distTiles = std::sqrt(dx * dx + dz * dz) / tile;
            float monsterDx = emitter.pos.x - gameWorld_.monster.position.x;
            float monsterDz = emitter.pos.z - gameWorld_.monster.position.z;
            float monsterDistTiles = std::sqrt(monsterDx * monsterDx + monsterDz * monsterDz) / tile;
            float farEnough = SmoothStep(1.15f, 2.45f, distTiles);
            float notTooFar = 1.0f - SmoothStep(7.0f, 10.0f, distTiles);
            float monsterBias = 1.0f - SmoothStep(0.0f, 7.0f, monsterDistTiles);
            float blockedBias = AudioRayClear({gameWorld_.player.position.x, 1.15f, gameWorld_.player.position.z}, emitter.pos) ? 0.0f : 0.35f;
            float score = farEnough * notTooFar * (0.70f + monsterBias * 0.55f + blockedBias) + RandRange(0.0f, 0.32f);
            if (score > bestScore) {
                bestScore = score;
                best = &emitter;
            }
        }
        return best ? best->pos : effectRuntime_.steamEmitters[static_cast<size_t>(sessionRuntime_.rng() % effectRuntime_.steamEmitters.size())].pos;
    }

    void PlayVentMonsterGroan() {
        size_t sampleIndex = audioRuntime_.engine.PickRandomSample(GameSound::MonsterGrowl);
        if (sampleIndex == static_cast<size_t>(-1)) return;

        XMFLOAT3 pos = PickVentGroanEmitter();
        float tile = std::max(0.1f, gameWorld_.maze.TileAverage());
        float monsterTiles = MonsterDistance() / tile;
        float monsterDistanceGain = Lerp(0.16f, 1.0f, 1.0f - SmoothStep(2.5f, 6.6f, monsterTiles));
        float baseVolume = RandRange(0.010f, 0.020f) * monsterDistanceGain;
        float pitch = RandRange(1.08f, 1.22f);
        float ductOcclusion = std::min(8.0f, AudioOcclusionFor(pos) + 1.65f + SmoothStep(3.0f, 6.6f, monsterTiles) * 1.25f);
        audioRuntime_.engine.PlaySample(sampleIndex, AudioBus::Monster, pos, baseVolume, true, pitch, GameSound::MonsterGrowl,
            ductOcclusion, AudioToneProfile::MetallicVent);

        XMFLOAT3 ductDir = Normalize3({gameWorld_.monster.position.x - gameWorld_.player.position.x, 0.0f, gameWorld_.monster.position.z - gameWorld_.player.position.z}, {std::sin(gameWorld_.player.yaw), 0.0f, std::cos(gameWorld_.player.yaw)});
        XMFLOAT3 echoA{pos.x + ductDir.z * tile * RandRange(-0.52f, 0.52f), pos.y, pos.z - ductDir.x * tile * RandRange(-0.52f, 0.52f)};
        XMFLOAT3 echoB{pos.x + ductDir.x * tile * RandRange(0.45f, 1.35f), pos.y, pos.z + ductDir.z * tile * RandRange(0.45f, 1.35f)};
        ScheduleDelayedAudio(sampleIndex, GameSound::MonsterGrowl, AudioBus::Monster, echoA, baseVolume * 0.28f,
            RandRange(0.085f, 0.15f), pitch * RandRange(1.035f, 1.075f), true, AudioToneProfile::MetallicVent,
            GameAudioEventCategory::MonsterVocal);
        ScheduleDelayedAudio(sampleIndex, GameSound::MonsterGrowl, AudioBus::Monster, echoB, baseVolume * 0.14f,
            RandRange(0.19f, 0.34f), pitch * RandRange(0.93f, 0.98f), true, AudioToneProfile::MetallicVent,
            GameAudioEventCategory::MonsterVocal);
    }
