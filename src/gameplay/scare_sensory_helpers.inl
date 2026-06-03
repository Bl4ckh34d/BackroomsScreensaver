// Scare/effect player sensory helper functions. 
// Included inside Renderer's private section from scare_effect_events.inl.

    bool PlayerLooksAt(const XMFLOAT3& p, float maxDist, float minDot) const {
        float dx = p.x - gameWorld_.player.position.x;
        float dy = p.y - gameWorld_.player.position.y;
        float dz = p.z - gameWorld_.player.position.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist < 0.001f || dist > maxDist) return false;
        XMFLOAT3 viewDir = DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch);
        float dot = (dx * viewDir.x + dy * viewDir.y + dz * viewDir.z) / dist;
        if (dot < minDot) return false;
        Tile targetTile = gameWorld_.maze.TileFromWorld(p.x, p.z);
        return gameWorld_.maze.LineClear(CameraTile(), targetTile);
    }

    float DistanceToPoint(const XMFLOAT3& p) const {
        float dx = p.x - gameWorld_.player.position.x;
        float dy = p.y - gameWorld_.player.position.y;
        float dz = p.z - gameWorld_.player.position.z;
        return std::sqrt(dx * dx + dy * dy + dz * dz);
    }

    bool PointWithinHorizontalRange(const XMFLOAT3& p, float maxDistance) const {
        float dx = p.x - gameWorld_.player.position.x;
        float dz = p.z - gameWorld_.player.position.z;
        return dx * dx + dz * dz <= maxDistance * maxDistance;
    }

    float ScareSensoryWeight(const XMFLOAT3& p, float visualDistance, float minDot, float loudDistance) const {
        if (PlayerLooksAt(p, visualDistance, minDot)) return 1.0f;
        float dist = DistanceToPoint(p);
        if (dist > loudDistance) return 0.0f;
        float close = Clamp01((loudDistance - dist) / std::max(0.1f, loudDistance));
        return 0.42f + close * 0.46f;
    }

    bool ScareSourceAhead(const XMFLOAT3& p, float minDistance, float maxDistance, int maxPathTiles, float minForwardDot) const {
        float dx = p.x - gameWorld_.player.position.x;
        float dz = p.z - gameWorld_.player.position.z;
        float horizontalDist = std::sqrt(dx * dx + dz * dz);
        if (horizontalDist < minDistance || horizontalDist > maxDistance) return false;

        Tile cameraTile = CameraTile();
        Tile sourceTile = gameWorld_.maze.TileFromWorld(p.x, p.z);
        if (!gameWorld_.maze.LineClear(cameraTile, sourceTile)) return false;

        XMFLOAT3 forward = Forward();
        float facing = horizontalDist > 0.001f ? (dx * forward.x + dz * forward.z) / horizontalDist : 1.0f;
        bool upcomingPath = false;
        size_t first = std::min(cameraRuntime_.pathIndex, cameraRuntime_.path.size());
        size_t last = std::min(cameraRuntime_.path.size(), first + static_cast<size_t>(std::max(1, maxPathTiles)));
        for (size_t i = first; i < last; ++i) {
            if (cameraRuntime_.path[i] == sourceTile) {
                upcomingPath = true;
                break;
            }
        }
        if (sourceTile == cameraTile && facing > 0.18f) {
            upcomingPath = true;
        }
        return (upcomingPath && facing > -0.18f) || facing >= minForwardDot;
    }
