        XMFLOAT3 blobSurfaceUp{0.0f, 1.0f, 0.0f};
        for (int i = 0; i < bodyCount; ++i) {
            float fi = static_cast<float>(i);
            float taper = 1.0f - fi / static_cast<float>(bodyCount);
            XMFLOAT3 side = bodySides[static_cast<size_t>(i)];
            XMFLOAT3 tangent = bodyTangents[static_cast<size_t>(i)];
            float radius = bodyRadii[static_cast<size_t>(i)];
            struct SurfaceChoice {
                XMFLOAT3 normal;
                XMFLOAT3 center;
                float score;
            };
            XMFLOAT3 p = bodyPoints[static_cast<size_t>(i)];
            Tile tile = maze.TileFromWorld(p.x, p.z);
            XMFLOAT3 tileCenter = maze.WorldCenter(tile, 0.0f);
            float halfW = maze.tileW * 0.5f;
            float halfD = maze.tileD * 0.5f;
            float localX = p.x - tileCenter.x;
            float localZ = p.z - tileCenter.z;
            float surfaceNoiseA = std::sin(timeRuntime_.time * 0.19f + fi * 0.71f + monsterPosition.x * 0.07f);
            float surfaceNoiseB = std::sin(timeRuntime_.time * 0.13f - fi * 0.53f + monsterPosition.z * 0.09f);
            float surfaceNoiseC = std::sin(timeRuntime_.time * 0.073f + fi * 1.37f + static_cast<float>(sessionRuntime_.runtimeSeed & 255) * 0.017f);
            float climbAffinity = SmoothStep(0.10f, 1.0f, monsterPresentation_.headChaseBlend) * 0.10f +
                SmoothStep(0.0f, 1.0f, Clamp01(world.monsterRoamBurstTimer / 1.05f)) * 0.07f +
                std::abs(surfaceNoiseC) * 0.08f;
            SurfaceChoice best{{0.0f, 1.0f, 0.0f}, Add3(p, {0.0f, radius * 1.08f + 0.050f, 0.0f}),
                0.44f - surfaceNoiseA * 0.10f};
            if (allowBodySurfaceClimb) {
                SurfaceChoice ceiling{{0.0f, -1.0f, 0.0f}, {p.x, settingsRuntime_.live.wallHeightMeters - radius * 0.92f - 0.026f, p.z},
                    0.38f - surfaceNoiseB * 0.24f - climbAffinity};
                if (ceiling.score < best.score) best = ceiling;
            }
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
            auto centerForSurface = [&](XMFLOAT3 normal) {
                normal = Normalize3(normal, up);
                XMFLOAT3 c = p;
                float surfaceGap = radius * 0.98f + 0.026f;
                if (normal.y > 0.72f) {
                    c.y = radius * 1.08f + 0.050f;
                } else if (normal.y < -0.72f) {
                    c.y = settingsRuntime_.live.wallHeightMeters - radius * 0.92f - 0.026f;
                } else {
                    c.y = std::clamp(c.y, radius * 0.92f + 0.026f,
                        settingsRuntime_.live.wallHeightMeters - radius * 0.92f - 0.026f);
                    if (std::abs(normal.x) > std::abs(normal.z)) {
                        c.x = tileCenter.x + (normal.x < 0.0f ? halfW : -halfW) + normal.x * surfaceGap;
                        c.z = std::clamp(c.z, tileCenter.z - halfD, tileCenter.z + halfD);
                    } else {
                        c.z = tileCenter.z + (normal.z < 0.0f ? halfD : -halfD) + normal.z * surfaceGap;
                        c.x = std::clamp(c.x, tileCenter.x - halfW, tileCenter.x + halfW);
                    }
                }
                return c;
            };
            XMFLOAT3 surfaceUp = Normalize3(best.normal, up);
            if (allowBodySurfaceClimb && i == 0 && hasHandSupportUp) {
                surfaceUp = handSupportUp;
                best.center = Add3(bodyCenterlinePoints[0], Scale3(surfaceUp, radius * 0.98f + 0.026f));
                best.center.y = std::clamp(best.center.y, radius * 0.74f + 0.026f,
                    settingsRuntime_.live.wallHeightMeters - radius * 0.74f - 0.026f);
            }
            if (i > 0) {
                XMFLOAT3 prevRawUp = Normalize3(bodyUps[static_cast<size_t>(i - 1)], up);
                bool directFloorCeilingFlip = std::abs(prevRawUp.y) > 0.70f &&
                    std::abs(surfaceUp.y) > 0.70f &&
                    Dot3(prevRawUp, surfaceUp) < -0.35f;
                if (directFloorCeilingFlip) {
                    surfaceUp = prevRawUp;
                    best.normal = prevRawUp;
                    best.center = centerForSurface(prevRawUp);
                }
            }
            if (i == 0) {
                blobSurfaceUp = surfaceUp;
            } else {
                surfaceUp = blobSurfaceUp;
                best.center = centerForSurface(surfaceUp);
            }
            if (i > 0) {
                XMFLOAT3 prevUp = bodyUps[static_cast<size_t>(i - 1)];
                float continuity = 0.72f;
                surfaceUp = Normalize3(Lerp3(surfaceUp, prevUp, continuity), prevUp);
                if (Dot3(surfaceUp, prevUp) < 0.0f) surfaceUp = Scale3(surfaceUp, -1.0f);
                surfaceUp = Normalize3(Sub3(surfaceUp, Scale3(tangent, Dot3(surfaceUp, tangent))), prevUp);
            }
            p = best.center;
            float gait = timeRuntime_.time * 3.65f - fi * 0.74f + monsterPosition.x * 0.09f + monsterPosition.z * 0.05f;
            float stepLift = std::pow(Clamp01(std::sin(gait) * 0.5f + 0.5f), 2.3f);
            float gutPulse = std::sin(timeRuntime_.time * 2.65f - fi * 0.58f) * 0.065f;
            float bodyBounce = std::sin(timeRuntime_.time * 3.75f - fi * 0.92f) * 0.026f * (0.55f + taper * 0.45f);
            float curiosityLift = curiosityPose * std::pow(taper, 2.4f) * (0.46f * modelY);
            p = Add3(p, Scale3(surfaceUp, 0.045f * modelY + stepLift * 0.018f + bodyBounce * 0.30f + gutPulse * 0.18f + curiosityLift * 0.36f));
            bodyUps[static_cast<size_t>(i)] = surfaceUp;
            bodySides[static_cast<size_t>(i)] = Normalize3(Cross3(surfaceUp, tangent), side);
            bodyPoints[static_cast<size_t>(i)] = p;
        }
