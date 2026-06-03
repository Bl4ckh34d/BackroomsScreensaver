        struct SurveyCandidate {
            float score;
            float yaw;
            float pitch;
        };
        std::vector<SurveyCandidate> candidates;
        candidates.reserve(8);
        Tile previous = HasPreviousMovementTile(cur) ? cameraRuntime_.previousTile : Tile{-1000, -1000};
        Tile nextTarget = cameraRuntime_.pathIndex < cameraRuntime_.path.size() ? cameraRuntime_.path[cameraRuntime_.pathIndex] : cur;
        const Tile dirs[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        for (Tile d : dirs) {
            Tile n{cur.x + d.x, cur.y + d.y};
            if (!gameWorld_.maze.IsOpen(n.x, n.y)) continue;
            XMFLOAT3 nw = gameWorld_.maze.WorldCenter(n, gameWorld_.player.position.y);
            float branchYaw = std::atan2(nw.x - gameWorld_.player.position.x, nw.z - gameWorld_.player.position.z);
            float ray = ViewRayOpenDistance(branchYaw, std::clamp(settingsRuntime_.live.fogEndMeters, 6.0f, 18.0f));
            if (ray < gameWorld_.maze.TileMinimum() * 0.92f) continue;
            float rel = std::abs(AngleWrap(branchYaw - gameWorld_.player.bodyYaw));
            float score = ray * 0.80f + static_cast<float>(gameWorld_.maze.LocalOpenCount(n, 2)) * 0.16f +
                SmoothStep(0.18f, 1.55f, rel) * 1.15f;
            if (n == nextTarget) score -= 1.25f;
            if (n == previous) score -= 0.70f;
            candidates.push_back({score + RandRange(-0.25f, 0.25f), branchYaw, -0.040f});
        }

        XMFLOAT3 propFocus{};
        if (FindRoomPropFocus(cur, propFocus) && RandRange(0.0f, 1.0f) < 0.72f) {
            candidates.push_back({7.0f + RandRange(-0.35f, 0.35f), YawToPoint(propFocus),
                std::clamp(PitchToPoint(propFocus), -0.32f, 0.20f)});
            viewRuntime_.lastPropLookTarget = propFocus;
            viewRuntime_.hasLastPropLookTarget = true;
        }
