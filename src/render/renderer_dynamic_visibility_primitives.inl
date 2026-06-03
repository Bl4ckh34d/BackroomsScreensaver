    bool DynamicVisualCandidate(const XMFLOAT3& pos, float radius, float maxDistance) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        float dx = pos.x - world.playerPosition.x;
        float dy = pos.y - world.playerPosition.y;
        float dz = pos.z - world.playerPosition.z;
        float padded = std::max(0.1f, maxDistance + radius);
        if (dx * dx + dy * dy + dz * dz > padded * padded) return false;
        XMFLOAT3 forward = DirectionFromYawPitch(world.playerYaw, world.playerPitch);
        float depth = dx * forward.x + dy * forward.y + dz * forward.z;
        return depth > -radius * 2.0f;
    }

    bool DynamicBillboardVisible(const XMFLOAT3& pos, float radius, float maxDistance, float minProjectedPixels) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 to = Sub3(pos, world.playerPosition);
        float distSq = Dot3(to, to);
        float padded = std::max(0.1f, maxDistance + radius);
        if (distSq > padded * padded) return false;
        XMFLOAT3 forward = DirectionFromYawPitch(world.playerYaw, world.playerPitch);
        float depth = Dot3(to, forward);
        if (depth <= -radius * 2.0f) return false;
        float projectedPixels = (radius / std::max(0.08f, std::abs(depth))) *
            static_cast<float>(std::max<LONG>(1, hostRuntime_.height)) * 0.72f;
        return projectedPixels >= minProjectedPixels;
    }
