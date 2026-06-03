        if (IsPlayableSimulationMode(sessionRuntime_.mode) || settingsRuntime_.live.debugAiMapOverlay) {
            for (const PlayerAudibleSoundPulse& pulse : soundPulses) {
                if (pulse.radius <= 0.05f || pulse.life <= 0.0f || pulse.age >= pulse.life) continue;
                float radiusSq = pulse.radius * pulse.radius;
                float fade = 1.0f - Clamp01(pulse.age / pulse.life);
                XMFLOAT4 hearingColor = pulse.heardByMonster
                    ? XMFLOAT4{1.0f, 0.02f, 0.02f, 0.18f + 0.30f * fade}
                    : XMFLOAT4{1.0f, 0.03f, 0.02f, 0.10f + 0.22f * fade};
                for (int y = 0; y < maze.h; ++y) {
                    for (int x = 0; x < maze.w; ++x) {
                        if (!maze.IsOpen(x, y)) continue;
                        Tile t{x, y};
                        if (playerExplorationMap && VisitCount(t) == 0 && !(t == cameraTile)) continue;
                        XMFLOAT3 center = maze.WorldCenter(t, 0.0f);
                        float dx = center.x - pulse.pos.x;
                        float dz = center.z - pulse.pos.z;
                        if (dx * dx + dz * dz <= radiusSq) {
                            pushTile(t, hearingColor, 0.02f);
                        }
                    }
                }
            }
        }
