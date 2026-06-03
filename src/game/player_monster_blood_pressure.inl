    void UpdateMonsterProximityBlood(float dt) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (world.deathActive || world.exitTransitionActive || scareRuntime_.bloodScarePoints.empty()) return;
        float monsterRange = std::max(0.1f, settingsRuntime_.live.dreadMonsterDistance);
        float proximity = SmoothStep(0.16f, 1.0f, Clamp01((monsterRange - MonsterDistance()) / monsterRange));
        if (proximity <= 0.001f) {
            scareRuntime_.proximityBloodPulseCooldown = std::min(scareRuntime_.proximityBloodPulseCooldown, 9.0f);
            return;
        }

        scareRuntime_.proximityBloodPulseCooldown -= dt;
        if (scareRuntime_.proximityBloodPulseCooldown > 0.0f) return;

        Tile cameraTile = CameraTile();
        float tileMin = std::max(0.1f, RenderMazeView().TileMinimum());
        float tileAvg = std::max(0.1f, RenderMazeView().TileAverage());
        float maxDist = Lerp(tileAvg * 4.2f, tileAvg * 8.8f, proximity);
        XMFLOAT3 forward = FlashlightForward();

        int revealCount = 1;
        if (proximity > 0.64f && RandRange(0.0f, 1.0f) < 0.42f) revealCount = 2;
        if (proximity > 0.86f && RandRange(0.0f, 1.0f) < 0.24f) revealCount = 3;

        for (int reveal = 0; reveal < revealCount; ++reveal) {
            int bestIndex = -1;
            float bestScore = -1.0e9f;
            for (size_t i = 0; i < scareRuntime_.bloodScarePoints.size(); ++i) {
                const BloodScarePoint& point = scareRuntime_.bloodScarePoints[i];
                if (point.triggered || !point.revealBlood || point.waterLiquid) continue;
                float dx = point.pos.x - world.playerPosition.x;
                float dz = point.pos.z - world.playerPosition.z;
                float distSq = dx * dx + dz * dz;
                if (distSq < tileMin * tileMin * 0.36f || distSq > maxDist * maxDist) continue;
                float dist = std::sqrt(distSq);
                Tile bloodTile = RenderMazeView().TileFromWorld(point.pos.x, point.pos.z);
                float lineBonus = RenderMazeView().LineClear(cameraTile, bloodTile) ? 2.4f : 0.0f;
                float ahead = dist > 0.001f ? (dx * forward.x + dz * forward.z) / dist : 0.0f;
                float aheadBonus = SmoothStep(-0.20f, 0.78f, ahead) * 1.7f;
                float nearPreferred = 1.0f - std::abs(dist - tileAvg * 3.2f) / std::max(tileAvg * 4.5f, 0.1f);
                float score = nearPreferred * 2.6f + lineBonus + aheadBonus + RandRange(0.0f, 1.0f);
                if (score > bestScore) {
                    bestScore = score;
                    bestIndex = static_cast<int>(i);
                }
            }
            if (bestIndex < 0) break;

            BloodScarePoint& point = scareRuntime_.bloodScarePoints[static_cast<size_t>(bestIndex)];
            point.triggered = true;
            point.activationTime = timeRuntime_.time - RandRange(0.75f, 2.8f) * (0.72f + proximity * 0.86f);
            IncludeBloodReveal(point);
            scareRuntime_.bloodScareActiveUntil = std::max(scareRuntime_.bloodScareActiveUntil, timeRuntime_.time + 150.0f);
        }

        float scareScale = ScareCooldownScale();
        float minSeconds = Lerp(8.5f, 1.35f, proximity) * scareScale;
        float maxSeconds = Lerp(14.0f, 2.80f, proximity) * scareScale;
        scareRuntime_.proximityBloodPulseCooldown = RandRange(std::max(0.85f, minSeconds), std::max(1.20f, maxSeconds));
        if (proximity > 0.50f) {
            viewRuntime_.dreadLevel = Clamp01(viewRuntime_.dreadLevel + proximity * 0.018f);
        }
    }
