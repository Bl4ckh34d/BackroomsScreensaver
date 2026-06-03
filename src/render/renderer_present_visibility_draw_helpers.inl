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
