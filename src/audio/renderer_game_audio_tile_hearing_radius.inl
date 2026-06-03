    float TileHearingRadius(float tiles) const {
        return std::max(0.1f, gameWorld_.maze.TileAverage()) * std::max(0.0f, tiles);
    }
