        if (settingsRuntime_.live.airVentScaresEnabled) {
            for (SteamEmitter& emitter : effectRuntime_.steamEmitters) {
                if (emitter.triggered) continue;
                if (!PointWithinHorizontalRange(emitter.pos, gameWorld_.maze.TileAverage() * 3.25f)) continue;
                if (!ScareSourceAhead(emitter.pos,
                    gameWorld_.maze.TileMinimum() * 0.82f,
                    gameWorld_.maze.TileAverage() * 2.80f,
                    4,
                    0.08f)) continue;
                float sensory = std::max(ScareSensoryWeight(emitter.pos, 7.8f, 0.76f, 2.10f), 0.70f);
                if (sensory > 0.0f && scareRuntime_.scareCooldown <= 0.0f && customScareAllowed(1)) {
                    emitter.triggered = true;
                    gameWorld_.QueueAudioEvent(GameAudioEvent::PlayerNoise(
                        emitter.pos,
                        AirVentHearingRadius(),
                        1.18f,
                        GameAudioEventCategory::Scare));
                    SpawnSteamBurst(emitter, PickAirVentSteamIntensity());
                    if (!emitter.panelDropped &&
                        RandRange(0.0f, 1.0f) < settingsRuntime_.live.effectAirVentPanelDropChance &&
                        SpawnVentDrop(emitter)) {
                        emitter.panelDropped = true;
                        viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.34f + sensory * 0.41f);
                        AddDread(settingsRuntime_.live.dreadJumpscareGain * 0.78f * sensory);
                    }
                    scareRuntime_.scareCooldown = RandRange(10.0f, 21.0f) * scareScale;
                    AddDread(settingsRuntime_.live.dreadJumpscareGain * 0.62f * sensory);
                    viewRuntime_.flashlightAgitation = std::max(viewRuntime_.flashlightAgitation, 0.28f + sensory * 0.36f);
                    BeginVentReaction(emitter, sensory);
                }
            }
        }
