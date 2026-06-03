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
