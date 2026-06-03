            } else {
                float creepSurge = 0.90f + std::pow(std::max(0.0f, std::sin(timeRuntime_.time * 0.36f + gameWorld_.monster.position.x * 0.10f + gameWorld_.monster.position.z * 0.14f)), 8.0f) * 0.34f;
                float speed = 1.08f * settingsRuntime_.live.monsterSpeed * creepSurge;
                if (seesPlayer) {
                    float proximity = Clamp01((7.2f - MonsterDistance()) / 6.2f);
                    float impulseSeed = Rand01(mt.x * 43 + mt.y * 59 + static_cast<int>(timeRuntime_.time * 2.1f), 1409, sessionRuntime_.runtimeSeed);
                    float impulse = std::pow(std::max(0.0f,
                        std::sin(timeRuntime_.time * Lerp(2.1f, 4.4f, impulseSeed) + gameWorld_.monster.position.x * 0.41f - gameWorld_.monster.position.z * 0.23f)), Lerp(2.7f, 7.5f, impulseSeed));
                    float stutter = Lerp(0.94f, 1.16f, impulse);
                    speed = Lerp(2.40f, 3.98f, proximity) * settingsRuntime_.live.monsterSprintSpeed * (0.88f + creepSurge * 0.12f) * stutter;
                } else if (gameWorld_.monster.hasLastKnown) {
                    float searchSeed = Rand01(mt.x * 23 + mt.y * 71 + static_cast<int>(timeRuntime_.time * 1.4f), 1423, sessionRuntime_.runtimeSeed);
                    float searchPulse = std::pow(std::max(0.0f,
                        std::sin(timeRuntime_.time * Lerp(1.2f, 3.1f, searchSeed) + gameWorld_.monster.position.z * 0.35f)), Lerp(2.5f, 6.0f, searchSeed));
                    speed = 2.12f * settingsRuntime_.live.monsterSprintSpeed * Lerp(0.80f, 1.20f, searchPulse) * (0.92f + creepSurge * 0.10f);
                } else if (gameWorld_.monster.hasSound) {
                    speed = 2.30f * settingsRuntime_.live.monsterSprintSpeed * (0.94f + creepSurge * 0.08f);
                } else {
                    float pulseSeed = Rand01(mt.x * 19 + mt.y * 37 + static_cast<int>(gameWorld_.monster.pathIndex) * 11, 1289, sessionRuntime_.runtimeSeed);
                    float lizardPulse = 0.5f + 0.5f *
                        std::sin(timeRuntime_.time * Lerp(0.55f, 1.05f, pulseSeed) + gameWorld_.monster.position.x * 0.19f + gameWorld_.monster.position.z * 0.17f);
                    lizardPulse = SmoothStep(0.0f, 1.0f, Clamp01(lizardPulse));
                    float burst = gameWorld_.monster.roamBurstTimer > 0.0f ? SmoothStep(0.0f, 1.0f, Clamp01(gameWorld_.monster.roamBurstTimer / 0.82f)) : 0.0f;
                    float burstNoise = Rand01(mt.x * 17 + mt.y * 31 + static_cast<int>(timeRuntime_.time * 1.7f), 1297, sessionRuntime_.runtimeSeed);
                    float slowFlow = Lerp(0.80f, 1.05f, lizardPulse);
                    float rush = Lerp(1.45f, 2.15f, burstNoise);
                    speed = settingsRuntime_.live.monsterSpeed * Lerp(slowFlow, rush, burst);
                    gameWorld_.monster.roamBurstTimer = std::max(0.0f, gameWorld_.monster.roamBurstTimer - dt);
                    if (gameWorld_.monster.roamBurstTimer <= 0.0f && burst > 0.35f && RandRange(0.0f, 1.0f) < 0.05f) {
                        gameWorld_.monster.roamPauseTimer = RandRange(0.28f, 0.72f);
                    }
                }
                MoveMonsterToward(target, std::min(dist, speed * dt));
                moved = true;
            }
        }
        return makeOutput(seesPlayer, heardPlayer, ValidMonsterTile(gameWorld_.monster.goal), moved);
