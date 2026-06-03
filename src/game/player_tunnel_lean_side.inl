    float TunnelLeanSideFromApproach(Tile tile, const XMFLOAT3& intendedMoveDir, bool hasMoveDir) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        XMFLOAT3 center = RenderMazeView().WorldCenter(tile, world.playerPosition.y);
        XMFLOAT3 fromTile = {world.playerPosition.x - center.x, 0.0f, world.playerPosition.z - center.z};
        if (Length3(fromTile) < 0.04f && hasMoveDir) {
            fromTile = Scale3(intendedMoveDir, -1.0f);
        }

        XMFLOAT3 right{std::cos(world.playerYaw), 0.0f, -std::sin(world.playerYaw)};
        float localSide = Dot3(Normalize3(fromTile, {0.0f, 0.0f, 1.0f}), right);
        if (std::abs(localSide) > 0.035f) return localSide >= 0.0f ? -1.0f : 1.0f;

        if (std::abs(fromTile.x) > std::abs(fromTile.z)) return fromTile.x >= 0.0f ? -1.0f : 1.0f;
        if (std::abs(fromTile.z) > 0.001f) return fromTile.z >= 0.0f ? -1.0f : 1.0f;
        return gameWorld_.PlayerTunnelLeanSideTarget();
    }
