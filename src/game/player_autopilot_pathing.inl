    void ChoosePath(bool force = false) {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        const Maze& maze = *world.maze;
        Tile cur = CameraTile();
        if (!maze.IsOpen(cur.x, cur.y)) {
            RecoverPlayerCollisionFootprint();
            return;
        }
        bool monsterActive = MonsterActiveForCurrentMode();
        Tile monsterTile = monsterActive ? MonsterTile() : Tile{-10000, -10000};
        if (!force && cameraRuntime_.pathIndex < cameraRuntime_.path.size()) {
            bool panicState = monsterActive && ChasePanicActive();
            bool freeRunPath = panicState && OpenAreaAllowsFreeRun(cur);
            bool panicContext = monsterActive && (IsThreatVisible() || panicState);
            if (ActivePathValidForMode(cur, freeRunPath) &&
                (!panicContext || !ActiveThreatPathShouldRepath(cur, monsterTile))) {
                return;
            }
            cameraRuntime_.path.clear();
            cameraRuntime_.pathIndex = 0;
        }
        if (VisibleInFront(maze.exit)) viewRuntime_.exitSpotted = true;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;

        float monsterDist = std::numeric_limits<float>::max();
        if (monsterActive) {
            float mdx = world.monsterPosition.x - world.playerPosition.x;
            float mdz = world.monsterPosition.z - world.playerPosition.z;
            monsterDist = std::sqrt(mdx * mdx + mdz * mdz);
        }
        bool threat = monsterActive && ((monsterDist < 12.0f && maze.LineClear(cur, monsterTile)) || ChasePanicActive());
        std::vector<Tile> localNeighbors = maze.Neighbors(cur);
        bool hasNonBacktrackingNeighbor = false;
        for (Tile n : localNeighbors) {
            if (!IsBacktrackingStep(cur, n)) {
                hasNonBacktrackingNeighbor = true;
                break;
            }
        }
        auto startsByBacktracking = [&](const std::vector<Tile>& p) {
            return hasNonBacktrackingNeighbor && p.size() > 1 && IsBacktrackingStep(cur, p[1]);
        };

        if (threat) {
            if (viewRuntime_.exitSpotted && ExitRouteNotBlockedByMonster()) {
                auto exitPath = maze.Path(cur, maze.exit);
                if (!exitPath.empty() && !ThreatPathMovesTowardMonster(exitPath, monsterTile)) {
                    cameraRuntime_.path = std::move(exitPath);
                    cameraRuntime_.pathIndex = cameraRuntime_.path.size() > 1 ? 1 : 0;
                    return;
                }
            }

            cameraRuntime_.path = BuildThreatEscapePath(cur, monsterTile);
            if (cameraRuntime_.path.empty()) {
                ForceImmediateFleeStep(cur, monsterTile);
                return;
            }
        } else if (VisibleInFront(maze.exit)) {
            viewRuntime_.exitSpotted = true;
            cameraRuntime_.path = maze.Path(cur, maze.exit);
        } else if (BuildCorridorContinuationPath(cur)) {
            return;
        } else {
            float bestScore = -1.0f;
            for (int n = 0; n < 80; ++n) {
                int x = 1 + static_cast<int>(sessionRuntime_.rng() % (maze.w - 2));
                int y = 1 + static_cast<int>(sessionRuntime_.rng() % (maze.h - 2));
                if (!maze.IsOpen(x, y)) continue;
                Tile t{x, y};
                float dx = static_cast<float>(t.x - cur.x);
                float dy = static_cast<float>(t.y - cur.y);
                float forwardBias = 0.0f;
                XMFLOAT3 tw = maze.WorldCenter(t, world.playerPosition.y);
                XMFLOAT3 f = NavigationForward(cur);
                float wx = tw.x - world.playerPosition.x;
                float wz = tw.z - world.playerPosition.z;
                float wl = std::sqrt(wx * wx + wz * wz);
                if (wl > 0.01f) forwardBias = (wx * f.x + wz * f.z) / wl;
                float score = dx * dx + dy * dy + forwardBias * 18.0f;
                auto p = maze.Path(cur, t);
                if (!p.empty()) {
                    if (startsByBacktracking(p)) continue;
                    score += PathExplorationScore(p, false);
                    if (VisitCount(t) == 0) score += 110.0f;
                    else score -= static_cast<float>(std::min<int>(VisitCount(t), 8)) * 42.0f;
                    if (p.size() > 1) {
                        if (VisitCount(p[1]) == 0) score += 92.0f;
                        else score -= static_cast<float>(std::min<int>(VisitCount(p[1]), 6)) * 34.0f;
                        if (IsBacktrackingStep(cur, p[1])) score -= hasNonBacktrackingNeighbor ? 520.0f : 120.0f;
                    }
                    if (score > bestScore) {
                        cameraRuntime_.path = std::move(p);
                        bestScore = score;
                    }
                }
            }
        }
        if (cameraRuntime_.path.empty()) {
            const std::vector<Tile>& neighbors = localNeighbors;
            if (!neighbors.empty()) {
                Tile best = neighbors.front();
                float bestScore = -1.0e9f;
                for (Tile n : neighbors) {
                    if (hasNonBacktrackingNeighbor && IsBacktrackingStep(cur, n)) continue;
                    float score = (VisitCount(n) == 0 ? 120.0f : -static_cast<float>(std::min<int>(VisitCount(n), 8)) * 35.0f);
                    score += static_cast<float>(maze.LocalOpenCount(n, 1)) * 3.0f;
                    score += (TileDistanceSq(cur, maze.exit) - TileDistanceSq(n, maze.exit)) * 1.1f;
                    if (IsBacktrackingStep(cur, n) && hasNonBacktrackingNeighbor) score -= 420.0f;
                    score += RandRange(-1.0f, 1.0f);
                    if (score > bestScore) {
                        bestScore = score;
                        best = n;
                    }
                }
                cameraRuntime_.path.push_back(cur);
                cameraRuntime_.path.push_back(best);
            }
        }
        cameraRuntime_.pathIndex = cameraRuntime_.path.size() > 1 ? 1 : 0;
    }

    void BeginChaseLookBack(float urgency) {
        XMFLOAT3 focus = MonsterFocusPoint();
        viewRuntime_.chaseLookBackYaw = YawToPoint(focus);
        viewRuntime_.chaseLookBackPitch = std::clamp(PitchToPoint(focus), -0.34f, 0.24f);
        viewRuntime_.chaseLookBackDuration = RandRange(0.56f, 0.88f) * (1.0f - urgency * 0.10f);
        viewRuntime_.chaseLookBackTimer = viewRuntime_.chaseLookBackDuration;
        viewRuntime_.chaseLookBackCooldown = std::max(0.85f, RandRange(1.35f, 3.15f) * (1.0f - urgency * 0.24f));
        viewRuntime_.secondsSinceLookBack = 0.0f;
        viewRuntime_.stumbleTimer = std::min(viewRuntime_.stumbleTimer, 0.08f);
    }

    float ChaseLookBackWeight() const {
        if (viewRuntime_.chaseLookBackTimer <= 0.0f || viewRuntime_.chaseLookBackDuration <= 0.001f) return 0.0f;
        float t = 1.0f - viewRuntime_.chaseLookBackTimer / viewRuntime_.chaseLookBackDuration;
        return SmoothStep(0.0f, 0.22f, t) * (1.0f - SmoothStep(0.70f, 1.0f, t));
    }

    void UpdateChaseLookBackTarget(float dt) {
        if (viewRuntime_.chaseLookBackTimer <= 0.0f || viewRuntime_.chaseLookBackDuration <= 0.001f) return;
        if (!MonsterCanSeePlayer()) return;
        XMFLOAT3 focus = MonsterFocusPoint();
        float liveYaw = YawToPoint(focus);
        float livePitch = std::clamp(PitchToPoint(focus), -0.34f, 0.24f);
        float follow = std::min(1.0f, dt * 2.35f);
        viewRuntime_.chaseLookBackYaw += AngleWrap(liveYaw - viewRuntime_.chaseLookBackYaw) * follow;
        viewRuntime_.chaseLookBackPitch += (livePitch - viewRuntime_.chaseLookBackPitch) * follow;
    }


