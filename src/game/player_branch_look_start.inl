    bool BeginBranchLook(Tile cur, Tile previous, Tile nextTarget, bool allowPause = false, bool allowRoomTile = false) {
        if (!gameWorld_.maze.IsOpen(cur.x, cur.y) || (!allowRoomTile && IsRoomLike(cur)) || DreadPressure() > 0.36f ||
            ChasePanicActive() || IsThreatVisible() || cameraRuntime_.branchLookTimer > 0.0f || cameraRuntime_.roomSurveyTimer > 0.0f ||
            cameraRuntime_.branchLookCooldown > 0.0f || cur == cameraRuntime_.lastBranchLookTile) {
            return false;
        }

        const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        Tile monsterTile = MonsterTile();
        bool monsterActive = MonsterActiveForCurrentMode();
        float bestScore = -1.0e9f;
        float bestYaw = gameWorld_.player.yaw;
        int candidates = 0;
        for (Tile d : dirs) {
            Tile n{cur.x + d.x, cur.y + d.y};
            if (!gameWorld_.maze.IsOpen(n.x, n.y)) continue;
            if (n == previous || n == nextTarget) continue;
            XMFLOAT3 nw = gameWorld_.maze.WorldCenter(n, gameWorld_.player.position.y);
            float branchYaw = std::atan2(nw.x - gameWorld_.player.position.x, nw.z - gameWorld_.player.position.z);
            float ray = ViewRayOpenDistance(branchYaw, std::clamp(settingsRuntime_.live.fogEndMeters, 6.0f, 18.0f));
            if (ray < gameWorld_.maze.TileMinimum() * 0.95f) continue;
            bool possibleThreatLine = monsterActive && MonsterDistance() < 18.0f && gameWorld_.maze.LineClear(n, monsterTile);
            float score = ray + static_cast<float>(gameWorld_.maze.LocalOpenCount(n, 1)) * 0.18f +
                (possibleThreatLine ? 5.5f : 0.0f) - std::abs(AngleWrap(branchYaw - gameWorld_.player.yaw)) * 0.35f;
            ++candidates;
            if (score > bestScore) {
                bestScore = score;
                bestYaw = branchYaw;
            }
        }
        if (candidates <= 0) return false;
        cameraRuntime_.branchLookYaw = bestYaw;
        cameraRuntime_.branchLookPitch = -0.045f + RandRange(-0.012f, 0.018f);
        float yawDelta = std::abs(AngleWrap(cameraRuntime_.branchLookYaw - gameWorld_.player.yaw));
        cameraRuntime_.branchLookDuration = RandRange(1.34f, 2.02f) + SmoothStep(0.35f, 1.55f, yawDelta) * 0.50f;
        cameraRuntime_.branchLookTimer = cameraRuntime_.branchLookDuration;
        cameraRuntime_.branchLookCooldown = RandRange(2.2f, 5.4f);
        cameraRuntime_.lastBranchLookTile = cur;
        cameraRuntime_.branchLookPaused = allowPause && RandRange(0.0f, 1.0f) < 0.72f;
        if (cameraRuntime_.branchLookPaused) {
            cameraRuntime_.stopTimer = std::max(cameraRuntime_.stopTimer, cameraRuntime_.branchLookDuration * RandRange(0.42f, 0.66f));
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.junctionScanActive = false;
            cameraRuntime_.lookBack = false;
            viewRuntime_.propLookTimer = 0.0f;
        }
        return true;
    }
