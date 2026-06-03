    bool LampHumAudibleCandidate(const RuntimeLampState& lamp, float maxDistSq) const {
        if (lamp.broken) return false;
        float dx = lamp.pos.x - gameWorld_.player.position.x;
        float dy = lamp.pos.y - gameWorld_.player.position.y;
        float dz = lamp.pos.z - gameWorld_.player.position.z;
        return dx * dx + dy * dy + dz * dz <= maxDistSq;
    }
