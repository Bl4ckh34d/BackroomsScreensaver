            auto considerWall = [&](int dx, int dz, XMFLOAT3 normal, float planeCoord, float distanceToPlane) {
                if (!allowBodySurfaceClimb) return;
                if (maze.IsOpen(tile.x + dx, tile.y + dz)) return;
                float wallBand = 0.18f + 0.66f * (0.5f + 0.5f * std::sin(timeRuntime_.time * 0.17f + fi * 0.31f + dx * 1.7f + dz * 2.3f));
                float y = std::clamp(settingsRuntime_.live.wallHeightMeters * wallBand,
                    radius * 0.92f + 0.026f,
                    settingsRuntime_.live.wallHeightMeters - radius * 0.92f - 0.026f);
                XMFLOAT3 c = p;
                c.y = y;
                if (dx != 0) {
                    c.x = planeCoord + normal.x * (radius * 0.98f + 0.026f);
                } else {
                    c.z = planeCoord + normal.z * (radius * 0.98f + 0.026f);
                }
                float nearWall = Clamp01(1.0f - distanceToPlane / std::max(0.1f, maze.TileMinimum() * 0.44f));
                float wallHunger = 0.30f + climbAffinity * 0.82f + nearWall * 0.55f;
                float score = distanceToPlane / std::max(0.1f, maze.TileMinimum()) - wallHunger;
                if (score < best.score) best = {normal, c, score};
            };
            considerWall(1, 0, {-1.0f, 0.0f, 0.0f}, tileCenter.x + halfW, halfW - localX);
            considerWall(-1, 0, {1.0f, 0.0f, 0.0f}, tileCenter.x - halfW, halfW + localX);
            considerWall(0, 1, {0.0f, 0.0f, -1.0f}, tileCenter.z + halfD, halfD - localZ);
            considerWall(0, -1, {0.0f, 0.0f, 1.0f}, tileCenter.z - halfD, halfD + localZ);
