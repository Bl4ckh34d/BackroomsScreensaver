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
