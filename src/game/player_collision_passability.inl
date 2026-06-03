    bool PlayerCollisionTilePassable(Tile t) const {
        if (RenderMazeView().IsOpen(t.x, t.y)) return true;
        return sessionRuntime_.mode == RendererRuntimeMode::PlayableGame &&
            EffectivePlayerCrouch() &&
            IsTunnelTile(t);
    }

    bool PlayerCollisionFootprintOpen(float x, float z) const {
        Tile t = RenderMazeView().TileFromWorld(x, z);
        if (!PlayerCollisionTilePassable(t)) return false;

        float ox = -static_cast<float>(RenderMazeView().w) * RenderMazeView().tileW * 0.5f;
        float oz = -static_cast<float>(RenderMazeView().h) * RenderMazeView().tileD * 0.5f;
        float radius = std::clamp(RenderMazeView().TileMinimum() * 0.26f, 0.12f, 0.46f);
        float radiusSq = radius * radius;

        for (int dz = -1; dz <= 1; ++dz) {
            for (int dx = -1; dx <= 1; ++dx) {
                Tile check{t.x + dx, t.y + dz};
                if (PlayerCollisionTilePassable(check)) continue;

                float minX = ox + static_cast<float>(check.x) * RenderMazeView().tileW;
                float minZ = oz + static_cast<float>(check.y) * RenderMazeView().tileD;
                float maxX = minX + RenderMazeView().tileW;
                float maxZ = minZ + RenderMazeView().tileD;
                float nearestX = std::clamp(x, minX, maxX);
                float nearestZ = std::clamp(z, minZ, maxZ);
                float overlapX = x - nearestX;
                float overlapZ = z - nearestZ;
                if (overlapX * overlapX + overlapZ * overlapZ < radiusSq) return false;
            }
        }
        return true;
    }

