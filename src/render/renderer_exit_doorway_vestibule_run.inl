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

        const float exitDepthTile = exitPortal.dy != 0 ? ctx.tileD : ctx.tileW;
        const int lampCount = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? std::clamp(static_cast<int>(std::floor(vestibuleLength / std::max(tileAvg * 1.75f, 2.2f))), 2, 7)
            : std::max(1, static_cast<int>(std::floor(vestibuleLength / std::max(0.1f, exitDepthTile))));
        const float firstDepth = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? std::min(tileAvg * 0.70f, vestibuleLength * 0.20f)
            : exitDepthTile * 0.5f;
        const float lastDepth = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
            ? std::max(firstDepth, vestibuleLength - tileAvg * 0.85f)
            : firstDepth + static_cast<float>(std::max(0, lampCount - 1)) * exitDepthTile;
        for (int i = 0; i < lampCount; ++i) {
            float t = lampCount <= 1 ? 0.0f : static_cast<float>(i) / static_cast<float>(lampCount - 1);
            float depth = Lerp(firstDepth, lastDepth, t);
            XMFLOAT3 exitLampCenter = p(0.0f, 0.0f, depth);
            Tile lampTile{
                exitPortal.tile.x + exitPortal.dx * (i + 1),
                exitPortal.tile.y + exitPortal.dy * (i + 1)
            };
            float exitLampSeed = sessionRuntime_.mode == RendererRuntimeMode::MainMenu
                ? std::fmod(LampSeed(exitPortal.tile.x + i * 3, exitPortal.tile.y + i * 5) * 0.73f + 0.19f, 0.49f)
                : LampSeed(lampTile.x, lampTile.y) * 0.49f;
            AddCeilingCard(vertices, indices, {exitLampCenter.x, 0.0f, exitLampCenter.z},
                ctx.tileW * (0.94f / 3.0f), ctx.tileD * (0.94f / 3.0f), 0.0f, vestibuleH - 0.004f, 3.0f + exitLampSeed);
            effectRuntime_.runtimeLamps.push_back({
                lampTile,
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
