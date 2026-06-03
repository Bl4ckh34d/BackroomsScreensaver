// Air particle simulation and adaptive budget helpers. 
// Included inside Renderer's private section from player_camera_movement.inl.

    void UpdateAirParticleFocus(float dt) {
        float target = FlashlightFocusTargetDistance();
        if (viewRuntime_.airFocusDistance <= 0.0f) viewRuntime_.airFocusDistance = target;
        float follow = std::min(1.0f, std::max(0.001f, dt) * 1.85f);
        viewRuntime_.airFocusDistance += (target - viewRuntime_.airFocusDistance) * follow;
    }

    void UpdateAirParticlePerformanceBudget(float dt) {
        if (!settingsRuntime_.live.airParticles || monsterPreview_.active) {
            effectRuntime_.airParticleBudgetScale = 1.0f;
            effectRuntime_.airParticleFrameDt = 0.0f;
            return;
        }
        if (dt <= 0.0f || dt >= 0.20f) return;
        if (effectRuntime_.airParticleFrameDt <= 0.0f) effectRuntime_.airParticleFrameDt = dt;
        effectRuntime_.airParticleFrameDt += (dt - effectRuntime_.airParticleFrameDt) * std::min(1.0f, dt * 2.0f);

        float target = 1.0f;
        if (timeRuntime_.time > 2.0f) {
            if (effectRuntime_.airParticleFrameDt > 0.034f) target = 0.40f;
            else if (effectRuntime_.airParticleFrameDt > 0.028f) target = 0.55f;
            else if (effectRuntime_.airParticleFrameDt > 0.023f) target = 0.72f;
            else if (effectRuntime_.airParticleFrameDt > 0.019f) target = 0.88f;
        }
        float follow = target < effectRuntime_.airParticleBudgetScale
            ? std::min(1.0f, dt * 3.0f)
            : std::min(1.0f, dt * 0.22f);
        effectRuntime_.airParticleBudgetScale += (target - effectRuntime_.airParticleBudgetScale) * follow;
        effectRuntime_.airParticleBudgetScale = std::clamp(effectRuntime_.airParticleBudgetScale, 0.34f, 1.0f);
    }

    float AirParticleLevelDensityScale() const {
        if (!IsPlayableSimulationMode(sessionRuntime_.mode)) return 1.0f;
        return gameWorld_.AirParticleDensityScale();
    }

    int DesiredAirParticleCount() const {
        if (!settingsRuntime_.live.airParticles || settingsRuntime_.live.airParticleDensity <= 0.001f || monsterPreview_.active) return 0;
        float density = std::clamp(settingsRuntime_.live.airParticleDensity, 0.0f, 4.0f) * AirParticleLevelDensityScale();
        float budget = std::clamp(effectRuntime_.airParticleBudgetScale, 0.34f, 1.0f);
        return std::clamp(static_cast<int>(std::round(2800.0f * density * budget)), 0, 9000);
    }

    void RespawnAirParticle(AirParticle& p, bool initial) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        float radius = std::clamp(settingsRuntime_.live.flashlightShadowDistanceMeters * 0.70f, 6.0f, 12.0f);
        XMFLOAT3 forward{std::sin(viewRuntime_.flashlightYaw), 0.0f, std::cos(viewRuntime_.flashlightYaw)};
        XMFLOAT3 right{std::cos(viewRuntime_.flashlightYaw), 0.0f, -std::sin(viewRuntime_.flashlightYaw)};
        XMFLOAT3 pos = world.playerPosition;
        float coneHalf = std::clamp(settingsRuntime_.live.flashlightConeDegrees, 20.0f, 140.0f) * 0.5f * kPi / 180.0f;
        float coneSpread = std::tan(std::min(coneHalf * 0.72f, 1.12f));
        float layerRoll = RandRange(0.0f, 1.0f);
        p.nearLayer = layerRoll < 0.018f ? 2.0f : (layerRoll < 0.165f ? 1.0f : 0.0f);
        for (int attempt = 0; attempt < 24; ++attempt) {
            float depthT = std::pow(RandRange(0.0f, 1.0f), 0.42f);
            float depth = Lerp(0.45f, radius, depthT);
            if (p.nearLayer > 1.5f) {
                depth = RandRange(0.24f, 1.20f);
            } else if (p.nearLayer > 0.5f) {
                depth = RandRange(0.65f, 2.85f);
            } else if (RandRange(0.0f, 1.0f) < 0.10f) {
                depth = RandRange(0.30f, radius);
            }
            float sideLimit = std::clamp(depth * coneSpread, 0.22f, radius * 0.82f);
            float side = RandRange(-sideLimit, sideLimit);
            float yMin = p.nearLayer > 0.5f ? 0.34f : 0.22f;
            float yMax = std::max(yMin + 0.02f, settingsRuntime_.live.wallHeightMeters - (p.nearLayer > 0.5f ? 0.24f : 0.14f));
            float y = RandRange(yMin, yMax);
            pos = Add3({world.playerPosition.x, y, world.playerPosition.z}, Add3(Scale3(forward, depth), Scale3(right, side)));
            Tile tile = maze.TileFromWorld(pos.x, pos.z);
            if (maze.IsOpen(tile.x, tile.y)) break;
        }

        p.pos = pos;
        float driftRoll = std::pow(RandRange(0.0f, 1.0f), 1.35f);
        float driftScale = Lerp(0.36f, 2.15f, driftRoll);
        if (RandRange(0.0f, 1.0f) < 0.075f) driftScale *= RandRange(1.35f, 2.25f);
        if (p.nearLayer > 0.5f) driftScale *= RandRange(0.78f, 1.32f);
        p.vel = {
            RandRange(-0.030f, 0.030f) * driftScale,
            RandRange(-0.010f, 0.026f) * driftScale,
            RandRange(-0.030f, 0.030f) * driftScale
        };
        p.life = RandRange(28.0f, 68.0f);
        p.age = initial ? RandRange(0.0f, p.life * 0.82f) : 0.0f;
        float sizeRoll = RandRange(0.0f, 1.0f);
        float sizeT = RandRange(0.0f, 1.0f);
        float baseSize = 0.0f;
        if (sizeRoll < 0.50f) {
            baseSize = Lerp(0.0018f, 0.0048f, std::pow(sizeT, 1.35f));
        } else if (sizeRoll < 0.90f) {
            baseSize = Lerp(0.0048f, 0.0083f, sizeT);
        } else {
            baseSize = Lerp(0.0083f, 0.0114f, std::sqrt(sizeT));
        }
        float layerScale = p.nearLayer > 1.5f ? RandRange(1.28f, 2.02f) : (p.nearLayer > 0.5f ? RandRange(1.10f, 1.62f) : 1.0f);
        p.size = baseSize * layerScale * std::clamp(settingsRuntime_.live.airParticleSize, 0.20f, 4.0f);
        float aspectRoll = RandRange(0.0f, 1.0f);
        if (aspectRoll < 0.34f) {
            p.aspect = RandRange(1.95f, 4.10f);
        } else if (aspectRoll < 0.66f) {
            p.aspect = RandRange(0.24f, 0.56f);
        } else {
            p.aspect = RandRange(0.62f, 1.62f);
        }
        p.seed = RandRange(0.0f, 1.0f);
        p.angle = RandRange(0.0f, kPi * 2.0f);
        p.spin = RandRange(-0.18f, 0.18f);
    }

    void UpdateAirParticles(float dt) {
        int desired = DesiredAirParticleCount();
        if (desired <= 0) {
            effectRuntime_.airParticles.clear();
            return;
        }
        if (effectRuntime_.airParticles.size() < static_cast<size_t>(desired)) {
            size_t oldSize = effectRuntime_.airParticles.size();
            effectRuntime_.airParticles.resize(static_cast<size_t>(desired));
            for (size_t i = oldSize; i < effectRuntime_.airParticles.size(); ++i) {
                RespawnAirParticle(effectRuntime_.airParticles[i], true);
            }
        } else if (effectRuntime_.airParticles.size() > static_cast<size_t>(desired)) {
            effectRuntime_.airParticles.resize(static_cast<size_t>(desired));
        }

        float step = std::min(std::max(dt, 0.0f), 0.10f);
        float keepRadius = std::clamp(settingsRuntime_.live.flashlightShadowDistanceMeters * 0.78f, 6.5f, 13.0f);
        float keepRadiusSq = keepRadius * keepRadius;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        for (size_t i = 0; i < effectRuntime_.airParticles.size(); ++i) {
            AirParticle& p = effectRuntime_.airParticles[i];
            p.age += step;
            float driftA = p.seed * kPi * 2.0f;
            p.pos.x += (p.vel.x + std::sin(timeRuntime_.time * 0.19f + driftA) * 0.012f) * step;
            p.pos.y += (p.vel.y + std::sin(timeRuntime_.time * 0.13f + driftA * 1.7f) * 0.008f) * step;
            p.pos.z += (p.vel.z + std::cos(timeRuntime_.time * 0.17f + driftA * 1.3f) * 0.012f) * step;
            p.angle += p.spin * step;

            float dx = p.pos.x - world.playerPosition.x;
            float dz = p.pos.z - world.playerPosition.z;
            bool validateTile = ((static_cast<int>(i) + effectRuntime_.airParticleValidationCursor) & 3) == 0;
            bool blocked = false;
            if (validateTile) {
                Tile tile = maze.TileFromWorld(p.pos.x, p.pos.z);
                blocked = !maze.IsOpen(tile.x, tile.y);
            }
            if (p.age >= p.life || p.pos.y < 0.16f || p.pos.y > settingsRuntime_.live.wallHeightMeters - 0.10f ||
                dx * dx + dz * dz > keepRadiusSq || blocked) {
                RespawnAirParticle(p, false);
            }
        }
        effectRuntime_.airParticleValidationCursor = (effectRuntime_.airParticleValidationCursor + 1) & 3;
    }
