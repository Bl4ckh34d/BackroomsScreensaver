    void AddExitDoorwayWallRun(std::vector<Vertex>& vertices,
                               std::vector<uint32_t>& indices,
                               const MazeSurfaceBuildContext& ctx,
                               const ExitPortal& exitPortal,
                               float tileAvg) {
        if (!exitPortal.valid) return;
        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        constexpr float kExitDoorHalfW = 0.60f;
        constexpr float kExitDoorHalfH = 1.05f;
        constexpr float kExitDoorCenterY = 1.05f;
        constexpr float kExitFramePostHalfW = 0.055f;
        constexpr float kExitFrameTopHalfH = 0.070f;
        constexpr float kExitDoorwayHalfW = kExitDoorHalfW + kExitFramePostHalfW * 2.0f;
        constexpr float kExitDoorwayTop = kExitDoorCenterY + kExitDoorHalfH + kExitFrameTopHalfH * 2.0f;
        float openingHalf = std::min(exitPortal.halfSpan * 0.86f, kExitDoorwayHalfW);
        float doorwayTop = std::min(ctx.wallH - 0.04f, kExitDoorwayTop);
        float vestibuleLength = std::min(
            std::max(tileAvg * 7.5f, settingsRuntime_.live.fogEndMeters + tileAvg * 3.0f),
            tileAvg * 14.0f);
        float vestibuleHalf = openingHalf;
        float vestibuleH = ctx.wallH;

        auto wallPoint = [&](float along, float y, float push = 0.0f) {
            return Add3(exitPortal.wallCenter,
                Add3(Scale3(exitPortal.right, along), Add3({0.0f, y, 0.0f}, Scale3(exitPortal.inward, push))));
        };

        auto addWallPatch = [&](float a, float b, float y0, float y1) {
            if (b <= a || y1 <= y0) return;
            AddQuadUV(vertices, indices,
                wallPoint(a, y0), wallPoint(b, y0), wallPoint(b, y1), wallPoint(a, y1),
                exitPortal.inward, exitPortal.right,
                {a / settingsRuntime_.live.wallTextureMeters, y0 / settingsRuntime_.live.wallTextureMeters},
                {b / settingsRuntime_.live.wallTextureMeters, y0 / settingsRuntime_.live.wallTextureMeters},
                {b / settingsRuntime_.live.wallTextureMeters, y1 / settingsRuntime_.live.wallTextureMeters},
                {a / settingsRuntime_.live.wallTextureMeters, y1 / settingsRuntime_.live.wallTextureMeters},
                0.0f);
        };

        addWallPatch(-exitPortal.halfSpan, -openingHalf, 0.0f, ctx.wallH);
        addWallPatch(openingHalf, exitPortal.halfSpan, 0.0f, ctx.wallH);
        addWallPatch(-openingHalf, openingHalf, doorwayTop, ctx.wallH);

        XMFLOAT3 outward = Scale3(exitPortal.inward, -1.0f);
        float portalFloorBackset = sessionRuntime_.mode == RendererRuntimeMode::MainMenu ? 0.0f : 0.05f;
        XMFLOAT3 nearCenter = Add3(exitPortal.wallCenter, Scale3(outward, portalFloorBackset));
        auto p = [&](float along, float y, float depth) {
            return Add3(nearCenter, Add3(Scale3(exitPortal.right, along), Add3(Scale3(up, y), Scale3(outward, depth))));
        };

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

        XMFLOAT3 vf0 = p(-vestibuleHalf, 0.0f, 0.0f);
        XMFLOAT3 vf1 = p(vestibuleHalf, 0.0f, 0.0f);
        XMFLOAT3 vf2 = p(vestibuleHalf, 0.0f, vestibuleLength);
        XMFLOAT3 vf3 = p(-vestibuleHalf, 0.0f, vestibuleLength);
        AddQuadUV(vertices, indices,
            vf0, vf1, vf2, vf3,
            {0, 1, 0}, exitPortal.right,
            FloorUv(vf0.x, vf0.z), FloorUv(vf1.x, vf1.z), FloorUv(vf2.x, vf2.z), FloorUv(vf3.x, vf3.z),
            1.0f);

        XMFLOAT3 vc0 = p(-vestibuleHalf, vestibuleH, vestibuleLength);
        XMFLOAT3 vc1 = p(vestibuleHalf, vestibuleH, vestibuleLength);
        XMFLOAT3 vc2 = p(vestibuleHalf, vestibuleH, 0.0f);
        XMFLOAT3 vc3 = p(-vestibuleHalf, vestibuleH, 0.0f);
        AddQuadUV(vertices, indices,
            vc0, vc1, vc2, vc3,
            {0, -1, 0}, exitPortal.right,
            CeilingUv(vc0.x, vc0.z), CeilingUv(vc1.x, vc1.z), CeilingUv(vc2.x, vc2.z), CeilingUv(vc3.x, vc3.z),
            2.0f);

        const float lampSpacing = std::max(tileAvg * 1.75f, 2.2f);
        const int lampCount = std::clamp(static_cast<int>(std::floor(vestibuleLength / lampSpacing)), 2, 7);
        const float firstDepth = std::min(tileAvg * 0.70f, vestibuleLength * 0.20f);
        const float lastDepth = std::max(firstDepth, vestibuleLength - tileAvg * 0.85f);
        for (int i = 0; i < lampCount; ++i) {
            float t = lampCount <= 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(lampCount - 1);
            float depth = Lerp(firstDepth, lastDepth, t);
            XMFLOAT3 exitLampCenter = p(0.0f, 0.0f, depth);
            float exitLampSeed = std::fmod(LampSeed(exitPortal.tile.x + i * 3, exitPortal.tile.y + i * 5) * 0.73f + 0.19f, 0.49f);
            AddCeilingCard(vertices, indices, {exitLampCenter.x, 0.0f, exitLampCenter.z},
                ctx.tileW * (0.94f / 3.0f), ctx.tileD * (0.94f / 3.0f), 0.0f, vestibuleH - 0.004f, 3.0f + exitLampSeed);
            effectRuntime_.runtimeLamps.push_back({
                exitPortal.tile,
                {exitLampCenter.x, vestibuleH - 0.08f, exitLampCenter.z},
                0.0f,
                RandRange(0.08f, 0.72f),
                false,
                1,
                false,
                0.0f
            });
        }

        auto addVestibuleSide = [&](float side) {
            XMFLOAT3 normal = Scale3(exitPortal.right, -side);
            AddQuadUV(vertices, indices,
                p(side * vestibuleHalf, 0.0f, vestibuleLength), p(side * vestibuleHalf, 0.0f, 0.0f),
                p(side * vestibuleHalf, vestibuleH, 0.0f), p(side * vestibuleHalf, vestibuleH, vestibuleLength),
                normal, Scale3(outward, -1.0f),
                {0, 0}, {vestibuleLength / settingsRuntime_.live.wallTextureMeters, 0},
                {vestibuleLength / settingsRuntime_.live.wallTextureMeters, vestibuleH / settingsRuntime_.live.wallTextureMeters},
                {0, vestibuleH / settingsRuntime_.live.wallTextureMeters},
                0.0f);
        };
        addVestibuleSide(-1.0f);
        addVestibuleSide(1.0f);
        AddQuadUV(vertices, indices,
            p(vestibuleHalf, 0.0f, vestibuleLength), p(-vestibuleHalf, 0.0f, vestibuleLength),
            p(-vestibuleHalf, vestibuleH, vestibuleLength), p(vestibuleHalf, vestibuleH, vestibuleLength),
            Scale3(outward, -1.0f), Scale3(exitPortal.right, -1.0f),
            {0, 0}, {1, 0}, {1, 1}, {0, 1}, 10.0f);
    }
