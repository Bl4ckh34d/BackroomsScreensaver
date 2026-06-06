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
        if (sessionRuntime_.mode != RendererRuntimeMode::MainMenu) {
            float exitDepthTile = exitPortal.dy != 0 ? ctx.tileD : ctx.tileW;
            int exitCellCount = std::max(1, static_cast<int>(std::floor(vestibuleLength / std::max(0.1f, exitDepthTile))));
            vestibuleLength = static_cast<float>(exitCellCount) * exitDepthTile;
        }
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
