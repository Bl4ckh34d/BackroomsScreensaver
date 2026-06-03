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
