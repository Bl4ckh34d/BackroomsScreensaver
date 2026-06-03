// Player camera attention visibility prop.

    static float Frac(float v) {
        return v - std::floor(v);
    }

    static float LampHash(float x, float z) {
        float px = Frac(x * 123.34f);
        float pz = Frac(z * 456.21f);
        float d = px * (px + 45.32f) + pz * (pz + 45.32f);
        px += d;
        pz += d;
        return Frac(px * pz);
    }

    float LampSeed(int cellX, int cellZ) const {
        return LampHash(static_cast<float>(cellX), static_cast<float>(cellZ));
    }

    bool LampBrokenZone(int cellX, int cellZ) const {
        return LampHash(static_cast<float>(cellX) + 83.7f, static_cast<float>(cellZ) - 29.4f) >= 1.0f - settingsRuntime_.live.brokenZoneRatio;
    }

    bool LampIsOn(int cellX, int cellZ) const {
        return !LampBrokenZone(cellX, cellZ) && LampSeed(cellX, cellZ) >= 1.0f - settingsRuntime_.live.lampOnRatio;
    }

    bool VisibleInFront(Tile target) const {
        Tile cur = CameraTile();
        if (!gameWorld_.maze.LineClear(cur, target)) return false;
        XMFLOAT3 tw = gameWorld_.maze.WorldCenter(target, gameWorld_.player.position.y);
        XMFLOAT3 f = Forward();
        float dx = tw.x - gameWorld_.player.position.x;
        float dz = tw.z - gameWorld_.player.position.z;
        float len = std::sqrt(dx * dx + dz * dz);
        if (len < 0.01f) return true;
        return (dx * f.x + dz * f.z) / len > 0.38f;
    }

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

    void UpdatePropLook(float dt, bool threat) {
        if (threat || cameraRuntime_.propLookPoints.empty()) {
            viewRuntime_.propLookTimer = 0.0f;
            return;
        }

        viewRuntime_.propLookCooldown = std::max(0.0f, viewRuntime_.propLookCooldown - dt);
        if (viewRuntime_.propLookTimer > 0.0f) {
            viewRuntime_.propLookTimer = std::max(0.0f, viewRuntime_.propLookTimer - dt);
            return;
        }
        if (viewRuntime_.propLookCooldown > 0.0f) return;

        XMFLOAT3 viewDir = Normalize3(DirectionFromYawPitch(gameWorld_.player.yaw, gameWorld_.player.pitch), {0.0f, 0.0f, 1.0f});
        Tile cur = CameraTile();
        XMFLOAT3 best{};
        float bestScore = -1.0f;
        for (const XMFLOAT3& p : cameraRuntime_.propLookPoints) {
            float dx = p.x - gameWorld_.player.position.x;
            float dy = p.y - gameWorld_.player.position.y;
            float dz = p.z - gameWorld_.player.position.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < 1.05f || dist > 8.8f) continue;
            XMFLOAT3 toProp = Normalize3({dx, dy, dz}, viewDir);
            float viewDot = Dot3(toProp, viewDir);
            if (viewDot < 0.88f) continue;
            float yawFromCamera = std::abs(AngleWrap(YawToPoint(p) - gameWorld_.player.yaw));
            float pitchFromCamera = std::abs(PitchToPoint(p) - gameWorld_.player.pitch);
            constexpr float kInnerYaw = 0.42f;
            constexpr float kInnerPitch = 0.27f;
            if (yawFromCamera > kInnerYaw || pitchFromCamera > kInnerPitch) continue;
            float rimPenalty = SmoothStep(0.58f, 1.0f, std::max(yawFromCamera / kInnerYaw, pitchFromCamera / kInnerPitch));
            Tile pt = gameWorld_.maze.TileFromWorld(p.x, p.z);
            if (!gameWorld_.maze.LineClear(cur, pt)) continue;
            float yawDelta = std::abs(AngleWrap(YawToPoint(p) - viewRuntime_.flashlightYaw));
            float closeInterest = SmoothStep(8.0f, 1.4f, dist);
            float verticalInterest = 1.0f - Clamp01(std::abs(dy) / 2.0f) * 0.18f;
            float repeatPenalty = 0.0f;
            if (viewRuntime_.hasLastPropLookTarget) {
                float ldx = p.x - viewRuntime_.lastPropLookTarget.x;
                float ldz = p.z - viewRuntime_.lastPropLookTarget.z;
                if (ldx * ldx + ldz * ldz < 0.70f) repeatPenalty = 1.45f;
            }
            float score = viewDot * 1.25f + closeInterest * 1.15f + verticalInterest -
                rimPenalty * 1.65f - std::min(yawDelta, 1.5f) * 0.18f - repeatPenalty + RandRange(-0.24f, 0.16f);
            if (score > bestScore) {
                bestScore = score;
                best = p;
            }
        }

        if (bestScore > 1.35f && RandRange(0.0f, 1.0f) < 0.48f) {
            viewRuntime_.propLookTarget = best;
            viewRuntime_.propLookDuration = RandRange(0.92f, 1.85f);
            viewRuntime_.propLookTimer = viewRuntime_.propLookDuration;
            viewRuntime_.propLookCooldown = RandRange(1.15f, 3.35f);
            viewRuntime_.propLookScanSeed = RandRange(0.0f, 1.0f);
            viewRuntime_.lastPropLookTarget = best;
            viewRuntime_.hasLastPropLookTarget = true;
        } else {
            viewRuntime_.propLookCooldown = RandRange(0.90f, 2.25f);
        }
    }
