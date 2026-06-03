// Wet floor/ceiling audio surface helpers. 
// Included inside Renderer's private section from renderer_audio.inl.

    void MarkWetFootstepTile(Tile tile) {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        if (index < effectRuntime_.wetFootstepTiles.size()) effectRuntime_.wetFootstepTiles[index] = 1;
    }

    void MarkWetFootstepArea(float px, float pz, float width, float depth, float yaw, float extra = 0.06f, float wetDelaySeconds = 0.0f) {
        WetFloorFootprint fp{};
        fp.center = {px, pz};
        float c = std::cos(yaw);
        float s = std::sin(yaw);
        fp.right = {c, -s};
        fp.forward = {s, c};
        fp.halfW = std::max(0.01f, width * 0.5f + extra);
        fp.halfD = std::max(0.01f, depth * 0.5f + extra);
        fp.wetDelaySeconds = std::max(0.0f, wetDelaySeconds);
        effectRuntime_.wetFloorFootprints.push_back(fp);
        if (effectRuntime_.wetFloorFootprints.size() > 4096) {
            effectRuntime_.wetFloorFootprints.erase(effectRuntime_.wetFloorFootprints.begin(), effectRuntime_.wetFloorFootprints.begin() + 512);
        }

        auto mark = [&](float lx, float lz) {
            float x = px + fp.right.x * lx + fp.forward.x * lz;
            float z = pz + fp.right.y * lx + fp.forward.y * lz;
            MarkWetFootstepTile(gameWorld_.maze.TileFromWorld(x, z));
        };
        MarkWetFootstepTile(gameWorld_.maze.TileFromWorld(px, pz));
        mark(-fp.halfW, -fp.halfD);
        mark( fp.halfW, -fp.halfD);
        mark(-fp.halfW,  fp.halfD);
        mark( fp.halfW,  fp.halfD);
    }

    bool IsWetFootstepTile(Tile tile) const {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return false;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        return index < effectRuntime_.wetFootstepTiles.size() && effectRuntime_.wetFootstepTiles[index] != 0;
    }

    bool IsNearWetFootstepTile(float x, float z) const {
        Tile center = gameWorld_.maze.TileFromWorld(x, z);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if (IsWetFootstepTile({center.x + dx, center.y + dy})) return true;
            }
        }
        return false;
    }

    bool MatureWaterRevealContains(float x, float z, float radius = 0.0f) const {
        for (const BloodRevealRegion& region : scareRuntime_.bloodRevealRegions) {
            if (!region.waterLiquid || region.radius <= 0.01f || region.activationTime <= -999000.0f) continue;
            float age = timeRuntime_.time - region.activationTime;
            if (age < 5.8f) continue;
            float grow = SmoothStep(5.8f, 24.0f, age);
            float activeRadius = Lerp(std::min(region.radius, 0.42f), region.radius * 0.86f, grow);
            float dx = x - region.center.x;
            float dz = z - region.center.z;
            float effectiveRadius = activeRadius + radius;
            if (dx * dx + dz * dz <= effectiveRadius * effectiveRadius) {
                return true;
            }
        }
        return false;
    }

    bool IsWetFootstepPoint(float x, float z, float radius = 0.08f) const {
        if (!MatureWaterRevealContains(x, z, radius)) return false;
        for (const WetFloorFootprint& fp : effectRuntime_.wetFloorFootprints) {
            if (fp.wetDelaySeconds > 0.0f) {
                bool delayedEnough = false;
                for (const BloodRevealRegion& region : scareRuntime_.bloodRevealRegions) {
                    if (!region.waterLiquid || region.activationTime <= -999000.0f) continue;
                    if (timeRuntime_.time - region.activationTime >= fp.wetDelaySeconds) {
                        delayedEnough = true;
                        break;
                    }
                }
                if (!delayedEnough) continue;
            }
            float dx = x - fp.center.x;
            float dz = z - fp.center.y;
            float localX = dx * fp.right.x + dz * fp.right.y;
            float localZ = dx * fp.forward.x + dz * fp.forward.y;
            if (std::abs(localX) <= fp.halfW + radius && std::abs(localZ) <= fp.halfD + radius) {
                return true;
            }
        }
        return false;
    }

    bool BloodWorldWetFootstepsActive() const {
        if (settingsRuntime_.live.bloodWorldCoverage <= 0.001f) return false;
        if (settingsRuntime_.live.bloodWorldAlwaysOn && settingsRuntime_.live.bloodWorldFlickerIntensity > 0.001f) return true;
        if (scareRuntime_.bloodWorldFlickerTimer > 0.0f && scareRuntime_.bloodWorldFlickerDuration > 0.001f &&
            settingsRuntime_.live.bloodWorldFlickerIntensity > 0.001f) {
            float elapsed = scareRuntime_.bloodWorldFlickerDuration - scareRuntime_.bloodWorldFlickerTimer;
            float envelope = SmoothStep(0.0f, 0.055f, elapsed) *
                (1.0f - SmoothStep(scareRuntime_.bloodWorldFlickerDuration - 0.18f, scareRuntime_.bloodWorldFlickerDuration, elapsed));
            float strobe = ((std::sin(elapsed * 41.0f) + std::sin(elapsed * 93.0f) * 0.48f +
                std::sin(elapsed * 151.0f) * 0.22f) > -0.06f) ? 1.0f : 0.0f;
            return envelope * strobe * settingsRuntime_.live.bloodWorldFlickerIntensity > 0.001f;
        }
        return false;
    }

    bool IsWetFootstepAtPlayer() const {
        if (BloodWorldWetFootstepsActive()) return true;

        float radius = std::max(0.28f, gameWorld_.maze.TileMinimum() * 0.22f);
        XMFLOAT3 forward{std::sin(gameWorld_.player.yaw), 0.0f, std::cos(gameWorld_.player.yaw)};
        XMFLOAT3 right{forward.z, 0.0f, -forward.x};
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { right.x * radius, right.z * radius },
            {-right.x * radius, -right.z * radius },
            { forward.x * radius, forward.z * radius },
            {-forward.x * radius, -forward.z * radius }
        };
        for (const XMFLOAT2& sample : samples) {
            if (IsWetFootstepPoint(gameWorld_.player.position.x + sample.x, gameWorld_.player.position.z + sample.y)) {
                return true;
            }
        }
        return false;
    }

    void MarkWetCeilingTile(Tile tile) {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        if (index < effectRuntime_.wetCeilingTiles.size()) effectRuntime_.wetCeilingTiles[index] = 1;
    }

    bool IsWetCeilingTile(Tile tile) const {
        if (!gameWorld_.maze.InBounds(tile.x, tile.y)) return false;
        size_t index = static_cast<size_t>(tile.y * gameWorld_.maze.w + tile.x);
        return index < effectRuntime_.wetCeilingTiles.size() && effectRuntime_.wetCeilingTiles[index] != 0;
    }

    void MarkWetCeilingDripEmitter(XMFLOAT3 pos, float seed) {
        pos.y = 0.10f;
        for (const WetDripEmitter& existing : effectRuntime_.wetDripEmitters) {
            float dx = existing.pos.x - pos.x;
            float dz = existing.pos.z - pos.z;
            if (dx * dx + dz * dz < 0.20f) return;
        }
        if (effectRuntime_.wetDripEmitters.size() >= 160) return;

        int sx = static_cast<int>(std::floor(pos.x * 41.0f + seed * 97.0f));
        int sz = static_cast<int>(std::floor(pos.z * 43.0f - seed * 83.0f));
        WetDripEmitter emitter{};
        emitter.pos = pos;
        emitter.interval = Lerp(1.0f / 3.0f, 2.0f, Rand01(sx, sz, sessionRuntime_.runtimeSeed ^ 0xD21F5u));
        emitter.timer = emitter.interval * Rand01(sx + 19, sz - 31, sessionRuntime_.runtimeSeed ^ 0x9E772u);
        emitter.volume = Lerp(0.24f, 0.38f, Rand01(sx - 7, sz + 23, sessionRuntime_.runtimeSeed ^ 0x512D9u));
        emitter.age = 0.0f;
        emitter.audibleDelay = Lerp(7.5f, 10.5f, Rand01(sx + 53, sz - 47, sessionRuntime_.runtimeSeed ^ 0x71A45u));
        effectRuntime_.wetDripEmitters.push_back(emitter);
    }

    void UpdateWetDripAudio(float dt) {
        if (monsterPreview_.active || sessionRuntime_.mode == RendererRuntimeMode::MainMenu) return;
        float audibleRange = std::max(18.0f, gameWorld_.maze.TileAverage() * 12.0f);
        for (WetDripEmitter& emitter : effectRuntime_.wetDripEmitters) {
            emitter.age += std::max(0.0f, dt);
            if (emitter.age < emitter.audibleDelay) continue;

            emitter.timer -= dt;
            if (emitter.timer > 0.0f) continue;
            if (DistanceToPoint(emitter.pos) <= audibleRange) {
                DispatchGameAudioEvent(GameAudioEvent::OneShot(
                    GameSound::WetCarpetCeilingDrip,
                    AudioBus::Effects,
                    emitter.pos,
                    emitter.volume * RandRange(0.90f, 1.10f),
                    true,
                    true).WithCategory(GameAudioEventCategory::WetDrip));
            }
            emitter.timer += std::max(1.0f / 3.0f, emitter.interval);
            if (emitter.timer <= 0.0f) emitter.timer = emitter.interval;
        }
    }
