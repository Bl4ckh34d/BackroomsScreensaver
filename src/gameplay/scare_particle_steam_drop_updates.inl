    void UpdateSteamAndDrops(float dt) {
        for (SteamParticle& sp : effectRuntime_.steam) {
            sp.age += dt;
            sp.vel.y += 0.18f * dt;
            sp.vel.x *= std::max(0.0f, 1.0f - dt * 0.42f);
            sp.vel.z *= std::max(0.0f, 1.0f - dt * 0.42f);
            sp.pos.x += sp.vel.x * dt;
            sp.pos.y += sp.vel.y * dt;
            sp.pos.z += sp.vel.z * dt;
        }
        effectRuntime_.steam.erase(std::remove_if(effectRuntime_.steam.begin(), effectRuntime_.steam.end(), [](const SteamParticle& sp) {
            return sp.age >= sp.life;
        }), effectRuntime_.steam.end());

        for (VentDrop& d : effectRuntime_.ventDrops) {
            d.age += dt;
            if (!d.landed) {
                d.vel.y -= 4.6f * dt;
                d.pos.x += d.vel.x * dt;
                d.pos.y += d.vel.y * dt;
                d.pos.z += d.vel.z * dt;
                d.roll += d.angular * dt;
                if (d.pos.y < kVentDropFloorY || d.age >= d.life) {
                    d.pos.y = kVentDropFloorY;
                    d.vel.y = -d.vel.y * 0.18f;
                    d.vel.x *= 0.38f;
                    d.vel.z *= 0.38f;
                    d.angular *= 0.28f;
                    if (std::abs(d.vel.y) < 0.16f || d.age >= d.life) {
                        d.landed = true;
                        d.vel = {};
                        d.angular = 0.0f;
                        d.roll = kPi * 0.5f;
                        SparkEmitter impact{{d.pos.x, d.pos.y + 0.08f, d.pos.z}, 0.0f};
                        SpawnSparkBurst(impact, 1.6f);
                        gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                            impact.pos,
                            TileHearingRadius(8.0f),
                            1.05f,
                            GameAudioEventCategory::Scare));
                    }
                }
            }
        }
    }
