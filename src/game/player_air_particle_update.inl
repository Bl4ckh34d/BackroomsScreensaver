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
