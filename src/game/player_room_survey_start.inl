    void BeginRoomSurvey(Tile cur, bool pauseFirst) {
        if (!IsRoomSurveySpot(cur) || DreadPressure() > 0.42f || ChasePanicActive() || IsThreatVisible() ||
            (!pauseFirst && cameraRuntime_.roomSurveyCooldown > 0.0f)) {
            return;
        }

        cameraRuntime_.roomSurveyCenter = gameWorld_.player.bodyYaw;
        cameraRuntime_.roomSurveySpan = std::clamp(0.52f + static_cast<float>(gameWorld_.maze.LocalOpenCount(cur, 2)) * 0.030f, 0.62f, 1.18f);
        cameraRuntime_.roomSurveyDirection = RandRange(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
        cameraRuntime_.roomSurveyYawCount = 0;
        cameraRuntime_.roomSurveyPitchCount = 0;

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

        std::sort(candidates.begin(), candidates.end(), [](const SurveyCandidate& a, const SurveyCandidate& b) {
            return a.score > b.score;
        });
        int maxTargets = pauseFirst ? 5 : 3;
        int selected = std::min<int>(static_cast<int>(candidates.size()), maxTargets);
        if (selected > 0) {
            std::sort(candidates.begin(), candidates.begin() + selected, [this](const SurveyCandidate& a, const SurveyCandidate& b) {
                return AngleWrap(a.yaw - gameWorld_.player.bodyYaw) < AngleWrap(b.yaw - gameWorld_.player.bodyYaw);
            });
            if (RandRange(0.0f, 1.0f) < 0.5f) {
                std::reverse(candidates.begin(), candidates.begin() + selected);
            }
            for (int i = 0; i < selected; ++i) {
                cameraRuntime_.roomSurveyYaws[static_cast<size_t>(i)] = candidates[static_cast<size_t>(i)].yaw;
                cameraRuntime_.roomSurveyPitches[static_cast<size_t>(i)] = candidates[static_cast<size_t>(i)].pitch;
            }
            cameraRuntime_.roomSurveyYawCount = selected;
            cameraRuntime_.roomSurveyPitchCount = selected;
        }

        float targetBonus = static_cast<float>(std::max(0, cameraRuntime_.roomSurveyYawCount - 1)) * (pauseFirst ? 0.32f : 0.20f);
        cameraRuntime_.roomSurveyDuration = (pauseFirst ? RandRange(1.95f, 2.70f) : RandRange(1.18f, 1.85f)) + targetBonus;
        cameraRuntime_.roomSurveyTimer = cameraRuntime_.roomSurveyDuration;
        cameraRuntime_.roomSurveyCooldown = RandRange(4.5f, 9.5f);
        cameraRuntime_.branchLookTimer = 0.0f;
        cameraRuntime_.branchLookPaused = false;
        if (pauseFirst) {
            cameraRuntime_.stopTimer = std::max(cameraRuntime_.stopTimer, cameraRuntime_.roomSurveyDuration * RandRange(0.66f, 0.86f));
            cameraRuntime_.headScanTimer = 0.0f;
            cameraRuntime_.headScanDuration = 0.0f;
            cameraRuntime_.junctionScanActive = false;
            cameraRuntime_.lookBack = false;
            viewRuntime_.propLookTimer = 0.0f;
        }
    }

