// Monster collision, recovery, and movement helper functions. 
// Included inside Renderer's private section from monster_ai.inl.

    bool MonsterFootprintOpen(const XMFLOAT3& pos) const {
        float visualScale = std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.35f);
        float radius = std::clamp(0.46f * visualScale, 0.22f, 0.68f);
        const XMFLOAT2 samples[] = {
            {0.0f, 0.0f},
            { radius, 0.0f},
            {-radius, 0.0f},
            {0.0f,  radius},
            {0.0f, -radius},
            { radius * 0.54f,  radius * 0.54f},
            {-radius * 0.54f,  radius * 0.54f},
            { radius * 0.54f, -radius * 0.54f},
            {-radius * 0.54f, -radius * 0.54f}
        };
        for (const XMFLOAT2& s : samples) {
            Tile tile = gameWorld_.maze.TileFromWorld(pos.x + s.x, pos.z + s.y);
            if (!gameWorld_.maze.IsOpen(tile.x, tile.y)) return false;
        }
        return true;
    }

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

    void MoveMonsterToward(const XMFLOAT3& target, float distance) {
        if (distance <= 0.0001f) return;
        XMFLOAT3 start = gameWorld_.monster.position;
        XMFLOAT3 delta{target.x - start.x, 0.0f, target.z - start.z};
        float len = Length3(delta);
        if (len <= 0.0001f) return;
        XMFLOAT3 directDir = Scale3(delta, 1.0f / len);
        XMFLOAT3 dir = directDir;
        bool erraticChase = gameWorld_.monster.chasingVisible || gameWorld_.monster.recognizedForChase || gameWorld_.monster.chaseMemoryTimer > 0.0f;
        if (erraticChase && len > gameWorld_.maze.TileMinimum() * 0.24f) {
            XMFLOAT3 lateral{dir.z, 0.0f, -dir.x};
            float burst = std::pow(std::max(0.0f, std::sin(timeRuntime_.time * 1.15f + gameWorld_.monster.position.x * 0.18f - gameWorld_.monster.position.z * 0.13f)), 5.0f);
            float twitch = std::sin(timeRuntime_.time * 2.6f + gameWorld_.monster.position.z * 0.27f) * 0.028f +
                std::sin(timeRuntime_.time * 4.1f + gameWorld_.monster.position.x * 0.17f) * 0.012f;
            float chaseBlend = std::max(monsterPresentation_.headChaseBlend, gameWorld_.monster.chasingVisible ? 0.72f : 0.36f);
            dir = Normalize3(Add3(dir, Scale3(lateral, twitch * (0.68f + burst * 0.32f) * chaseBlend)), dir);
        }
        XMFLOAT3 avoid = MonsterSelfAvoidanceVector(Add3(start, Scale3(dir, std::min(distance, gameWorld_.maze.TileMinimum() * 0.24f))));
        float avoidLen = Length3(avoid);
        if (avoidLen > 0.001f) {
            float avoidWeight = std::min(0.48f, avoidLen * 0.42f);
            dir = Normalize3(Add3(dir, Scale3(Scale3(avoid, 1.0f / avoidLen), avoidWeight)), dir);
        }
        float remaining = std::min(distance, len);
        int steps = std::max(1, static_cast<int>(std::ceil(remaining / (gameWorld_.maze.TileMinimum() * 0.085f))));
        XMFLOAT3 pos = start;
        bool moved = false;
        for (int i = 0; i < steps; ++i) {
            float step = remaining / static_cast<float>(steps);
            XMFLOAT3 next{pos.x + dir.x * step, 0.0f, pos.z + dir.z * step};
            if (!MonsterFootprintOpen(next)) {
                XMFLOAT3 slideX{pos.x + dir.x * step, 0.0f, pos.z};
                XMFLOAT3 slideZ{pos.x, 0.0f, pos.z + dir.z * step};
                XMFLOAT3 directNext{pos.x + directDir.x * step, 0.0f, pos.z + directDir.z * step};
                bool preferX = std::abs(delta.x) > std::abs(delta.z);
                bool movedSlide = false;
                if (preferX && MonsterFootprintOpen(slideX)) {
                    pos = slideX;
                    movedSlide = true;
                } else if (!preferX && MonsterFootprintOpen(slideZ)) {
                    pos = slideZ;
                    movedSlide = true;
                } else if (MonsterFootprintOpen(directNext)) {
                    pos = directNext;
                    movedSlide = true;
                } else if (MonsterFootprintOpen(preferX ? slideZ : slideX)) {
                    pos = preferX ? slideZ : slideX;
                    movedSlide = true;
                }
                if (!movedSlide) {
                    if (!MonsterFootprintOpen(pos)) {
                        XMFLOAT3 center = gameWorld_.maze.WorldCenter(MonsterTile(), 0.0f);
                        if (MonsterFootprintOpen(center)) pos = Lerp3(pos, center, 0.35f);
                    }
                    break;
                }
                moved = true;
                continue;
            }
            pos = next;
            moved = true;
        }
        if (moved) {
            gameWorld_.monster.position = pos;
            RecordMonsterTrailPoint(gameWorld_.monster.position);
            float moveYaw = std::atan2(gameWorld_.monster.position.x - start.x, gameWorld_.monster.position.z - start.z);
            if (std::isfinite(moveYaw)) {
                gameWorld_.monster.yaw += AngleWrap(moveYaw - gameWorld_.monster.yaw) * 0.22f;
            }
        }
    }

    bool RecoverMonsterToNearestOpenTile(Tile fromTile) {
        if (gameWorld_.maze.IsOpen(fromTile.x, fromTile.y)) return true;
        Tile best{-1000, -1000};
        float bestSq = std::numeric_limits<float>::max();
        for (int radius = 1; radius <= 5 && !ValidMonsterTile(best); ++radius) {
            for (int dz = -radius; dz <= radius; ++dz) {
                for (int dx = -radius; dx <= radius; ++dx) {
                    if (std::abs(dx) != radius && std::abs(dz) != radius) continue;
                    Tile t{fromTile.x + dx, fromTile.y + dz};
                    if (!gameWorld_.maze.IsOpen(t.x, t.y)) continue;
                    XMFLOAT3 c = gameWorld_.maze.WorldCenter(t, 0.0f);
                    float sx = c.x - gameWorld_.monster.position.x;
                    float sz = c.z - gameWorld_.monster.position.z;
                    float d2 = sx * sx + sz * sz;
                    if (d2 < bestSq) {
                        bestSq = d2;
                        best = t;
                    }
                }
            }
        }
        if (!ValidMonsterTile(best)) return false;
        XMFLOAT3 target = gameWorld_.maze.WorldCenter(best, 0.0f);
        float maxStep = std::max(gameWorld_.maze.TileMinimum() * 0.18f, settingsRuntime_.live.monsterSpeed * 0.12f);
        XMFLOAT3 delta{target.x - gameWorld_.monster.position.x, 0.0f, target.z - gameWorld_.monster.position.z};
        float len = Length3(delta);
        gameWorld_.monster.position = len > maxStep ? Add3(gameWorld_.monster.position, Scale3(delta, maxStep / std::max(0.001f, len))) : target;
        if (len > gameWorld_.maze.TileAverage() * 1.3f) {
            ResetMonsterPresentationState(true, false, false);
        }
        RecordMonsterTrailPoint(gameWorld_.monster.position);
        ClearMonsterPath();
        return true;
    }

    bool MonsterReachedTile(Tile t) const {
        if (!(MonsterTile() == t)) return false;
        XMFLOAT3 center = gameWorld_.maze.WorldCenter(t, 0.0f);
        float dx = center.x - gameWorld_.monster.position.x;
        float dz = center.z - gameWorld_.monster.position.z;
        float tile = gameWorld_.maze.TileMinimum();
        return dx * dx + dz * dz < tile * tile * 0.070f;
    }

    bool MonsterGoalFarEnough(Tile goal) const {
        if (!ValidMonsterTile(goal)) return false;
        XMFLOAT3 center = gameWorld_.maze.WorldCenter(goal, 0.0f);
        float dx = center.x - gameWorld_.monster.position.x;
        float dz = center.z - gameWorld_.monster.position.z;
        float minDist = std::max(gameWorld_.maze.TileMinimum() * 0.82f, 0.70f * std::clamp(settingsRuntime_.live.monsterScale, 0.35f, 1.35f));
        return dx * dx + dz * dz >= minDist * minDist;
    }
