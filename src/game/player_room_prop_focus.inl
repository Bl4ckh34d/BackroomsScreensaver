    bool FindRoomPropFocus(Tile cur, XMFLOAT3& focus) const {
        if (cameraRuntime_.propLookPoints.empty()) return false;
        float bestScore = -1.0e9f;
        XMFLOAT3 best{};
        for (const XMFLOAT3& p : cameraRuntime_.propLookPoints) {
            float dx = p.x - gameWorld_.player.position.x;
            float dz = p.z - gameWorld_.player.position.z;
            float dist = std::sqrt(dx * dx + dz * dz);
            if (dist < 1.05f || dist > 8.5f) continue;
            Tile pt = gameWorld_.maze.TileFromWorld(p.x, p.z);
            if (!gameWorld_.maze.LineClear(cur, pt)) continue;
            float yawDelta = std::abs(AngleWrap(YawToPoint(p) - gameWorld_.player.bodyYaw));
            float closeInterest = SmoothStep(8.5f, 1.6f, dist);
            float sideInterest = SmoothStep(0.12f, 1.25f, yawDelta) * (1.0f - SmoothStep(2.65f, 3.14f, yawDelta));
            float heightInterest = 1.0f - Clamp01(std::abs(p.y - 0.95f) / 2.0f) * 0.22f;
            float repeatPenalty = 0.0f;
            if (viewRuntime_.hasLastPropLookTarget) {
                float ldx = p.x - viewRuntime_.lastPropLookTarget.x;
                float ldz = p.z - viewRuntime_.lastPropLookTarget.z;
                if (ldx * ldx + ldz * ldz < 0.90f) repeatPenalty = 0.85f;
            }
            float score = closeInterest * 2.1f + sideInterest * 1.1f + heightInterest -
                std::min(yawDelta, 2.8f) * 0.10f - repeatPenalty;
            if (score > bestScore) {
                bestScore = score;
                best = p;
            }
        }
        if (bestScore < 1.35f) return false;
        focus = best;
        return true;
    }

