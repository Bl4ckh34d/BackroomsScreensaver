// Water surface placement mark floor wall pools.

    void MarkWaterTile(WaterSurfaceBuildContext& build,
                       Tile t,
                       bool ceiling,
                       int side,
                       int mode,
                       float seed,
                       float score,
                       bool suppressCard = false) {
        if (!RenderMazeView().IsOpen(t.x, t.y) ||
            (!gEffectDebugViewer && (t == RenderMazeView().start || t == RenderMazeView().exit))) {
            return;
        }
        WaterTileSurface& surface = (ceiling ? build.ceilingWaterTiles : build.floorWaterTiles)[WaterTileIndex(t)];
        side = std::clamp(side, 0, 3);
        mode = std::clamp(mode, 0, 3);
        if (ceiling) {
            MarkWetCeilingTile(t);
        } else {
            MarkWetFootstepTile(t);
        }
        if (!surface.active) {
            surface.active = true;
            surface.suppressCard = suppressCard;
            surface.side = side;
            surface.mode = mode;
            surface.seed = seed;
            surface.score = score;
            return;
        }
        surface.suppressCard = surface.suppressCard && suppressCard;
        surface.mode = MergeWaterMode(surface.mode, mode);
        if (score >= surface.score) {
            surface.side = side;
            surface.seed = seed;
            surface.score = score;
        }
    }

    bool EmitFloorWaterPoolCard(WaterSurfaceBuildContext& build,
                                Tile owner,
                                float cx,
                                float cz,
                                int side,
                                float seed,
                                float width,
                                float depth,
                                float yaw,
                                float uvModeBase,
                                float score) {
        if (!RenderMazeView().IsOpen(owner.x, owner.y) ||
            (!gEffectDebugViewer && (owner == RenderMazeView().start || owner == RenderMazeView().exit))) {
            return false;
        }
        float w = width;
        float d = depth;
        for (int attempt = 0; attempt < 4; ++attempt) {
            if (FootprintFitsMaze(cx, cz, w, d, yaw, 0.020f, build.tileMin)) {
                float cYaw = std::cos(yaw);
                float sYaw = std::sin(yaw);
                XMFLOAT3 right{cYaw, 0.0f, -sYaw};
                XMFLOAT3 forward{sYaw, 0.0f, cYaw};
                XMFLOAT3 center{cx, build.floorLift, cz};
                XMFLOAT3 a = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(forward,  d * 0.5f)));
                XMFLOAT3 b = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(forward,  d * 0.5f)));
                XMFLOAT3 c0 = Add3(center, Add3(Scale3(right,  w * 0.5f), Scale3(forward, -d * 0.5f)));
                XMFLOAT3 d0 = Add3(center, Add3(Scale3(right, -w * 0.5f), Scale3(forward, -d * 0.5f)));
                AddQuadUV(build.vertices, build.waterIndices, a, b, c0, d0, {0, 1, 0}, right,
                    {0, uvModeBase}, {1, uvModeBase}, {1, uvModeBase + 1.0f}, {0, uvModeBase + 1.0f},
                    WaterDecalMaterial(seed, 0.0f, 0.014f));
                MarkWaterTile(build, owner, false, side, 0, seed, score, true);
                MarkWetFootstepArea(cx, cz, w, d, yaw);
                return true;
            }
            w *= 0.86f;
            d *= 0.86f;
        }
        return false;
    }

    void QueueWallWaterPoolCard(WaterSurfaceBuildContext& build,
                                Tile owner,
                                float cx,
                                float cz,
                                int side,
                                float seed,
                                float width,
                                float depth,
                                float yaw,
                                float score) {
        if (!RenderMazeView().IsOpen(owner.x, owner.y) ||
            (!gEffectDebugViewer && (owner == RenderMazeView().start || owner == RenderMazeView().exit))) {
            return;
        }
        build.pendingWallWaterPools.push_back({owner, side, cx, cz, width, depth, yaw, seed, score});
    }

    void EmitMergedWallWaterPools(WaterSurfaceBuildContext& build) {
        std::vector<uint8_t> used(build.pendingWallWaterPools.size(), 0);
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
            if (count <= 1) {
                EmitFloorWaterPoolCard(build, first.owner, first.cx, first.cz, first.side, first.seed,
                    first.width, first.depth, first.yaw, 5.0f, first.score);
                continue;
            }
            float finalCx = (minX + maxX) * 0.5f;
            float finalCz = (minZ + maxZ) * 0.5f;
            float finalW = std::max(0.05f, maxX - minX);
            float finalD = std::max(0.05f, maxZ - minZ);
            if (!FootprintFitsMaze(finalCx, finalCz, finalW, finalD, 0.0f, 0.020f, build.tileMin)) {
                float marginX = build.surface.tileW * 0.012f;
                float marginZ = build.surface.tileD * 0.012f;
                float l = build.surface.ox + static_cast<float>(first.owner.x) * build.surface.tileW + marginX;
                float r = l + build.surface.tileW - marginX * 2.0f;
                float z0 = build.surface.oz + static_cast<float>(first.owner.y) * build.surface.tileD + marginZ;
                float z1 = z0 + build.surface.tileD - marginZ * 2.0f;
                minX = std::clamp(minX, l, r);
                maxX = std::clamp(maxX, l, r);
                minZ = std::clamp(minZ, z0, z1);
                maxZ = std::clamp(maxZ, z0, z1);
                finalCx = (minX + maxX) * 0.5f;
                finalCz = (minZ + maxZ) * 0.5f;
                finalW = std::max(0.05f, maxX - minX);
                finalD = std::max(0.05f, maxZ - minZ);
            }
            EmitFloorWaterPoolCard(build, first.owner, finalCx, finalCz, bestSide,
                seedSum / static_cast<float>(std::max(1, count)), finalW, finalD,
                0.0f, 5.0f, std::max(1.18f, bestScore));
        }
    }
