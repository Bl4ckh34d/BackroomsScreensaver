    float ViewRayOpenDistance(float yaw, float maxMeters) const {
        XMFLOAT3 dir{std::sin(yaw), 0.0f, std::cos(yaw)};
        float step = std::clamp(gameWorld_.maze.TileMinimum() * 0.16f, 0.10f, 0.26f);
        float lastOpen = 0.0f;
        for (float d = step; d <= maxMeters; d += step) {
            float x = gameWorld_.player.position.x + dir.x * d;
            float z = gameWorld_.player.position.z + dir.z * d;
            if (!PlayerCollisionFootprintOpen(x, z)) break;
            lastOpen = d;
        }
        return lastOpen;
    }
