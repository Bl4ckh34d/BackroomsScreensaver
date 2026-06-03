    void UpdateBrokenRuntimeLampSparks(float dt, float minSeconds, float maxSeconds, float chainChance) {
        if (dt <= 0.0f || !settingsRuntime_.live.sparkParticles || effectRuntime_.runtimeLamps.empty()) return;
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            if (!lamp.broken) continue;
            if (!LampCanEmitSparks(lamp)) {
                lamp.sparkTimer = std::max(lamp.sparkTimer, maxSeconds);
                continue;
            }
            lamp.sparkTimer -= dt;
            if (lamp.sparkTimer > 0.0f) continue;

            float intensity = RandRange(0.42f, 1.28f);
            EmitSparkBurstAt(lamp.pos, intensity);
            if (RandRange(0.0f, 1.0f) < chainChance) {
                ScheduleSparkChain(lamp.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale,
                    std::max(1, PickBrokenLampChainBursts() / 2));
            }
            lamp.sparkTimer = RandRange(std::max(0.12f, minSeconds), std::max(minSeconds, maxSeconds));
        }
    }

    void UpdateMonsterLampDamage(float dt) {
        if (dt <= 0.0f || monsterPreview_.active || gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) return;
        if (effectRuntime_.runtimeLamps.empty() || effectRuntime_.lampDamagePixels.empty()) return;

        float tileAvg = std::max(0.1f, gameWorld_.maze.TileAverage());
        float influenceRadius = std::max(tileAvg * 4.30f, 6.50f);
        float breakRadius = influenceRadius * 0.72f;
        int maxTileReach = static_cast<int>(std::ceil(influenceRadius / std::max(0.1f, gameWorld_.maze.TileMinimum()))) + 1;

        struct MonsterLampInfluencePoint {
            XMFLOAT3 pos;
            Tile tile;
        };
        std::vector<MonsterLampInfluencePoint> influencePoints;
        float bodySpacing = MonsterBodySpacing();
        int bodySamples = std::clamp(static_cast<int>(std::ceil(MonsterBodyLengthMeters() / bodySpacing)) + 1, 4, 48);
        influencePoints.reserve(static_cast<size_t>(bodySamples));
        for (int i = 0; i < bodySamples; ++i) {
            XMFLOAT3 p = (i == 0) ? gameWorld_.monster.position : MonsterTrailSample(static_cast<float>(i) * bodySpacing);
            Tile tile = gameWorld_.maze.TileFromWorld(p.x, p.z);
            if (!gameWorld_.maze.IsOpen(tile.x, tile.y)) continue;
            influencePoints.push_back({ p, tile });
        }
        if (influencePoints.empty()) return;

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
    }
