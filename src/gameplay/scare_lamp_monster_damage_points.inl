
        if (dt <= 0.0f || monsterPreview_.active || gEffectDebugViewer || gBloodDebugEveryWall || settingsRuntime_.live.bloodStudyView) return;
        if (effectRuntime_.runtimeLamps.empty() || effectRuntime_.lampDamagePixels.empty()) return;

        float tileAvg = std::max(0.1f, gameWorld_.maze.TileAverage());
        float influenceRadius = std::max(tileAvg * 4.30f, 6.50f);
        float breakRadius = influenceRadius * 0.72f;
        int maxTileReach = static_cast<int>(std::ceil(influenceRadius / std::max(0.1f, gameWorld_.maze.TileMinimum()))) + 1;

        struct MonsterLampInfluencePoint {
            XMFLOAT3 pos;
            Tile tile;
        };
        std::vector<MonsterLampInfluencePoint> influencePoints;
        float bodySpacing = MonsterBodySpacing();
        int bodySamples = std::clamp(static_cast<int>(std::ceil(MonsterBodyLengthMeters() / bodySpacing)) + 1, 4, 48);
        influencePoints.reserve(static_cast<size_t>(bodySamples));
        for (int i = 0; i < bodySamples; ++i) {
            XMFLOAT3 p = (i == 0) ? gameWorld_.monster.position : MonsterTrailSample(static_cast<float>(i) * bodySpacing);
            Tile tile = gameWorld_.maze.TileFromWorld(p.x, p.z);
            if (!gameWorld_.maze.IsOpen(tile.x, tile.y)) continue;
            influencePoints.push_back({ p, tile });
        }
        if (influencePoints.empty()) return;
