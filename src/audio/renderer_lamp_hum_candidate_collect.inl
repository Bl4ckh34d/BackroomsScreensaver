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
