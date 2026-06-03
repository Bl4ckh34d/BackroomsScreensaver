    XMFLOAT3 MonsterSelfAvoidanceVector(const XMFLOAT3& candidate) const {
        if (monsterPresentation_.trail.size() < 10) return {0.0f, 0.0f, 0.0f};
        float spacing = MonsterBodySpacing();
        float ignoreDistance = std::max(gameWorld_.maze.TileMinimum() * 0.82f, spacing * 4.0f);
        float bodyLength = MonsterBodyLengthMeters();
        float repelRadius = std::max(gameWorld_.maze.TileMinimum() * 0.34f, settingsRuntime_.live.monsterScale * 0.62f);
        float repelRadiusSq = repelRadius * repelRadius;
        XMFLOAT3 repel{0.0f, 0.0f, 0.0f};
        for (float d = ignoreDistance; d <= bodyLength; d += spacing * 1.35f) {
            XMFLOAT3 sample = MonsterTrailSample(d);
            float dx = candidate.x - sample.x;
            float dz = candidate.z - sample.z;
            float distSq = dx * dx + dz * dz;
            if (distSq <= 0.0001f || distSq > repelRadiusSq) continue;
            float dist = std::sqrt(distSq);
            float weight = 1.0f - dist / repelRadius;
            weight *= weight;
            repel.x += (dx / dist) * weight;
            repel.z += (dz / dist) * weight;
        }
        return repel;
    }
