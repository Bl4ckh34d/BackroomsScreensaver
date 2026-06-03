    void UpdateLampFlickerStarterClicks(float dt) {
        if (!audioRuntime_.ready || monsterPreview_.active || effectRuntime_.runtimeLamps.empty() || settingsRuntime_.live.lampFlickerRatio <= 0.001f) return;
        float audibleRange = std::max(5.5f, gameWorld_.maze.TileAverage() * 4.75f);
        float audibleRangeSq = audibleRange * audibleRange;
        for (RuntimeLampState& lamp : effectRuntime_.runtimeLamps) {
            lamp.flickerClickCooldown = std::max(0.0f, lamp.flickerClickCooldown - dt);
            if (lamp.broken) {
                lamp.flickerWasDim = false;
                continue;
            }
            float dx = lamp.pos.x - gameWorld_.player.position.x;
            float dy = lamp.pos.y - gameWorld_.player.position.y;
            float dz = lamp.pos.z - gameWorld_.player.position.z;
            bool nearby = dx * dx + dy * dy + dz * dz <= audibleRangeSq;
            bool dim = nearby && RuntimeLampFlickerDim(lamp);
            if (nearby && lamp.flickerWasDim && !dim && lamp.flickerClickCooldown <= 0.0f) {
                QueueNeonFlickerStarterClickAt(lamp.pos);
                lamp.flickerClickCooldown = 0.18f;
            }
            lamp.flickerWasDim = dim;
        }
    }
