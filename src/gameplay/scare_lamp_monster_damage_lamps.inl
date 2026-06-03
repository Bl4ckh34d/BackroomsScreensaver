        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            if (lamp.broken) continue;

            float nearestDist = influenceRadius + 1.0f;
            bool affected = false;
            for (const MonsterLampInfluencePoint& point : influencePoints) {
                int tileDx = std::abs(lamp.tile.x - point.tile.x);
                int tileDz = std::abs(lamp.tile.y - point.tile.y);
                if (tileDx > maxTileReach || tileDz > maxTileReach) continue;
                float dx = lamp.pos.x - point.pos.x;
                float dz = lamp.pos.z - point.pos.z;
                float distSq = dx * dx + dz * dz;
                if (distSq > influenceRadius * influenceRadius) continue;
                if (!gameWorld_.maze.LineClear(point.tile, lamp.tile)) continue;
                float dist = std::sqrt(distSq);
                affected = true;
                nearestDist = std::min(nearestDist, dist);
            }
            float oldDamage = lamp.damage;

            if (affected) {
                float dist = nearestDist;
                float proximity = Clamp01((influenceRadius - dist) / std::max(0.001f, influenceRadius));
                float close = SmoothStep(0.0f, 1.0f, proximity);
                lamp.damage = std::max(lamp.damage, 0.12f * close);
                lamp.damage += dt * (0.10f + close * 0.92f + close * close * 0.65f);
                if (dist < breakRadius && lamp.damage > 0.58f) {
                    lamp.damage += dt * Lerp(0.18f, 0.58f, close);
                }
            } else if (lamp.damage > 0.055f) {
                lamp.damage += dt * (0.014f + lamp.damage * 0.036f);
            }

            lamp.damage = Clamp01(lamp.damage);
            if (lamp.damage > oldDamage + 0.001f) {
                MarkLampDamagePixel(lamp.tile, lamp.damage);
            }

            float fail = SmoothStep(0.28f, 0.96f, lamp.damage);
            if (fail > 0.001f) {
                lamp.sparkTimer -= dt * Lerp(0.85f, affected ? 3.5f : 1.7f, fail);
                if (lamp.sparkTimer <= 0.0f) {
                    if (settingsRuntime_.live.sparkParticles && LampCanEmitSparks(lamp)) {
                        float intensity = Lerp(0.68f, 3.20f, fail);
                        EmitSparkBurstAt(lamp.pos, intensity);
                        if (fail > 0.68f && RandRange(0.0f, 1.0f) < Lerp(0.16f, 0.62f, fail)) {
                            ScheduleSparkChain(lamp.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale,
                                std::max(1, PickBrokenLampChainBursts() / 2));
                        }
                    }
                    float cooldown = Lerp(1.75f, 0.12f, fail);
                    lamp.sparkTimer = RandRange(cooldown * 0.55f, cooldown * 1.25f);
                    if (affected && fail > 0.80f) {
                        lamp.damage = Clamp01(lamp.damage + 0.025f);
                        MarkLampDamagePixel(lamp.tile, lamp.damage);
                    }
                }
            }

            if (lamp.damage >= 0.995f) {
                // Monster-caused fixture failures are audible ambience, not player noise.
                BreakRuntimeLamp(lamp, false, true);
            }
        }
