    void PrimeMonsterTrail(float spacing, int pointCount = 96) {
        monsterPresentation_.trail.clear();
        pointCount = std::max(0, pointCount);
        for (int i = 0; i < pointCount; ++i) {
            float back = static_cast<float>(i) * spacing;
            monsterPresentation_.trail.push_back({
                gameWorld_.monster.position.x - std::sin(gameWorld_.monster.yaw) * back,
                0.0f,
                gameWorld_.monster.position.z - std::cos(gameWorld_.monster.yaw) * back
            });
        }
    }
