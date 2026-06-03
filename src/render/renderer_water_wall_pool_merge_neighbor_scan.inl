            include(i);
            for (size_t j = i + 1; j < build.pendingWallWaterPools.size(); ++j) {
                if (used[j]) continue;
                const PendingWallWaterPool& pool = build.pendingWallWaterPools[j];
                if ((pool.side < 2 ? 0 : 1) != axis) continue;
                int tileDx = std::abs(pool.owner.x - first.owner.x);
                int tileDy = std::abs(pool.owner.y - first.owner.y);
                if (!(pool.owner == first.owner) && tileDx + tileDy > 1) continue;
                include(j);
            }
