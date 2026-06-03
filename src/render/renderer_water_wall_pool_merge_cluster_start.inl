        for (size_t i = 0; i < build.pendingWallWaterPools.size(); ++i) {
            if (used[i]) continue;
            const PendingWallWaterPool& first = build.pendingWallWaterPools[i];
            int axis = first.side < 2 ? 0 : 1;
            float minX = std::numeric_limits<float>::max();
            float maxX = -std::numeric_limits<float>::max();
            float minZ = std::numeric_limits<float>::max();
            float maxZ = -std::numeric_limits<float>::max();
            float seedSum = 0.0f;
            float bestScore = -1.0f;
            int bestSide = first.side;
            int count = 0;
