        if (!specialMonsterView) {
            Tile cameraTile = CameraTile();
            bool cameraTileOpen = maze.IsOpen(cameraTile.x, cameraTile.y);
            XMFLOAT3 viewForward = DirectionFromYawPitch(world.playerYaw, world.playerPitch);
            float renderRadius = std::max(tileScale * 0.58f, modelXZ * 0.85f);
            float maxRenderDistance = std::max(settingsRuntime_.live.fogEndMeters + bodyLengthMeters + tileScale * 2.0f,
                tileScale * 10.0f);
            auto sampleVisible = [&](const XMFLOAT3& sample, float radius) {
                if (!cameraTileOpen) return false;
                XMFLOAT3 toSample = Sub3(sample, playerPosition);
                float padded = maxRenderDistance + radius;
                if (Dot3(toSample, toSample) > padded * padded) return false;
                if (Dot3(toSample, viewForward) <= -radius * 2.0f) return false;
                Tile sampleTile = maze.TileFromWorld(sample.x, sample.z);
                return maze.IsOpen(sampleTile.x, sampleTile.y) && maze.LineClear(cameraTile, sampleTile);
            };

            // Walls already depth-test the pieces; the CPU cull only decides when every sampled body section is hidden.
            monsterTileVisible = sampleVisible(monsterPosition, renderRadius);
            monsterAnyPartVisible = monsterTileVisible;
            float sampleStep = std::max(tileScale * 0.82f, 0.72f);
            int sampleCount = std::clamp(static_cast<int>(std::ceil(bodyLengthMeters / sampleStep)) + 1, 3, 10);
            for (int i = 1; i < sampleCount && !monsterAnyPartVisible; ++i) {
                float t = static_cast<float>(i) / static_cast<float>(sampleCount - 1);
                monsterAnyPartVisible = sampleVisible(sampleTrail(bodyLengthMeters * t), renderRadius);
            }
            if (monsterAnyPartVisible) monsterPresentation_.renderVisibleUntil = timeRuntime_.time + 0.24f;
            monsterViewRelevant = monsterAnyPartVisible || timeRuntime_.time <= monsterPresentation_.renderVisibleUntil;
            monsterOccluded = !monsterAnyPartVisible;
            canTrackPlayer = monsterTileVisible && MonsterVisualEncounterPlayer();
        }
