    bool EffectivePlayerCrouch() const {
        return sessionRuntime_.input.crouch || gameWorld_.PlayerTunnelCrouchLocked();
    }

    bool IsTunnelTile(Tile t) const {
        return !RenderMazeView().IsOpen(t.x, t.y) && RenderMazeView().WallFeature(t.x, t.y) == MazeWallFeature::Tunnel;
    }

    bool ProbeTunnelAhead(const XMFLOAT3& primaryDir, bool hasPrimaryDir, Tile& tunnelTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || RenderMazeView().w <= 0 || RenderMazeView().h <= 0) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        std::array<XMFLOAT3, 2> dirs{};
        int dirCount = 0;
        if (hasPrimaryDir && Length3(primaryDir) > 0.001f) {
            dirs[static_cast<size_t>(dirCount++)] = Normalize3(primaryDir, Forward());
        }
        dirs[static_cast<size_t>(dirCount++)] = Forward();

        float tile = RenderMazeView().TileMinimum();
        const float distances[] = {tile * 0.18f, tile * 0.34f, tile * 0.52f, tile * 0.74f, tile * 0.96f};
        for (int d = 0; d < dirCount; ++d) {
            for (float dist : distances) {
                Tile t = RenderMazeView().TileFromWorld(world.playerPosition.x + dirs[static_cast<size_t>(d)].x * dist,
                    world.playerPosition.z + dirs[static_cast<size_t>(d)].z * dist);
                if (IsTunnelTile(t)) {
                    tunnelTile = t;
                    return true;
                }
            }
        }
        return false;
    }

    bool TunnelImmediateVicinity(Tile& tunnelTile) const {
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame || RenderMazeView().w <= 0 || RenderMazeView().h <= 0) return false;
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        Tile cur = CameraTile();
        float ox = -static_cast<float>(RenderMazeView().w) * RenderMazeView().tileW * 0.5f;
        float oz = -static_cast<float>(RenderMazeView().h) * RenderMazeView().tileD * 0.5f;
        float margin = std::clamp(RenderMazeView().TileMinimum() * 0.22f, 0.34f, 0.58f);
        float bestDistSq = margin * margin;
        bool found = false;

        for (int y = cur.y - 1; y <= cur.y + 1; ++y) {
            for (int x = cur.x - 1; x <= cur.x + 1; ++x) {
                Tile t{x, y};
                if (!IsTunnelTile(t)) continue;
                float x0 = ox + static_cast<float>(x) * RenderMazeView().tileW;
                float x1 = x0 + RenderMazeView().tileW;
                float z0 = oz + static_cast<float>(y) * RenderMazeView().tileD;
                float z1 = z0 + RenderMazeView().tileD;
                float dx = std::max(std::max(x0 - world.playerPosition.x, 0.0f), world.playerPosition.x - x1);
                float dz = std::max(std::max(z0 - world.playerPosition.z, 0.0f), world.playerPosition.z - z1);
                float distSq = dx * dx + dz * dz;
                if (distSq <= bestDistSq) {
                    bestDistSq = distSq;
                    tunnelTile = t;
                    found = true;
                }
            }
        }
        return found;
    }

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

    void UpdateTunnelLeanSideTarget(Tile tile, const XMFLOAT3& intendedMoveDir, bool hasMoveDir) {
        gameWorld_.SetPlayerTunnelLeanSideTarget(TunnelLeanSideFromApproach(tile, intendedMoveDir, hasMoveDir));
    }

    void UpdateTunnelCrouchLock(float dt, const XMFLOAT3& intendedMoveDir, bool hasMoveDir) {
        dt = std::max(0.0f, dt);
        if (sessionRuntime_.mode != RendererRuntimeMode::PlayableGame) {
            gameWorld_.ResetPlayerTunnelPosture();
            return;
        }

        gameWorld_.AdvancePlayerTunnelPostureTimer(dt);
        Tile tile = CameraTile();
        bool inTunnel = IsTunnelTile(tile);
        Tile approachTile{};
        bool approachingTunnel = sessionRuntime_.input.crouch && ProbeTunnelAhead(intendedMoveDir, hasMoveDir, approachTile);
        Tile vicinityTile{};
        bool nearTunnel = TunnelImmediateVicinity(vicinityTile);
        bool tunnelMouthLock = nearTunnel && (sessionRuntime_.input.crouch || gameWorld_.PlayerTunnelCrouchLocked());

        if (inTunnel) {
            gameWorld_.HoldPlayerTunnelCrouch(0.58f);
        } else if (tunnelMouthLock) {
            gameWorld_.SetPlayerTunnelCrouchLocked(true);
            UpdateTunnelLeanSideTarget(vicinityTile, intendedMoveDir, hasMoveDir);
        } else if (approachingTunnel) {
            UpdateTunnelLeanSideTarget(approachTile, intendedMoveDir, hasMoveDir);
            if (gameWorld_.PlayerTunnelPostureHoldTimer() <= 0.0f && !sessionRuntime_.input.crouch) gameWorld_.SetPlayerTunnelCrouchLocked(false);
        } else if (!sessionRuntime_.input.crouch) {
            gameWorld_.SetPlayerTunnelCrouchLocked(false);
        }

        gameWorld_.SetPlayerTunnelLeanTarget(inTunnel || tunnelMouthLock || approachingTunnel || gameWorld_.PlayerTunnelPostureHoldTimer() > 0.0f);
    }

    void UpdateTunnelCameraLean(float dt) {
        gameWorld_.AdvancePlayerTunnelCameraLean(dt);
    }

    #include "player_collision_service.inl"
