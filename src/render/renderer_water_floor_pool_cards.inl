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
