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
