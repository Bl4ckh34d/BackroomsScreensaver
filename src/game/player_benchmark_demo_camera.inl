    void ApplyBenchmarkDemoCamera(float seconds) {
        const Maze& maze = RenderMazeView();
        auto& path = benchmarkRuntime_.flightPath;
        if (path.empty()) {
            // Each leg follows open corridors between the large rooms in GenerateBenchmarkDemo.
            static constexpr Tile waypoints[] = {
                {13, 55}, {13, 42}, {24, 38}, {35, 31}, {43, 38}, {57, 38}, {57, 56},
                {65, 58}, {54, 54}, {54, 36}, {56, 23}, {43, 17}, {19, 17}, {16, 30}, {13, 55}
            };
            path.push_back(maze.start);
            for (Tile waypoint : waypoints) {
                const auto leg = maze.Path(path.back(), waypoint);
                if (leg.size() > 1) path.insert(path.end(), leg.begin() + 1, leg.end());
            }
        }
        constexpr float kCycleSeconds = 59.5f;
        const size_t legs = path.size() - 1;
        auto samplePath = [&](float tSeconds, float heightOffset) {
            const float height = 1.48f + heightOffset + std::sin(tSeconds * 0.60f) * 0.04f;
            if (legs == 0) return maze.WorldCenter(path.front(), height);
            const float cursor = std::fmod(tSeconds, kCycleSeconds) / kCycleSeconds * static_cast<float>(legs);
            const size_t index = std::min(static_cast<size_t>(cursor), legs - 1);
            return Lerp3(maze.WorldCenter(path[index], height), maze.WorldCenter(path[index + 1], height),
                cursor - static_cast<float>(index));
        };

        XMFLOAT3 eye = samplePath(seconds, 0.0f);
        XMFLOAT3 target = samplePath(seconds + 0.75f, -0.12f);
        float dx = target.x - eye.x;
        float dz = target.z - eye.z;
        float invLen = 1.0f / std::max(0.001f, std::sqrt(dx * dx + dz * dz));
        float sideLook = std::sin(seconds * 0.42f) * maze.TileAverage() * 0.34f;
        target.x += -dz * invLen * sideLook;
        target.z += dx * invLen * sideLook;
        target.y += std::sin(seconds * 0.31f) * 0.045f;

        ApplyDebugCameraLookAt(eye, target, -0.48f, 0.42f);
        viewRuntime_.cameraMotionBlur = {};
        gameWorld_.SetPlayerSmoothedMoveSpeed(1.25f);
        gameWorld_.RestorePlayerFullVitals();
    }
