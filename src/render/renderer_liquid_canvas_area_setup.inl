        if (width <= 0.02f || depth <= 0.02f) return false;
        FloorFootprint area = MakeFloorFootprint(px, pz, width, depth, yaw, 0.0f);
        if (!std::isfinite(sourceX)) sourceX = px;
        if (!std::isfinite(sourceZ)) sourceZ = pz;
        Tile sourceTile = RenderMazeView().TileFromWorld(sourceX, sourceZ);
        float cYaw = std::cos(yaw);
        float sYaw = std::sin(yaw);
        XMFLOAT3 right{cYaw, 0.0f, -sYaw};
        XMFLOAT3 forward{sYaw, 0.0f, cYaw};
        float minX = std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = -std::numeric_limits<float>::max();
        const float xs[] = {-width * 0.5f, width * 0.5f};
        const float zs[] = {-depth * 0.5f, depth * 0.5f};
        for (float lx : xs) {
            for (float lz : zs) {
                XMFLOAT3 p = Add3({px, 0.0f, pz}, OrientedOffset(right, {0.0f, 1.0f, 0.0f}, forward, lx, 0.0f, lz));
                minX = std::min(minX, p.x);
                maxX = std::max(maxX, p.x);
                minZ = std::min(minZ, p.z);
                maxZ = std::max(maxZ, p.z);
            }
        }
