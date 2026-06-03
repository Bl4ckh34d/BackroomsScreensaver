        XMFLOAT3 avoid = MonsterSelfAvoidanceVector(Add3(start, Scale3(dir, std::min(distance, gameWorld_.maze.TileMinimum() * 0.24f))));
        float avoidLen = Length3(avoid);
        if (avoidLen > 0.001f) {
            float avoidWeight = std::min(0.48f, avoidLen * 0.42f);
            dir = Normalize3(Add3(dir, Scale3(Scale3(avoid, 1.0f / avoidLen), avoidWeight)), dir);
        }
