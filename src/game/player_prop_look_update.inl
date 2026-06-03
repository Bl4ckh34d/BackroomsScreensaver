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
