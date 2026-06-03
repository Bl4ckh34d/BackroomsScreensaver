            auto include = [&](size_t index) {
                const PendingWallWaterPool& pool = build.pendingWallWaterPools[index];
                used[index] = 1;
                ++count;
                seedSum += pool.seed;
                if (pool.score > bestScore) {
                    bestScore = pool.score;
                    bestSide = pool.side;
                }
                float cYaw = std::cos(pool.yaw);
                float sYaw = std::sin(pool.yaw);
                XMFLOAT3 right{cYaw, 0.0f, -sYaw};
                XMFLOAT3 forward{sYaw, 0.0f, cYaw};
                std::array<XMFLOAT3, 4> corners{
                    Add3({pool.cx, 0.0f, pool.cz}, Add3(Scale3(right, -pool.width * 0.5f), Scale3(forward,  pool.depth * 0.5f))),
                    Add3({pool.cx, 0.0f, pool.cz}, Add3(Scale3(right,  pool.width * 0.5f), Scale3(forward,  pool.depth * 0.5f))),
                    Add3({pool.cx, 0.0f, pool.cz}, Add3(Scale3(right,  pool.width * 0.5f), Scale3(forward, -pool.depth * 0.5f))),
                    Add3({pool.cx, 0.0f, pool.cz}, Add3(Scale3(right, -pool.width * 0.5f), Scale3(forward, -pool.depth * 0.5f)))
                };
                for (const XMFLOAT3& corner : corners) {
                    minX = std::min(minX, corner.x);
                    maxX = std::max(maxX, corner.x);
                    minZ = std::min(minZ, corner.z);
                    maxZ = std::max(maxZ, corner.z);
                }
            };
