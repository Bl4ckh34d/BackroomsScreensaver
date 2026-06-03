// Render pass visibility, maze visibility, and chunk draw helpers.

        auto chunkVisible = [](const auto& chunk, XMFLOAT3 origin, XMFLOAT3 direction, float maxDistance, float coneCos) {
            float dx = chunk.center.x - origin.x;
            float dy = chunk.center.y - origin.y;
            float dz = chunk.center.z - origin.z;
            float padded = std::max(0.1f, maxDistance + chunk.radius);
            float distSq = dx * dx + dy * dy + dz * dz;
            if (distSq > padded * padded) return false;
            float depth = dx * direction.x + dy * direction.y + dz * direction.z;
            if (depth < -chunk.radius) return false;
            if (coneCos > -0.99f && distSq > 0.0001f) {
                float dist = std::sqrt(distSq);
                float radiusSlack = std::min(0.55f, chunk.radius / std::max(0.1f, dist));
                if (depth / dist < coneCos - radiusSlack) return false;
            }
            return true;
        };

        GameWorldRenderSnapshot visibilityWorld = gameWorld_.BuildRenderSnapshot();
        const Maze* visibilityMaze = visibilityWorld.maze;
        std::vector<uint8_t> visibleMazeTiles;
        auto buildMazeVisibility = [&](XMFLOAT3 origin,
                                       XMFLOAT3 direction,
                                       float maxDistance,
                                       float coneCos,
                                       int wallHaloTiles) {
            if (!visibilityMaze || visibilityMaze->w <= 0 || visibilityMaze->h <= 0) return false;
            const Maze& maze = *visibilityMaze;
            Tile originTile = maze.TileFromWorld(origin.x, origin.z);
            if (!maze.IsOpen(originTile.x, originTile.y)) return false;
            visibleMazeTiles.assign(static_cast<size_t>(maze.w * maze.h), 0);
            auto mark = [&](int x, int y) {
                if (!maze.InBounds(x, y)) return;
                visibleMazeTiles[static_cast<size_t>(y * maze.w + x)] = 1;
            };
            auto markWithHalo = [&](int x, int y) {
                mark(x, y);
                for (int yy = -wallHaloTiles; yy <= wallHaloTiles; ++yy) {
                    for (int xx = -wallHaloTiles; xx <= wallHaloTiles; ++xx) {
                        if (std::abs(xx) + std::abs(yy) > wallHaloTiles) continue;
                        mark(x + xx, y + yy);
                    }
                }
            };

            int tileRadius = std::clamp(static_cast<int>(std::ceil(maxDistance / std::max(0.1f, maze.TileMinimum()))) + wallHaloTiles,
                1, std::max(maze.w, maze.h));
            int minX = std::max(0, originTile.x - tileRadius);
            int maxX = std::min(maze.w - 1, originTile.x + tileRadius);
            int minY = std::max(0, originTile.y - tileRadius);
            int maxY = std::min(maze.h - 1, originTile.y + tileRadius);
            float maxDistanceSq = maxDistance * maxDistance;
            float tilePad = std::max(maze.TileAverage() * 0.85f, 0.1f);
            float tilePadSq = tilePad * tilePad;
            for (int y = minY; y <= maxY; ++y) {
                for (int x = minX; x <= maxX; ++x) {
                    if (!maze.IsOpen(x, y)) continue;
                    XMFLOAT3 center = maze.WorldCenter({x, y}, origin.y);
                    float dx = center.x - origin.x;
                    float dz = center.z - origin.z;
                    float distSq = dx * dx + dz * dz;
                    if (distSq > maxDistanceSq + tilePadSq) continue;
                    if (coneCos > -0.99f && distSq > 0.0001f) {
                        float invDist = 1.0f / std::sqrt(distSq);
                        float facing = (dx * direction.x + dz * direction.z) * invDist;
                        float slack = std::min(0.42f, tilePad * invDist);
                        if (facing < coneCos - slack) continue;
                    }
                    if (!maze.LineClear(originTile, {x, y})) continue;
                    markWithHalo(x, y);
                }
            }
            markWithHalo(originTile.x, originTile.y);
            return true;
        };

        auto chunkMazeVisible = [&](const auto& chunk, int forceTileRadius) {
            if (visibleMazeTiles.empty() || !visibilityMaze) return true;
            const Maze& maze = *visibilityMaze;
            int minX = std::max(0, chunk.minTileX - forceTileRadius);
            int maxX = std::min(maze.w - 1, chunk.maxTileX + forceTileRadius);
            int minY = std::max(0, chunk.minTileY - forceTileRadius);
            int maxY = std::min(maze.h - 1, chunk.maxTileY + forceTileRadius);
            for (int y = minY; y <= maxY; ++y) {
                const size_t row = static_cast<size_t>(y * maze.w);
                for (int x = minX; x <= maxX; ++x) {
                    if (visibleMazeTiles[row + static_cast<size_t>(x)] != 0) return true;
                }
            }
            return false;
        };

        auto drawVisibleChunks = [&](const std::vector<StaticIndexChunk>& chunks,
                                     XMFLOAT3 origin,
                                     XMFLOAT3 direction,
                                     float maxDistance,
                                     float coneCos,
                                     float forceVisibleDistance = 0.0f,
                                     bool useMazeVisibility = false,
                                     int forceTileRadius = 1) {
            UINT drawn = 0;
            for (const StaticIndexChunk& chunk : chunks) {
                bool forceByDistance = false;
                if (forceVisibleDistance > 0.0f) {
                    float dx = chunk.center.x - origin.x;
                    float dy = chunk.center.y - origin.y;
                    float dz = chunk.center.z - origin.z;
                    float force = forceVisibleDistance + chunk.radius;
                    if (dx * dx + dy * dy + dz * dz <= force * force) {
                        forceByDistance = true;
                    }
                }
                if (!forceByDistance && !chunkVisible(chunk, origin, direction, maxDistance, coneCos)) continue;
                if (useMazeVisibility && !chunkMazeVisible(chunk, forceTileRadius)) continue;
                d3dRuntime_.context->DrawIndexed(chunk.indexCount, chunk.startIndex, 0);
                drawn += chunk.indexCount;
            }
            return drawn;
        };

        auto drawVisibleInstancedChunks = [&](const std::vector<StaticInstanceChunk>& chunks,
                                              XMFLOAT3 origin,
                                              XMFLOAT3 direction,
                                              float maxDistance,
                                              float coneCos,
                                              float forceVisibleDistance = 0.0f,
                                              bool useMazeVisibility = false,
                                              int forceTileRadius = 1) {
            UINT drawn = 0;
            for (const StaticInstanceChunk& chunk : chunks) {
                bool forceByDistance = false;
                if (forceVisibleDistance > 0.0f) {
                    float dx = chunk.center.x - origin.x;
                    float dy = chunk.center.y - origin.y;
                    float dz = chunk.center.z - origin.z;
                    float force = forceVisibleDistance + chunk.radius;
                    if (dx * dx + dy * dy + dz * dz <= force * force) {
                        forceByDistance = true;
                    }
                }
                if (!forceByDistance && !chunkVisible(chunk, origin, direction, maxDistance, coneCos)) continue;
                if (useMazeVisibility && !chunkMazeVisible(chunk, forceTileRadius)) continue;
                d3dRuntime_.context->DrawIndexedInstanced(chunk.indexCount, chunk.instanceCount, chunk.startIndex, chunk.baseVertex, chunk.startInstance);
                drawn += chunk.indexCount * chunk.instanceCount;
            }
            return drawn;
        };

        float mainHalfFov = fovDegrees * 0.5f * kPi / 180.0f;
        float mainHorizontalHalfFov = std::atan(std::tan(mainHalfFov) * std::max(0.1f, aspect));
        float mainConeCos = std::cos(std::min(kPi * 0.98f, std::max(mainHalfFov, mainHorizontalHalfFov) + 0.82f));
        float mainCullDistance = monsterPreview_.active
            ? 1000.0f
            : std::max(viewFarMeters, settingsRuntime_.live.fogEndMeters + (visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 12.0f);
        float mainForceVisibleDistance = std::max((visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 5.0f, 8.0f);
        float transparentCullDistance = monsterPreview_.active
            ? mainCullDistance
            : std::min(mainCullDistance, std::max((visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 8.0f, settingsRuntime_.live.fogEndMeters + (visibilityMaze ? visibilityMaze->TileAverage() : 0.1f) * 4.0f));
        if (!monsterPreview_.active) {
            buildMazeVisibility(eyePos, viewDirFloat, mainCullDistance, mainConeCos, 4);
        }

