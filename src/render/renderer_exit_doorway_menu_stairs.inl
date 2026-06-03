        if (sessionRuntime_.mode == RendererRuntimeMode::MainMenu) {
            constexpr int kStepCount = 9;
            float stepDepth = vestibuleLength / static_cast<float>(kStepCount);
            float totalRise = std::min(ctx.wallH * 0.76f, 2.18f);
            float stepRise = totalRise / static_cast<float>(kStepCount);
            for (int i = 0; i < kStepCount; ++i) {
                float d0 = static_cast<float>(i) * stepDepth;
                float d1 = static_cast<float>(i + 1) * stepDepth;
                float y0 = static_cast<float>(i) * stepRise;
                float y1 = static_cast<float>(i + 1) * stepRise;
                AddQuadUV(vertices, indices,
                    p(-vestibuleHalf, y0, d0), p(vestibuleHalf, y0, d0),
                    p(vestibuleHalf, y0, d1), p(-vestibuleHalf, y0, d1),
                    {0, 1, 0}, exitPortal.right,
                    FloorUv(p(-vestibuleHalf, y0, d0).x, p(-vestibuleHalf, y0, d0).z),
                    FloorUv(p(vestibuleHalf, y0, d0).x, p(vestibuleHalf, y0, d0).z),
                    FloorUv(p(vestibuleHalf, y0, d1).x, p(vestibuleHalf, y0, d1).z),
                    FloorUv(p(-vestibuleHalf, y0, d1).x, p(-vestibuleHalf, y0, d1).z),
                    1.0f);
                AddQuadUV(vertices, indices,
                    p(vestibuleHalf, y0, d1), p(-vestibuleHalf, y0, d1),
                    p(-vestibuleHalf, y1, d1), p(vestibuleHalf, y1, d1),
                    Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
                    {0, 0}, {1, 0},
                    {1, stepRise / settingsRuntime_.live.wallTextureMeters},
                    {0, stepRise / settingsRuntime_.live.wallTextureMeters},
                    0.0f);
            }
            auto addStairSide = [&](float side) {
                XMFLOAT3 normal = Scale3(exitPortal.right, -side);
                AddQuadUV(vertices, indices,
                    p(side * vestibuleHalf, 0.0f, vestibuleLength), p(side * vestibuleHalf, 0.0f, 0.0f),
                    p(side * vestibuleHalf, vestibuleH, 0.0f),
                    p(side * vestibuleHalf, vestibuleH + totalRise * 0.44f, vestibuleLength),
                    normal, Scale3(outward, -1.0f),
                    {0, 0}, {vestibuleLength / settingsRuntime_.live.wallTextureMeters, 0},
                    {vestibuleLength / settingsRuntime_.live.wallTextureMeters,
                        vestibuleH / settingsRuntime_.live.wallTextureMeters},
                    {0, (vestibuleH + totalRise) / settingsRuntime_.live.wallTextureMeters},
                    0.0f);
            };
            addStairSide(-1.0f);
            addStairSide(1.0f);
            XMFLOAT3 c0 = p(-vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength);
            XMFLOAT3 c1 = p(vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength);
            XMFLOAT3 c2 = p(vestibuleHalf, vestibuleH + 0.48f, stepDepth * 0.72f);
            XMFLOAT3 c3 = p(-vestibuleHalf, vestibuleH + 0.48f, stepDepth * 0.72f);
            AddQuadUV(vertices, indices,
                c0, c1, c2, c3,
                {0, -1, 0}, exitPortal.right,
                CeilingUv(c0.x, c0.z), CeilingUv(c1.x, c1.z), CeilingUv(c2.x, c2.z), CeilingUv(c3.x, c3.z),
                2.0f);
            AddQuadUV(vertices, indices,
                p(vestibuleHalf, totalRise, vestibuleLength), p(-vestibuleHalf, totalRise, vestibuleLength),
                p(-vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength),
                p(vestibuleHalf, vestibuleH + totalRise * 0.78f, vestibuleLength),
                Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
                {0, 0}, {1, 0}, {1, 1}, {0, 1}, 0.0f);
            return;
        }
