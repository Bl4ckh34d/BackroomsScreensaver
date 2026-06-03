// Monster vocal and vent-groan audio helpers. 
// Included inside Renderer's private section from renderer_audio.inl.

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

    bool MonsterAlertAudioActive() const {
        return gameWorld_.monster.AlertAudioActive();
    }

    void PlayLayeredMonsterSound(GameSound sound, XMFLOAT3 pos, float volume, float pitchMin = 0.86f, float pitchMax = 1.10f) {
        size_t first = audioRuntime_.engine.PickRandomSample(sound);
        if (first == static_cast<size_t>(-1)) return;
        size_t second = audioRuntime_.engine.PickRandomSampleExcept(sound, first);
        float firstPitch = RandRange(pitchMin, pitchMax);
        float initialOcclusion = AudioOcclusionFor(pos);
        audioRuntime_.engine.PlaySample(first, AudioBus::Monster, pos, volume * RandRange(0.84f, 1.08f), true, firstPitch, sound,
            initialOcclusion);
        if (second != static_cast<size_t>(-1)) {
            audioRuntime_.engine.PlaySample(second, AudioBus::Monster, pos, volume * RandRange(0.52f, 0.78f), true,
                std::clamp(firstPitch * RandRange(0.82f, 1.18f), 0.50f, 2.0f), sound, initialOcclusion);
        }
    }

    void ScheduleLayeredMonsterSound(GameSound sound, XMFLOAT3 pos, float volume, float delay, float pitchMin = 0.86f, float pitchMax = 1.10f) {
        size_t first = audioRuntime_.engine.PickRandomSample(sound);
        if (first == static_cast<size_t>(-1)) return;
        size_t second = audioRuntime_.engine.PickRandomSampleExcept(sound, first);
        float firstPitch = RandRange(pitchMin, pitchMax);
        ScheduleDelayedAudio(first, sound, AudioBus::Monster, pos, volume * RandRange(0.84f, 1.08f), delay, firstPitch,
            true, AudioToneProfile::Normal, GameAudioEventCategory::MonsterVocal);
        if (second != static_cast<size_t>(-1)) {
            ScheduleDelayedAudio(second, sound, AudioBus::Monster, pos, volume * RandRange(0.52f, 0.78f),
                delay + RandRange(0.015f, 0.055f), std::clamp(firstPitch * RandRange(0.82f, 1.18f), 0.50f, 2.0f),
                true, AudioToneProfile::Normal, GameAudioEventCategory::MonsterVocal);
        }
    }

    void PlayMonsterAlertGroan(float volume = 0.82f) {
        PlayLayeredMonsterSound(GameSound::MonsterGrowl, MonsterSoundOrigin(),
            std::clamp(volume * RandRange(0.86f, 1.12f), 0.18f, 1.25f), 0.82f, 1.12f);
    }

    float MonsterAlertVocalVolume(bool visibleChase, float closePressure) const {
        if (visibleChase) return Lerp(0.92f, 1.18f, closePressure);
        if (gameWorld_.monster.hasLastKnown || gameWorld_.monster.hasSound || gameWorld_.monster.chaseMemoryTimer > 0.0f) return Lerp(0.52f, 0.78f, closePressure);
        return Lerp(0.28f, 0.44f, closePressure);
    }

    void UpdateMonsterVocalAudio(float dt) {
        audioRuntime_.game.monsterSpottedScreamCooldown = std::max(0.0f, audioRuntime_.game.monsterSpottedScreamCooldown - dt);
        if (monsterPreview_.active || !IsPlayableSimulationMode(sessionRuntime_.mode) || !MonsterActiveForCurrentMode()) {
            audioRuntime_.game.monsterAlertAudioActive = false;
            audioRuntime_.game.monsterAlertVocalTimer = 0.0f;
            return;
        }

        bool alert = MonsterAlertAudioActive();
        if (!alert) {
            audioRuntime_.game.monsterAlertAudioActive = false;
            audioRuntime_.game.monsterAlertVocalTimer = 0.0f;
            audioRuntime_.game.nextMonsterGrowlSeconds -= dt;
            if (audioRuntime_.game.nextMonsterGrowlSeconds <= 0.0f) {
                PlayMonsterAlertGroan(RandRange(0.20f, 0.32f));
                audioRuntime_.game.nextMonsterGrowlSeconds = RandRange(12.0f, 36.0f);
            }
            return;
        }

        float distance = MonsterDistance();
        float closePressure = 1.0f - SmoothStep(2.0f, 10.0f, distance);
        bool visibleChase = gameWorld_.monster.chasingVisible || gameWorld_.monster.recognizedForChase;
        float alertVolume = MonsterAlertVocalVolume(visibleChase, closePressure);

        if (!audioRuntime_.game.monsterAlertAudioActive && visibleChase && audioRuntime_.game.monsterSpottedScreamCooldown <= 0.0f) {
            PlayLayeredMonsterSound(GameSound::MonsterSpottedScream, MonsterSoundOrigin(), alertVolume, 0.88f, 1.08f);
            ScheduleLayeredMonsterSound(GameSound::MonsterGrowl, MonsterSoundOrigin(),
                alertVolume * RandRange(0.74f, 0.92f), RandRange(0.55f, 0.95f), 0.82f, 1.10f);
            audioRuntime_.game.monsterSpottedScreamCooldown = RandRange(5.2f, 7.4f);
            audioRuntime_.game.monsterAlertVocalTimer = RandRange(1.0f, 1.8f);
        }

        audioRuntime_.game.monsterAlertAudioActive = true;
        audioRuntime_.game.monsterAlertVocalTimer = std::max(0.0f, audioRuntime_.game.monsterAlertVocalTimer - dt);
        if (audioRuntime_.game.monsterAlertVocalTimer > 0.0f) return;

        if (visibleChase && audioRuntime_.game.monsterSpottedScreamCooldown <= 0.0f && RandRange(0.0f, 1.0f) < 0.34f) {
            PlayLayeredMonsterSound(GameSound::MonsterSpottedScream, MonsterSoundOrigin(), alertVolume * RandRange(0.88f, 1.06f), 0.88f, 1.08f);
            audioRuntime_.game.monsterSpottedScreamCooldown = RandRange(5.2f, 8.0f);
            audioRuntime_.game.monsterAlertVocalTimer = RandRange(1.2f, 2.1f);
            return;
        }

        PlayMonsterAlertGroan(alertVolume);
        audioRuntime_.game.monsterAlertVocalTimer = visibleChase
            ? RandRange(1.05f, 2.35f)
            : RandRange(4.0f, 8.5f);
    }

    void UpdateVentMonsterGroans(float dt) {
        if (monsterPreview_.active || !IsPlayableSimulationMode(sessionRuntime_.mode) || !MonsterActiveForCurrentMode() || gameWorld_.deathActive || gameWorld_.exitTransitionActive) {
            audioRuntime_.game.ventMonsterGroanTimer = std::min(audioRuntime_.game.ventMonsterGroanTimer, 2.0f);
            audioRuntime_.game.ventMonsterGroanCooldown = std::max(0.0f, audioRuntime_.game.ventMonsterGroanCooldown - dt);
            return;
        }

        float tile = std::max(0.1f, gameWorld_.maze.TileAverage());
        float monsterTiles = MonsterDistance() / tile;
        float aroundFourTiles = 1.0f - Clamp01(std::abs(monsterTiles - 4.0f) / 2.15f);
        bool eligible = aroundFourTiles > 0.0f && !IsThreatVisible() && !ChasePanicActive();
        if (!eligible) {
            audioRuntime_.game.ventMonsterGroanTimer = std::min(audioRuntime_.game.ventMonsterGroanTimer, RandRange(2.2f, 5.4f));
            audioRuntime_.game.ventMonsterGroanCooldown = std::max(0.0f, audioRuntime_.game.ventMonsterGroanCooldown - dt);
            return;
        }

        audioRuntime_.game.ventMonsterGroanTimer = std::max(0.0f, audioRuntime_.game.ventMonsterGroanTimer - dt);
        audioRuntime_.game.ventMonsterGroanCooldown = std::max(0.0f, audioRuntime_.game.ventMonsterGroanCooldown - dt);
        if (audioRuntime_.game.ventMonsterGroanTimer > 0.0f || audioRuntime_.game.ventMonsterGroanCooldown > 0.0f) return;

        float chance = Lerp(0.18f, 0.52f, aroundFourTiles);
        if (RandRange(0.0f, 1.0f) < chance) {
            PlayVentMonsterGroan();
            audioRuntime_.game.ventMonsterGroanCooldown = RandRange(18.0f, 42.0f);
        }
        audioRuntime_.game.ventMonsterGroanTimer = RandRange(7.0f, 18.0f);
    }
