// Player camera attention corridor scan.

    bool StraightCorridorTravelYaw(Tile cameraTile, float& corridorYaw) const {
        corridorYaw = gameWorld_.player.yaw;
        if (!IsStraightCorridor(cameraTile) || cameraRuntime_.pathIndex >= cameraRuntime_.path.size()) return false;

        size_t startIndex = cameraRuntime_.pathIndex;
        Tile next = cameraRuntime_.path[startIndex];
        if (next == cameraTile) {
            if (startIndex + 1 >= cameraRuntime_.path.size()) return false;
            ++startIndex;
            next = cameraRuntime_.path[startIndex];
        }
        if (!AdjacentTiles(cameraTile, next)) return false;

        int dirX = next.x - cameraTile.x;
        int dirY = next.y - cameraTile.y;
        Tile previous = cameraTile;
        int straightSteps = 0;
        const size_t limit = std::min(cameraRuntime_.path.size(), startIndex + 3);
        for (size_t i = startIndex; i < limit; ++i) {
            Tile step = cameraRuntime_.path[i];
            if (!AdjacentTiles(previous, step)) return false;
            int stepX = step.x - previous.x;
            int stepY = step.y - previous.y;
            if (stepX != dirX || stepY != dirY) return false;
            previous = step;
            ++straightSteps;
        }
        if (straightSteps < 2) return false;

        XMFLOAT3 a = gameWorld_.maze.WorldCenter(cameraTile, gameWorld_.player.position.y);
        XMFLOAT3 b = gameWorld_.maze.WorldCenter(next, gameWorld_.player.position.y);
        corridorYaw = std::atan2(b.x - a.x, b.z - a.z);
        return true;
    }

    bool FindUpcomingCorridorTurn(Tile cameraTile, float& turnYaw, float& turnWeight) const {
        turnYaw = gameWorld_.player.yaw;
        turnWeight = 0.0f;
        if (!IsCorridorLike(cameraTile) || cameraRuntime_.pathIndex >= cameraRuntime_.path.size() || cameraRuntime_.path.size() < 2) return false;

        const float tile = std::max(gameWorld_.maze.TileMinimum(), 0.1f);
        const size_t lastCandidate = std::min(cameraRuntime_.path.size() - 2, cameraRuntime_.pathIndex + 4);
        Tile previous = cameraTile;
        for (size_t i = cameraRuntime_.pathIndex; i <= lastCandidate; ++i) {
            Tile turn = cameraRuntime_.path[i];
            if (turn == previous) continue;
            if (!AdjacentTiles(previous, turn)) break;

            Tile next = cameraRuntime_.path[i + 1];
            if (!AdjacentTiles(turn, next)) break;

            int inX = turn.x - previous.x;
            int inY = turn.y - previous.y;
            int outX = next.x - turn.x;
            int outY = next.y - turn.y;
            if (inX == outX && inY == outY) {
                previous = turn;
                continue;
            }

            if (outX == -inX && outY == -inY) return false;

            XMFLOAT3 turnCenter = gameWorld_.maze.WorldCenter(turn, gameWorld_.player.position.y);
            float dx = turnCenter.x - gameWorld_.player.position.x;
            float dz = turnCenter.z - gameWorld_.player.position.z;
            float distToTurn = std::sqrt(dx * dx + dz * dz);
            float weight = SmoothStep(tile * 1.42f, tile * 0.22f, distToTurn);
            float futureFade = 1.0f - std::min(static_cast<float>(i - cameraRuntime_.pathIndex), 3.0f) * 0.16f;
            turnWeight = weight * futureFade * 0.46f;
            if (turnWeight <= 0.001f) return false;

            XMFLOAT3 nextCenter = gameWorld_.maze.WorldCenter(next, gameWorld_.player.position.y);
            turnYaw = std::atan2(nextCenter.x - turnCenter.x, nextCenter.z - turnCenter.z);
            return true;
        }
        return false;
    }

    bool BeginJunctionScan(Tile cur, Tile previous) {
        if (!IsTightTJunction(cur, previous)) return false;

        std::vector<std::pair<float, float>> branches;
        const int dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        Tile monsterTile = MonsterTile();
        bool monsterActive = MonsterActiveForCurrentMode();
        for (auto& d : dirs) {
            Tile n{cur.x + d[0], cur.y + d[1]};
            if (!gameWorld_.maze.IsOpen(n.x, n.y)) continue;
            if (n == previous) continue;
            XMFLOAT3 nw = gameWorld_.maze.WorldCenter(n, gameWorld_.player.position.y);
            float branchYaw = std::atan2(nw.x - gameWorld_.player.position.x, nw.z - gameWorld_.player.position.z);
            float rel = AngleWrap(branchYaw - gameWorld_.player.bodyYaw);
            bool branchThreat = monsterActive && MonsterDistance() < 18.0f && gameWorld_.maze.LineClear(n, monsterTile);
            float order = branchThreat ? -10.0f + std::abs(rel) * 0.01f : rel;
            branches.push_back({order, branchYaw});
        }

        if (branches.size() < 2) return false;
        std::sort(branches.begin(), branches.end(), [](const auto& a, const auto& b) {
            return a.first < b.first;
        });

        cameraRuntime_.junctionScanCount = std::min<int>(static_cast<int>(branches.size()), static_cast<int>(cameraRuntime_.junctionScanYaws.size()));
        for (int i = 0; i < cameraRuntime_.junctionScanCount; ++i) {
            cameraRuntime_.junctionScanYaws[static_cast<size_t>(i)] = branches[static_cast<size_t>(i)].second;
        }

        cameraRuntime_.junctionScanActive = true;
        cameraRuntime_.junctionScanTile = cur;
        float baseSeconds = std::max(0.72f, settingsRuntime_.live.junctionScanBaseSeconds);
        float branchSeconds = std::max(0.96f, settingsRuntime_.live.junctionScanBranchSeconds);
        cameraRuntime_.stopTimer = baseSeconds +
            static_cast<float>(cameraRuntime_.junctionScanCount) * RandRange(branchSeconds * 0.92f, branchSeconds * 1.16f);
        cameraRuntime_.headScanDuration = cameraRuntime_.stopTimer;
        cameraRuntime_.headScanTimer = cameraRuntime_.stopTimer;
        cameraRuntime_.headScanCenter = gameWorld_.player.bodyYaw;
        cameraRuntime_.lookBack = false;
        cameraRuntime_.path.clear();
        cameraRuntime_.pathIndex = 0;
        return true;
    }
