    bool MonsterMoveSegmentOpen(const XMFLOAT3& from, const XMFLOAT3& to) const {
        float dx = to.x - from.x;
        float dz = to.z - from.z;
        float len = std::sqrt(dx * dx + dz * dz);
        int steps = std::max(2, static_cast<int>(std::ceil(len / (gameWorld_.maze.TileMinimum() * 0.055f))));
        for (int i = 0; i <= steps; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            XMFLOAT3 p{from.x + dx * t, 0.0f, from.z + dz * t};
            if (!MonsterFootprintOpen(p)) return false;
        }
        return true;
    }
