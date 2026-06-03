    XMFLOAT3 MonsterTrailSample(float targetDistance) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (monsterPresentation_.trail.empty()) return world.monsterPosition;
        XMFLOAT3 prev = monsterPresentation_.trail.front();
        float travelled = 0.0f;
        for (size_t i = 1; i < monsterPresentation_.trail.size(); ++i) {
            XMFLOAT3 next = monsterPresentation_.trail[i];
            float dx = next.x - prev.x;
            float dz = next.z - prev.z;
            float len = std::sqrt(dx * dx + dz * dz);
            if (travelled + len >= targetDistance && len > 0.001f) {
                float t = (targetDistance - travelled) / len;
                return XMFLOAT3{Lerp(prev.x, next.x, t), 0.0f, Lerp(prev.z, next.z, t)};
            }
            travelled += len;
            prev = next;
        }
        float back = std::max(0.0f, targetDistance - travelled);
        return XMFLOAT3{prev.x - std::sin(world.monsterYaw) * back, 0.0f, prev.z - std::cos(world.monsterYaw) * back};
    }
