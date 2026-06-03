        float pathAdvanceRadius = std::clamp(gameWorld_.maze.TileMinimum() * 0.045f, 0.045f, 0.085f);
        for (int advance = 0; !softStopActive && dist < pathAdvanceRadius && advance < 3; ++advance) {
            ++cameraRuntime_.pathIndex;
            if (CameraTile() == gameWorld_.maze.exit) {
                BeginExitTransition();
                return;
            }
