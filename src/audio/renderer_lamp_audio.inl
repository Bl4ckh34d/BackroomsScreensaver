// Runtime lamp hum and flicker audio helpers. 
// Included inside Renderer's private section from renderer_audio.inl.

    int LampHumVoiceTag(size_t lampIndex) const {
        return 100000 + static_cast<int>(lampIndex);
    }

    bool LampHumAudibleCandidate(const RuntimeLampState& lamp, float maxDistSq) const {
        if (lamp.broken) return false;
        float dx = lamp.pos.x - gameWorld_.player.position.x;
        float dy = lamp.pos.y - gameWorld_.player.position.y;
        float dz = lamp.pos.z - gameWorld_.player.position.z;
        return dx * dx + dy * dy + dz * dz <= maxDistSq;
    }

    void UpdatePersistentLampHumVoices(float dt, bool force = false) {
        if (!audioRuntime_.ready) return;
        if (!force) {
            audioRuntime_.game.lampHumRefreshTimer -= std::max(0.0f, dt);
            if (audioRuntime_.game.lampHumRefreshTimer > 0.0f) return;
        }
        audioRuntime_.game.lampHumRefreshTimer = force ? 0.0f : 0.22f;

        float maxDist = std::clamp(gameWorld_.maze.TileAverage() * 5.5f, 5.5f, 7.9f);
        float startDistSq = maxDist * maxDist;
        float stopDist = maxDist + std::max(1.75f, gameWorld_.maze.TileAverage() * 0.9f);
        float stopDistSq = stopDist * stopDist;
        constexpr int kMaxActiveLampHums = 24;

        std::vector<LampHumCandidate>& candidates = audioRuntime_.game.lampHumCandidates;
        candidates.clear();
        candidates.reserve(std::min<size_t>(effectRuntime_.runtimeLamps.size(), kMaxActiveLampHums * 2));
        for (size_t i = 0; i < effectRuntime_.runtimeLamps.size(); ++i) {
            const RuntimeLampState& lamp = effectRuntime_.runtimeLamps[i];
            if (!LampHumAudibleCandidate(lamp, startDistSq)) continue;
            if (RuntimeLampFlickerDim(lamp)) continue;
            float dx = lamp.pos.x - gameWorld_.player.position.x;
            float dy = lamp.pos.y - gameWorld_.player.position.y;
            float dz = lamp.pos.z - gameWorld_.player.position.z;
            candidates.push_back({i, dx * dx + dy * dy + dz * dz});
        }
        auto byDistance = [](const LampHumCandidate& a, const LampHumCandidate& b) {
            return a.distSq < b.distSq;
        };
        if (candidates.size() > kMaxActiveLampHums) {
            std::nth_element(candidates.begin(), candidates.begin() + kMaxActiveLampHums, candidates.end(), byDistance);
            candidates.resize(kMaxActiveLampHums);
        }
        std::sort(candidates.begin(), candidates.end(), byDistance);

        audioRuntime_.game.lampHumShouldPlay.assign(effectRuntime_.runtimeLamps.size(), 0);
        std::vector<uint8_t>& shouldPlay = audioRuntime_.game.lampHumShouldPlay;
        for (const LampHumCandidate& candidate : candidates) {
            shouldPlay[candidate.index] = 1;
        }

        for (size_t i = 0; i < effectRuntime_.runtimeLamps.size(); ++i) {
            int tag = LampHumVoiceTag(i);
            const RuntimeLampState& lamp = effectRuntime_.runtimeLamps[i];
            float dx = lamp.pos.x - gameWorld_.player.position.x;
            float dy = lamp.pos.y - gameWorld_.player.position.y;
            float dz = lamp.pos.z - gameWorld_.player.position.z;
            bool tooFar = dx * dx + dy * dy + dz * dz > stopDistSq;
            if (lamp.broken || tooFar || RuntimeLampFlickerDim(lamp) || i >= shouldPlay.size() || !shouldPlay[i]) {
                audioRuntime_.engine.StopTaggedVoice(tag);
                continue;
            }
            if (audioRuntime_.engine.HasTaggedVoice(tag)) continue;

            GameSound hum = GameSound::NeonHumQuiet;
            if (lamp.humVariant == 1) hum = GameSound::NeonHumLoud;
            if (lamp.humVariant == 2) hum = GameSound::NeonHumLoud2;
            float baseVolume = 0.045f * 1.15f * 0.80f;
            float lampVariation = Lerp(0.84f, 1.18f,
                Rand01(lamp.tile.x * 19 + 17, lamp.tile.y * 29 + 31, sessionRuntime_.runtimeSeed ^ 0xA04D10u));
            float damageLift = Lerp(0.94f, 1.10f, Clamp01(lamp.damage));
            float volume = baseVolume * lampVariation * damageLift;
            uint32_t tx = static_cast<uint32_t>(lamp.tile.x + 4096);
            uint32_t ty = static_cast<uint32_t>(lamp.tile.y + 4096);
            uint32_t stableHumId = sessionRuntime_.runtimeSeed ^ (tx * 73856093u) ^ (ty * 19349663u) ^
                (static_cast<uint32_t>(lamp.humVariant + 17) * 83492791u);
            size_t humSample = audioRuntime_.engine.PickStableSample(hum, stableHumId);
            audioRuntime_.engine.StartLoopTaggedSample(hum, humSample, AudioBus::Ambience, lamp.pos, volume, true, tag, AudioOcclusionFor(lamp.pos));
        }
    }

    bool RuntimeLampFlickerDim(const RuntimeLampState& lamp) const {
        float cellX = static_cast<float>(lamp.tile.x);
        float cellZ = static_cast<float>(lamp.tile.y);
        bool flickerFixture = LampHash(cellX + 17.0f, cellZ + 17.0f) >= 1.0f - settingsRuntime_.live.lampFlickerRatio;
        if (!flickerFixture) return false;
        float h = LampHash(cellX, cellZ);
        float tick = std::floor(timeRuntime_.time * (1.3f + LampHash(cellX + 37.0f, cellZ + 37.0f) * 2.5f));
        bool event = LampHash(cellX + tick + 71.0f, cellZ + tick + 71.0f) >= 0.86f;
        if (!event) return false;
        float flutter = 0.18f + 0.82f * Clamp01(std::sin(timeRuntime_.time * (41.0f + h * 50.0f)) * 0.5f + 0.5f);
        return flutter < 0.42f;
    }

    void UpdateLampFlickerStarterClicks(float dt) {
        if (!audioRuntime_.ready || monsterPreview_.active || effectRuntime_.runtimeLamps.empty() || settingsRuntime_.live.lampFlickerRatio <= 0.001f) return;
        float audibleRange = std::max(5.5f, gameWorld_.maze.TileAverage() * 4.75f);
        float audibleRangeSq = audibleRange * audibleRange;
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            lamp.flickerClickCooldown = std::max(0.0f, lamp.flickerClickCooldown - dt);
            if (lamp.broken) {
                lamp.flickerWasDim = false;
                continue;
            }
            float dx = lamp.pos.x - gameWorld_.player.position.x;
            float dy = lamp.pos.y - gameWorld_.player.position.y;
            float dz = lamp.pos.z - gameWorld_.player.position.z;
            bool nearby = dx * dx + dy * dy + dz * dz <= audibleRangeSq;
            bool dim = nearby && RuntimeLampFlickerDim(lamp);
            if (nearby && lamp.flickerWasDim && !dim && lamp.flickerClickCooldown <= 0.0f) {
                QueueNeonFlickerStarterClickAt(lamp.pos);
                lamp.flickerClickCooldown = 0.18f;
            }
            lamp.flickerWasDim = dim;
        }
    }
