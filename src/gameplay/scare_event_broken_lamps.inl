        Tile currentTile = CameraTile();
        bool enteredTile = !(currentTile == scareRuntime_.scareEventTile);
        if (enteredTile) {
            scareRuntime_.scareEventTile = currentTile;
        }

        if (settingsRuntime_.live.brokenLampScaresEnabled) {
            for (SparkEmitter& emitter : effectRuntime_.sparkEmitters) {
                if (emitter.triggered) continue;
                if (!PointWithinHorizontalRange(emitter.pos, gameWorld_.maze.TileAverage() * 3.1f)) continue;
                if (!ScareSourceAhead(emitter.pos,
                    gameWorld_.maze.TileMinimum() * 0.78f,
                    gameWorld_.maze.TileAverage() * 2.65f,
                    4,
                    0.10f)) continue;
                float sensory = std::max(ScareSensoryWeight(emitter.pos, 8.5f, 0.80f, 2.35f), 0.72f);
                if (scareRuntime_.scareCooldown <= 0.0f && sensory > 0.0f && customScareAllowed(0)) {
                    emitter.triggered = true;
                    float intensity = PickBrokenLampSparkIntensity();
                    gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                        emitter.pos,
                        SparkHearingRadius(intensity),
                        1.42f,
                        GameAudioEventCategory::Scare));
                    if (!BreakNearestRuntimeLampAt(emitter.pos, gameWorld_.maze.TileMinimum() * 0.46f)) {
                        SpawnSparkBurst(emitter, intensity);
                        ScheduleSparkChain(emitter.pos, intensity * settingsRuntime_.live.effectBrokenLampChainIntensityScale, PickBrokenLampChainBursts());
                    }
                    scareRuntime_.scareCooldown = RandRange(9.0f, 18.0f) * scareScale;
                    viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.42f + sensory * 0.43f);
                    AddDread(settingsRuntime_.live.dreadJumpscareGain *
                        Clamp01(intensity / std::max(0.1f, settingsRuntime_.live.effectBrokenLampSparkIntensityMax)) * sensory);
                    if (sensory > 0.55f) {
                        viewRuntime_.stumbleTimer = std::max(viewRuntime_.stumbleTimer, 0.12f + sensory * 0.06f);
                        viewRuntime_.stumbleDuration = std::max(viewRuntime_.stumbleDuration, 0.16f + sensory * 0.06f);
                        viewRuntime_.stumbleYawOffset = RandRange(-0.18f, 0.18f) * sensory;
                    }
                }
            }
        }
