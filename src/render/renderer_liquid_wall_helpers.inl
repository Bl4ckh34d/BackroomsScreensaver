// Liquid placement wall helpers.

    static LiquidWallLeakFrame BuildLiquidWallLeakFrame(const MazeSurfaceBuildContext& surface,
                                                        const XMFLOAT3& tileCenter,
                                                        int side,
                                                        float lateral,
                                                        float centerY,
                                                        float width,
                                                        float height,
                                                        float wallInset,
                                                        float seed) {
        LiquidWallLeakFrame frame{};
        frame.center = {tileCenter.x, centerY, tileCenter.z};
        if (side == 0) {
            frame.normal = {0.0f, 0.0f, 1.0f};
            frame.right = {1.0f, 0.0f, 0.0f};
            frame.inward = {0.0f, 0.0f, 1.0f};
            frame.center = {tileCenter.x + lateral, centerY, tileCenter.z - surface.tileD * 0.5f + wallInset};
        } else if (side == 1) {
            frame.normal = {0.0f, 0.0f, -1.0f};
            frame.right = {-1.0f, 0.0f, 0.0f};
            frame.inward = {0.0f, 0.0f, -1.0f};
            frame.center = {tileCenter.x + lateral, centerY, tileCenter.z + surface.tileD * 0.5f - wallInset};
        } else if (side == 2) {
            frame.normal = {1.0f, 0.0f, 0.0f};
            frame.right = {0.0f, 0.0f, 1.0f};
            frame.inward = {1.0f, 0.0f, 0.0f};
            frame.center = {tileCenter.x - surface.tileW * 0.5f + wallInset, centerY, tileCenter.z + lateral};
        } else {
            frame.normal = {-1.0f, 0.0f, 0.0f};
            frame.right = {0.0f, 0.0f, -1.0f};
            frame.inward = {-1.0f, 0.0f, 0.0f};
            frame.center = {tileCenter.x + surface.tileW * 0.5f - wallInset, centerY, tileCenter.z + lateral};
        }
        frame.center = Add3(frame.center, Scale3(frame.normal,
            0.0008f + std::fmod(std::abs(seed) * 9.713f, 1.0f) * 0.0018f));

        XMFLOAT3 up{0.0f, 1.0f, 0.0f};
        frame.a = Add3(frame.center, Add3(Scale3(frame.right, -width * 0.5f), Scale3(up, -height * 0.5f)));
        frame.b = Add3(frame.center, Add3(Scale3(frame.right,  width * 0.5f), Scale3(up, -height * 0.5f)));
        frame.c = Add3(frame.center, Add3(Scale3(frame.right,  width * 0.5f), Scale3(up,  height * 0.5f)));
        frame.d = Add3(frame.center, Add3(Scale3(frame.right, -width * 0.5f), Scale3(up,  height * 0.5f)));
        return frame;
    }

    static LiquidTileTouchInfo BuildLiquidTileTouchInfo(const MazeSurfaceBuildContext& surface,
                                                        const XMFLOAT3& tileCenter,
                                                        float px,
                                                        float pz,
                                                        float width,
                                                        float depth,
                                                        float yaw) {
        LiquidTileTouchInfo info{};
        info.halfX = std::abs(std::cos(yaw)) * width * 0.5f + std::abs(std::sin(yaw)) * depth * 0.5f;
        info.halfZ = std::abs(std::sin(yaw)) * width * 0.5f + std::abs(std::cos(yaw)) * depth * 0.5f;
        float xMin = tileCenter.x - surface.tileW * 0.5f;
        float xMax = tileCenter.x + surface.tileW * 0.5f;
        float zMin = tileCenter.z - surface.tileD * 0.5f;
        float zMax = tileCenter.z + surface.tileD * 0.5f;
        info.touches[0] = pz - info.halfZ <= zMin + surface.tileD * 0.10f;
        info.touches[1] = pz + info.halfZ >= zMax - surface.tileD * 0.10f;
        info.touches[2] = px - info.halfX <= xMin + surface.tileW * 0.10f;
        info.touches[3] = px + info.halfX >= xMax - surface.tileW * 0.10f;
        return info;
    }

    bool AddBloodWallLiquidCard(WaterSurfaceBuildContext& waterBuild,
                                LiquidCanvasBuildContext& liquidBuild,
                                Tile tile,
                                int side,
                                float lateral,
                                float yCenter,
                                float width,
                                float height,
                                float seed,
                                bool waterLiquid) {
        XMFLOAT3 tileCenter = RenderMazeView().WorldCenter(tile, 0.0f);
        float minAlong = 0.0f;
        float maxAlong = 0.0f;
        if (!WallWaterSupportSpan(waterBuild, tile, side, minAlong, maxAlong)) return false;

        constexpr float kBloodWallDecalInset = 0.0050f;
        constexpr float wallDecalMargin = 0.13f;
        minAlong += wallDecalMargin;
        maxAlong -= wallDecalMargin;
        float available = maxAlong - minAlong;
        if (available < 0.28f) return false;

        width = std::min(width, available);
        float halfWidth = width * 0.5f;
        float desiredAlong = (side == 0 || side == 1) ? tileCenter.x + lateral : tileCenter.z + lateral;
        float clampedAlong = std::clamp(desiredAlong, minAlong + halfWidth, maxAlong - halfWidth);
        float clampedLateral = (side == 0 || side == 1) ? clampedAlong - tileCenter.x : clampedAlong - tileCenter.z;

        constexpr float wallBloodFloorMargin = 0.002f;
        constexpr float wallBloodCeilingMargin = 0.004f;
        height = liquidBuild.wallH - wallBloodFloorMargin - wallBloodCeilingMargin;
        float centerY = wallBloodFloorMargin + height * 0.5f;

        LiquidWallLeakFrame frame = BuildLiquidWallLeakFrame(waterBuild.surface, tileCenter, side,
            clampedLateral, centerY, width, height, kBloodWallDecalInset, seed);
        AddQuadUV(liquidBuild.vertices, liquidBuild.liquidIndices,
            frame.a, frame.b, frame.c, frame.d, frame.normal, frame.right,
            {0, 1}, {1, 1}, {1, 0}, {0, 0},
            LiquidSurfaceMaterial(waterLiquid, 0.11f + std::fmod(seed, 0.83f)));

        float bottomY = frame.center.y - height * 0.5f;
        if (bottomY > wallBloodFloorMargin + 0.045f) {
            int dripStrips = 1 + static_cast<int>(LampHash(seed * 41.0f + frame.center.x, frame.center.z - seed * 13.0f) * 3.0f);
            for (int strip = 0; strip < dripStrips; ++strip) {
                float r0 = LampHash(seed * 53.0f + static_cast<float>(strip) * 7.1f, frame.center.x + frame.center.z);
                float r1 = LampHash(seed * 67.0f + static_cast<float>(strip) * 11.3f, frame.center.z - frame.center.x);
                float stripW = std::min(width * (0.10f + r0 * 0.18f), 0.34f);
                float offset = (r1 - 0.5f) * std::max(0.0f, width - stripW) * 0.62f;
                float bridgeTop = std::min(bottomY + 0.12f + r0 * 0.10f, liquidBuild.wallH - wallBloodCeilingMargin);
                float bridgeBottom = wallBloodFloorMargin;
                float bridgeH = bridgeTop - bridgeBottom;
                if (bridgeH <= 0.035f) continue;
                XMFLOAT3 up{0.0f, 1.0f, 0.0f};
                XMFLOAT3 bridgeCenter = Add3(frame.center, Add3(Scale3(frame.right, offset),
                    {0.0f, (bridgeTop + bridgeBottom) * 0.5f - frame.center.y, 0.0f}));
                XMFLOAT3 ba = Add3(bridgeCenter, Add3(Scale3(frame.right, -stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                XMFLOAT3 bb = Add3(bridgeCenter, Add3(Scale3(frame.right,  stripW * 0.5f), Scale3(up, -bridgeH * 0.5f)));
                XMFLOAT3 bc = Add3(bridgeCenter, Add3(Scale3(frame.right,  stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                XMFLOAT3 bd = Add3(bridgeCenter, Add3(Scale3(frame.right, -stripW * 0.5f), Scale3(up,  bridgeH * 0.5f)));
                AddQuadUV(liquidBuild.vertices, liquidBuild.liquidIndices, ba, bb, bc, bd,
                    frame.normal, frame.right, {0, 1}, {1, 1}, {1, 0}, {0, 0},
                    LiquidSurfaceMaterial(waterLiquid,
                        0.37f + std::fmod(seed + r0 * 0.61f + static_cast<float>(strip) * 0.17f, 0.51f)));
            }
        }

        BloodScarePoint scare{};
        scare.pos = frame.center;
        float sourceY = frame.center.y + height * 0.40f;
        float topY = frame.center.y + height * 0.5f;
        if (topY > liquidBuild.wallH * 0.78f) {
            sourceY = std::max(sourceY, topY - 0.035f);
        }
        scare.source = {frame.center.x, std::clamp(sourceY, 0.18f, liquidBuild.wallH - 0.055f), frame.center.z};
        scare.normal = frame.normal;
        scare.radius = std::clamp(1.25f + std::max(width, height) * 0.72f, 1.75f, 4.35f);
        scare.focusDelaySeconds = 0.30f + LampHash(seed * 43.0f + frame.center.x, frame.center.z) * 0.64f;
        scare.requireFacing = true;
        scare.waterLiquid = waterLiquid;
        if (waterLiquid) {
            scare.dreadScale = 0.30f;
        }
        scareRuntime_.bloodScarePoints.push_back(scare);
        return true;
    }
